using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace openu_video_fetcher
{
    public class VideoData
    {
        public VideoData(string contextId, string courseId, string vid)
        {
            ContextId = contextId;
            CourseId = courseId;
            VidId = vid;
        }

        public string ContextId { get; set; }
        public string CourseId { get; set; }
        public string PlaylistUrl { get; set; }
        public string VidId { get; set; }

        public string formatCacheFileName()
        {
            return string.Format("{0}-{1}-{2}_addr", ContextId, CourseId, VidId);
        }

        public string formatPlaylistFile()
        {
            return formatCacheFileName() + "_file";
        }

        private string formatChunksFile(string quality)
        {
            return formatCacheFileName() + "_chunks_" + quality;
        }

        internal string formatChunksFileUrl(string wantedQuality)
        {
            return formatChunksFile(wantedQuality) + "_url";
        }

        internal string formatChunksFileContent(string wantedQuality)
        {
            return formatChunksFile(wantedQuality) + ".m3u8";
        }

        internal string formatFFmpegFile()
        {
            return formatCacheFileName() + ".m3u8";
        }

        internal object formatOutputFile()
        {
            return formatCacheFileName() + ".mp4";
        }
    }

    public class UrlFileCache
    {
        public UrlFileCache(DirectoryInfo input)
        {
            parentDir = input;
        }

        public string GetContent(string playlistName, string subDir = "")
        {
            try
            {
                return RetrieveCachedFile(playlistName, subDir);
            }
            catch
            {
                return null;
            }
        }

        public FileInfo GetFile(string key, string subDir = "")
        {
            return new FileInfo(Path.Combine(getDstDir(subDir), key));
        }

        public void PutBinary(string key, byte[] content, string subDir = "")
        {
            try
            {
                var dstDir = getDstDir(subDir);
                if (!Directory.Exists(dstDir))
                    parentDir.CreateSubdirectory(subDir);
                File.WriteAllBytes(Path.Combine(dstDir, key), content);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }

        }

        public void PutContent(string key, string content, string subDir = "")
        {
            try
            {
                var dstDir = getDstDir(subDir);
                if (!Directory.Exists(dstDir))
                    parentDir.CreateSubdirectory(subDir);
                File.WriteAllText(Path.Combine(dstDir, key), content);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
        }

        private string RetrieveCachedFile(string key, string subDir)
        {
            var dstDir = getDstDir(subDir);
            return File.ReadAllText(Path.Combine(dstDir, key));
        }

        private string getDstDir(string subDir)
        {
            return Path.Combine(parentDir.FullName, subDir);
        }

        DirectoryInfo parentDir;

        internal bool Contains(string key, string subDir = "")
        {
            var dstDir = Path.Combine(parentDir.FullName, subDir);

            try
            {
                return File.Exists(Path.Combine(dstDir, key));
            }
            catch
            {
                return false;
            }
        }
    }

    public class VideoDownloader
    {
        UrlFileCache cache;
        VideoData data;
        ChunksFile chunks;
        List<string> chunkNames;
        string baseUrl;

        string wantedQuality = "40000";

        public VideoDownloader(UrlFileCache bank, VideoData data)
        {
            this.cache = bank;
            this.data = data;
            durations = new ConcurrentBag<TimeSpan>();
        }

        string requestPlaylist(VideoData data)
        {
            var playlistFile = cache.GetContent(data.formatPlaylistFile(), data.VidId);
            if (playlistFile != null)
                return playlistFile;

            HttpWebRequest req = (HttpWebRequest)WebRequest.Create(data.PlaylistUrl);
            req.AllowAutoRedirect = true;
            var response = (HttpWebResponse) req.GetResponse();
            if (response.StatusCode != HttpStatusCode.OK)
            {
                return null;
            }
            playlistFile = new StreamReader(response.GetResponseStream()).ReadToEnd();
            cache.PutContent(data.formatPlaylistFile(), playlistFile, data.VidId);
            return playlistFile;
        }

        public void Download()
        {
            string playlistM3U8 = requestPlaylist(data);
            parsePlaylist(playlistM3U8);

            baseUrl = chunks.Url.Remove(chunks.Url.LastIndexOf("/"));

            new Thread(printProgress).Start();

            Parallel.ForEach(chunkNames, new ParallelOptions { MaxDegreeOfParallelism = 20 }, 
                chunkName => downloadChunk(chunkName));

            var inputFile = cache.GetFile(data.formatFFmpegFile(), data.VidId);


            var p = new Process();
            p.StartInfo.UseShellExecute = false;
            p.StartInfo.RedirectStandardOutput = true;
            p.StartInfo.RedirectStandardError = true;
            p.StartInfo.FileName = Path.Combine("ffmpeg", "ffmpeg.exe");
            p.StartInfo.Arguments = String.Format("-i \"{0}\" -c copy -bsf:a aac_adtstoasc {1}", inputFile, data.formatOutputFile());
            p.StartInfo.WorkingDirectory = inputFile.Directory.FullName;
            p.Start();

            var errors = p.StandardError;
            var output = p.StandardOutput;
            Console.WriteLine(errors.ReadToEnd());
            Console.WriteLine(errors.ReadToEnd());
            p.WaitForExit();
        }

        private void printProgress()
        {
            Stopwatch totalRuntime = new Stopwatch();
            totalRuntime.Start();
            while (total != counter)
            {
                TimeSpan overallDuration = new TimeSpan();
                foreach (var dur in durations)
                    overallDuration = overallDuration.Add(dur);

                double average = 0;
                long remainingMilli = 0;
                if (counter != 0)
                {
                    average = ((double)overallDuration.TotalMilliseconds) / counter;
                    remainingMilli = (long)((double)totalRuntime.ElapsedMilliseconds / counter) * (total - counter);
                }
                var remaining = TimeSpan.FromMilliseconds(remainingMilli);

                Console.WriteLine("Progress: remaining - {0}, {1} / {2}, active {3}, avg = {4}", 
                                   remaining, counter, total, active, average);
                Thread.Sleep((int)TimeSpan.FromSeconds(1).TotalMilliseconds);
            }
        }

        private void parsePlaylist(string playlistM3U8)
        {
            chunks = getChunksFile(playlistM3U8);
            var allChunkNames = parseChunksFile(chunks.Content);
            chunkNames = new List<string>();

            foreach (var chunkName in allChunkNames)
            {
                if (cache.Contains(chunkName, data.VidId))
                    Console.WriteLine("Skipping '{0}' on {1}, piece exists.", chunkName, data.VidId);
                else
                    chunkNames.Add(chunkName);
            }

            total = chunkNames.Count;
        }

        private ChunksFile getChunksFile(string playlistM3U8)
        {
            ChunksFile chunks = new ChunksFile();

            chunks.Url = cache.GetContent(data.formatChunksFileUrl(wantedQuality), data.VidId);
            chunks.Content = cache.GetContent(data.formatChunksFileContent(wantedQuality), data.VidId);

            if (chunks.Content == null || chunks.Url == null)
                chunks = fetchAndCacheChunksFile(playlistM3U8);

            return chunks;
        }

        int counter = 0;
        int active = 0;
        ConcurrentBag<TimeSpan> durations;
        int total;

        private void downloadChunk(string chunkName)
        {
            Stopwatch w = new Stopwatch();
            w.Start();
            Interlocked.Increment(ref active);

            int tryNum = 0;
            bool success = false;

            do
                try
                {
                    Console.WriteLine("Started downloading '{0}' on {1}.", chunkName, data.VidId);
                    var req = WebRequest.Create(baseUrl + "/" + chunkName);
                    var resp = req.GetResponse();
                    var chunkData = new BinaryReader(resp.GetResponseStream()).ReadBytes((int)resp.ContentLength);

                    cache.PutBinary(chunkName, chunkData, data.VidId);
                    success = true;
                }
                catch
                {
                    tryNum++;
                    Console.WriteLine("Failed downloading '{0}' on {1}.", chunkName, data.VidId);
                }
            while (tryNum < 3 && success == false);

            if (success)
                Console.WriteLine("Done downloading '{0}' on {1}.", chunkName, data.VidId);
            else
            {
                
            }

            Interlocked.Increment(ref counter);
            Interlocked.Decrement(ref active);
            w.Stop();
            durations.Add(w.Elapsed);
        }

        private List<string> parseChunksFile(string chunksFile)
        {
            List<string> chunkNames = new List<string>();
            var ext = ".ts";

            var chunkMetaFinder = new Regex("#EXTINF:");

            using (StringReader sr = new StringReader(chunksFile))
            {
                // This is needed because the resources names inside the chunks file
                // might not be legal file names, that means ffmpeg conversion won't work.
                using (StringWriter wr = new StringWriter())
                {
                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        wr.WriteLine(line);
                        if (chunkMetaFinder.IsMatch(line))
                        {
                            var rawChunkName = sr.ReadLine();
                            var chunkFileName = rawChunkName.Remove(rawChunkName.IndexOf(ext)) + ext;
                            wr.WriteLine(chunkFileName);
                            chunkNames.Add(chunkFileName);
                        }
                    }
                    wr.Close();
                    cache.PutContent(data.formatFFmpegFile(), wr.ToString(), data.VidId);
                }
            }

            return chunkNames;
        }

        struct ChunksFile
        {
            public string Url { get; set; }
            public string Content { get; set; }
        }

        private ChunksFile fetchAndCacheChunksFile(string playlistM3U8)
        {
            string chunksUrl = getChunksFileUrl(playlistM3U8);
            var chunksReq = (HttpWebRequest)WebRequest.Create(chunksUrl);
            var resp = chunksReq.GetResponse();
            string chunksFile = new StreamReader(resp.GetResponseStream()).ReadToEnd();

            cache.PutContent(data.formatChunksFileUrl(wantedQuality), chunksUrl, data.VidId);
            cache.PutContent(data.formatChunksFileContent(wantedQuality), chunksFile, data.VidId);

            return new ChunksFile { Url = chunksUrl, Content = chunksFile };
        }

        private string getChunksFileUrl(string playlistM3U8)
        {
            var qualityTester = new Regex(String.Format(":BANDWIDTH={0}", wantedQuality));
            var lines = playlistM3U8.Split('\n');
            for (int i = 0; i < lines.Length; i++)
            {
                if (qualityTester.IsMatch(lines[i]))
                {
                    return lines[i + 1];
                }
            }
            return null;
        }
    }
}
