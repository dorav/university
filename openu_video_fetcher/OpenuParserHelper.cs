using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;

namespace openu_video_fetcher
{
    class OpenuCollectionData
    {
        public string CourseId { get; set; }
        public string ContextId { get; set; }
    }

    public class OpenuParser
    {
        private CookieContainer cookieJar;
        private UrlFileCache cache;

        public OpenuParser(CookieContainer jar, UrlFileCache cache)
        {
            this.cookieJar = jar;
            this.cache = cache;
        }

        private List<OpenuVideoData> extractViewedCollection(string html)
        {
            var collectionHtml = getWantedCollection(html);

            var videos = extractPlaylistData(collectionHtml);

            incorporateCollectionData(html, videos);

            return videos;
        }

        public List<OpenuVideoData> GetVideosInView(string vidChooserHtml)
        {
            List<OpenuVideoData> videoUrls = extractViewedCollection(vidChooserHtml);

            foreach (var vid in videoUrls)
            {
                var vidUrlCacheName = vid.formatCacheFileName();
                var vidIdentifier = cache.GetContent(vidUrlCacheName);
                if (vidIdentifier == null)
                {
                    vidIdentifier = GetVidUrl(vid, cookieJar);
                    cache.PutContent(vidUrlCacheName, vidIdentifier);
                }
                if (vidIdentifier == null)
                    Console.WriteLine("Error when trying to fetch video with id = {0}", vid);
                else
                {
                    vid.PlaylistUrl = string.Format("http://api.bynetcdn.com/Redirector/openu/manifest/{0}_mp4/HLS/playlist.m3u8", vidIdentifier);
                }
            }

            // This way we get the videos in chronological order
            videoUrls.Reverse();
            return videoUrls;
        }

        private static void incorporateCollectionData(string vidChooserHtml, List<OpenuVideoData> videoUrls)
        {
            var collectionData = new OpenuCollectionData() { ContextId = GetContext(vidChooserHtml), CourseId = GetCourseId(vidChooserHtml) };
            videoUrls.ForEach((x) => {
                x.CourseId = collectionData.CourseId;
                x.ContextId = collectionData.ContextId;
            });
        }

        private static JObject ResponseAsJson(HttpWebResponse resp)
        {
            var specificVidData = new StreamReader(resp.GetResponseStream()).ReadToEnd();
            try
            {
                return JObject.Parse(specificVidData);
            }
            catch { }

            return new JObject();
        }

        private static void PutData(string data, HttpWebRequest request)
        {
            var bytes = new UTF8Encoding().GetBytes(data);
            request.ContentLength = bytes.Length;
            request.GetRequestStream().Write(bytes, 0, bytes.Length);
            request.GetRequestStream().Close();
        }

        private static string GetVidUrl(OpenuVideoData data, CookieContainer cookieJar)
        {
            var specificVidParams = String.Format("action=getplaylist&context={0}&playlistid={1}&course={2}", data.ContextId, data.VideoId, data.CourseId);
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

        private static string getWantedCollection(string html)
        {
            HtmlAgilityPack.HtmlDocument doc = new HtmlAgilityPack.HtmlDocument();
            doc.LoadHtml(html);

            var vidCollections = new Regex("id=\"(collection[0-9]*)").Matches(html);
            foreach (Match vidsCollection in vidCollections)
            {
                var ids = doc.DocumentNode.SelectNodes("//div[contains(@id,'" + vidsCollection.Groups[1].Value + "')]");

                if (!isHidden(ids))
                    return ids[0].InnerHtml;
            }

            return null;
        }

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

        private static List<OpenuVideoData> extractPlaylistData(string collectionHtml)
        {
            var playlist = new List<OpenuVideoData>();

            HtmlAgilityPack.HtmlDocument doc2 = new HtmlAgilityPack.HtmlDocument();
            doc2.LoadHtml(collectionHtml);

            foreach (var videoHtml in doc2.DocumentNode.SelectNodes("//div[contains(@id,'playlist')]//span[contains(@class, 'ovc_playlist_title')]"))
            {
                var id = videoHtml.ParentNode.ParentNode.Id;

                id = new Regex("[0-9]+").Match(id).Value;

                var title = videoHtml.InnerText;
                playlist.Add(new OpenuVideoData() { VideoId = id, Title = title });
            }

            return playlist;
        }

        private static bool isHidden(HtmlAgilityPack.HtmlNodeCollection doc)
        {
            var props = doc[0].GetAttributeValue("class", null);

            return props.Contains("ovc_hidden");
        }
    }
}
