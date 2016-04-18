using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.IO;
using CommandLine;
using CommandLine.Text;
using System.Text.RegularExpressions;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace openu_video_fetcher
{
    class ProgramArgs
    {
        [Option('u', "username", Required = true, HelpText = "openu.ac.il/sheilta username.")]
        public string UserName { get; set; }

        [Option('p', "password", Required = true, HelpText = "openu.ac.il/sheilta password.")]
        public string Password { get; set; }

        [Option('i', "id", Required = true, HelpText = "openu.ac.il/sheilta id.")]
        public string ID { get; set; }

        [Option('l', "playlist", Required = true, HelpText = "The playlist you wish to download")]
        public string PlaylistURL { get; set; }

        public DirectoryInfo DownloadDirectory { get; set; }
        public DirectoryInfo CacheDirectory { get; set; }

        [Option('d', "directory", Required = true, HelpText = "The playlist you wish to download")]
        public string DownloadDirectoryPath
        {
            get
            {
                return DownloadDirectory.FullName;
            }
            set
            {
                if (Directory.Exists(value))
                    DownloadDirectory = new DirectoryInfo(value);
                else
                    CreateDownloadDir(value);

                string cacheDirName = "cache";
                string cacheDirPath = Path.Combine(DownloadDirectory.FullName, cacheDirName);
                if (DownloadDirectory != null && Directory.Exists(cacheDirPath))
                {
                    CacheDirectory = new DirectoryInfo(cacheDirPath);
                }
                else
                {
                    try
                    {
                        CacheDirectory = DownloadDirectory.CreateSubdirectory(cacheDirName);
                    }
                    catch
                    {
                        Console.Write("Creating directory \"{0}\"...", value);
                    }
                }
            }
        }

        private void CreateDownloadDir(string value)
        {
            Console.Write("Creating directory \"{0}\"...", value);
            try
            {
                DownloadDirectory = Directory.CreateDirectory(value);
            }
            catch
            {
                Console.WriteLine(" Failed to create directory. Stopping");
                DownloadDirectory = null;
                return;
            }

            Console.WriteLine("OK");
        }

        [HelpOption('h', "help")]
        public string GetUsage()
        {
            return HelpText.AutoBuild(this,
              (HelpText current) => HelpText.DefaultParsingErrorsHandler(this, current));
        }
    }

    class Program
    {
        static string referrer = "https://sso.apps.openu.ac.il/SheiltaPortalLogin?T_PLACE=https://sheilta.apps.openu.ac.il/pls/dmyopt2/sheilta.main";

        static ProgramArgs GetCredentials(string[] args)
        {
            ProgramArgs opts = new ProgramArgs();

            if (!CommandLine.Parser.Default.ParseArguments(args, opts) || opts.DownloadDirectory == null)
            {
                Console.WriteLine("Problem with given arguments. exiting");
                return null;
            }
        
            return opts;
        }

        static CookieContainer cookieJar;

        static string GetContext(string playlistHtml)
        {
            var contextDelimiter = new Regex("context-(\\d+)");
            return contextDelimiter.Match(playlistHtml).Groups[1].Value;
        }

        static string GetCourseId(string playlistHtml)
        {
            var contextDelimiter = new Regex(" course-(\\d+)");
            return contextDelimiter.Match(playlistHtml).Groups[1].Value;
        }

        static List<String> GetVideoCodes(string html)
        {
            List<String> arr = new List<string>();
            var tokenToFind = new Regex("id=\"playlist(\\d+)");
            foreach (Match match in tokenToFind.Matches(html))
                arr.Add(match.Groups[1].Value);
            return arr;
        }

        static void DownloadVideo(VideoData data, UrlFileCache cache)
        {
            // TODO: prompt user for each video
            new VideoDownloader(cache, data).Download();
        }

        static void Main(string[] args)
        {
            var opts = GetCredentials(args);
            if (opts == null)
                return;

            cookieJar = new CookieContainer();
            UrlFileCache cache = new UrlFileCache(opts.CacheDirectory);

            if (login(opts))
            {
                var videoUrls = GetVideosUrls(opts, cache);
                videoUrls.ForEach(x => DownloadVideo(x, cache));
            }
            else
            {
                Console.WriteLine("Failed to login, check your credentials!");
            }
        }

        private static List<VideoData> GetVideosUrls(ProgramArgs opts, UrlFileCache cache)
        {
            // This request can't be cached as we need cookies from it
            var playlistPageReq = (HttpWebRequest)WebRequest.Create(opts.PlaylistURL);
            playlistPageReq.CookieContainer = cookieJar;
            using (HttpWebResponse playlistPageResp = (HttpWebResponse)playlistPageReq.GetResponse())
            {
                if (playlistPageResp.StatusCode == HttpStatusCode.OK)
                {
                    var playlistHtml = new StreamReader(playlistPageResp.GetResponseStream()).ReadToEnd();
                    return GetVideosUrls(playlistHtml, cache);
                }
                else
                {
                    Console.WriteLine("Failed to get playlist of course videos");
                    return new List<VideoData>();
                }
            }
        }

        private static List<VideoData> GetVideosUrls(string playlistHtml, UrlFileCache cache)
        {
            List<VideoData> videoUrls = new List<VideoData>();

            var contextId = GetContext(playlistHtml);
            var courseId = GetCourseId(playlistHtml);

            foreach (var vid in GetVideoCodes(playlistHtml))
            {
                VideoData data = new VideoData(contextId, courseId, vid);
                var vidUrlCacheName = data.formatCacheFileName();
                var vidIdentifier = cache.GetContent(vidUrlCacheName);
                if (vidIdentifier == null)
                {
                    vidIdentifier = GetVidUrl(data);
                    cache.PutContent(vidUrlCacheName, vidIdentifier);
                }
                if (vidIdentifier == null)
                    Console.WriteLine("Error when trying to fetch video with id = {0}", vid);
                else
                {
                    data.PlaylistUrl = string.Format("http://api.bynetcdn.com/Redirector/openu/manifest/{0}_mp4/HLS/playlist.m3u8", vidIdentifier);
                    videoUrls.Add(data);
                }
            }

            // This way we get the videos in chronological order
            videoUrls.Reverse();
            return videoUrls;
        }

        private static bool login(ProgramArgs opts)
        {
            var initialRequest = (HttpWebRequest)WebRequest.Create("https://sso.apps.openu.ac.il/SheiltaPortalProcess");
            initialRequest.Method = "POST";
            initialRequest.ContentType = "application/x-www-form-urlencoded";
            initialRequest.Referer = referrer;
            initialRequest.UserAgent = "Mozilla/5.0 (Windows NT 10.0; WOW64; rv:38.0) Gecko/20100101 Firefox/38.0";
            initialRequest.CookieContainer = cookieJar;

            // The login page sets an initial cookie to indicate weather the browser supports cookies
            var loginPage = (HttpWebRequest)WebRequest.Create(referrer);
            loginPage.UserAgent = "Mozilla/5.0 (Windows NT 10.0; WOW64; rv:38.0) Gecko/20100101 Firefox/38.0";
            loginPage.CookieContainer = cookieJar;
            loginPage.GetResponse().Close();

            string credFormat = string.Format("p_user={0}&p_sisma={1}&p_mis_student={2}&T_PLACE={3}", opts.UserName, opts.Password, opts.ID, opts.PlaylistURL);
            PutData(credFormat, initialRequest);
            HttpWebResponse response = (HttpWebResponse)initialRequest.GetResponse();

            return response.StatusCode == HttpStatusCode.OK &&
                   response.Cookies.Count > 1;
        }

        private static string GetVidUrl(VideoData data)
        {
            var specificVidParams = String.Format("action=getplaylist&context={0}&playlistid={1}&course={2}", data.ContextId, data.VidId, data.CourseId);
            var specificVidReq = (HttpWebRequest)WebRequest.Create("http://opal.openu.ac.il/mod/ouilvideocollection/actions.php");
            specificVidReq.CookieContainer = cookieJar;
            specificVidReq.Method = "POST";
            specificVidReq.ContentType = "application/x-www-form-urlencoded; charset=UTF-8";
            specificVidReq.UserAgent = "Mozilla/5.0 (Windows NT 10.0; WOW64; rv:38.0) Gecko/20100101 Firefox/38.0";
            PutData(specificVidParams, specificVidReq);

            var response = (HttpWebResponse)specificVidReq.GetResponse();
            if (response.StatusCode != HttpStatusCode.OK)
                return null;
            var videoData = ResponseAsJson(response);

            JToken value;

            if (videoData.TryGetValue("media", out value))
                return value["ar"].ToString();
            return null;
        }

        private static JObject ResponseAsJson(HttpWebResponse resp)
        {
            var specificVidData = new StreamReader(resp.GetResponseStream()).ReadToEnd();
            try
            {
                return JObject.Parse(specificVidData);
            }
            catch {}

            return new JObject();
        }

        private static void PutData(string data, HttpWebRequest request)
        {
            var bytes = new UTF8Encoding().GetBytes(data);
            request.ContentLength = bytes.Length;
            request.GetRequestStream().Write(bytes, 0, bytes.Length);
            request.GetRequestStream().Close();
        }
    }
}
