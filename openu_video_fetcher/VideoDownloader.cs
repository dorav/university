using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace openu_video_fetcher
{
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

        internal bool HasFile(string key, string subDir = "")
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

        internal string GetSubDir(string v)
        {
            var fullPath = Path.Combine(parentDir.FullName, v);
            if (!Directory.Exists(fullPath))
                parentDir.CreateSubdirectory(v);
            return fullPath;
        }
    }

    public class VideoDownloader
    {
        UrlFileCache cache;
        OpenuVideoData data;
        ChunksFile chunks;
        List<string> chunkNames;
        string baseUrl;

        string wantedQuality = "40000";
        string wantedQuality2nd = "800000";

        public VideoDownloader(UrlFileCache bank, OpenuVideoData data, BackgroundWorker changesReporter)
        {
            this.cache = bank;
            this.data = data;
            durations = new ConcurrentBag<TimeSpan>();
            this.changesReporter = changesReporter;
        }

        string requestPlaylist(OpenuVideoData data)
        {
            var playlistFile = cache.GetContent(data.formatPlaylistFile(), data.VideoId);
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
            cache.PutContent(data.formatPlaylistFile(), playlistFile, data.VideoId);
            return playlistFile;
        }

        public void Download()
        {
            string playlistM3U8 = requestPlaylist(data);
            parsePlaylist(playlistM3U8);

            baseUrl = chunks.Url.Remove(chunks.Url.LastIndexOf("/"));

            new Thread(printProgress).Start();

            Parallel.ForEach(chunkNames, new ParallelOptions { MaxDegreeOfParallelism = 10 }, 
                chunkName => downloadChunk(chunkName));

            var inputFile = cache.GetFile(data.formatFFmpegFile(), data.VideoId);
            var outputFile = data.formatOutputFile();

            if (File.Exists(outputFile))
            {
                try
                {
                    File.Delete(outputFile);
                }
                catch
                {
                    Console.WriteLine("Unable to combine video - '{0}', because the output file '{1}' exists", data.VideoId, outputFile);
                    return;
                }
            }

            var p = new Process();
            p.StartInfo.UseShellExecute = false;
            p.StartInfo.RedirectStandardOutput = true;
            p.StartInfo.RedirectStandardError = true;
            p.StartInfo.FileName = Path.Combine("ffmpeg", "ffmpeg.exe");
            p.StartInfo.Arguments = string.Format("-i \"{0}\" -c copy -bsf:a aac_adtstoasc {1}", inputFile, outputFile);
            p.StartInfo.WorkingDirectory = inputFile.Directory.FullName;
            p.Start();

            var errors = p.StandardError;
            var output = p.StandardOutput;
            Console.WriteLine(errors.ReadToEnd());
            Console.WriteLine(errors.ReadToEnd());
            p.WaitForExit();
            
            if (success)
                cache.PutContent("done", "", data.VideoId);
        }

        private void printProgress()
        {
            Stopwatch totalRuntime = new Stopwatch();
            totalRuntime.Start();
            int remainingUnits = totalUnits;
            int downloaded = this.downloaded;
            while (remainingUnits > 0)
            {
                TimeSpan overallDuration = new TimeSpan();
                foreach (var dur in durations)
                    overallDuration = overallDuration.Add(dur);

                double average = 0;
                long remainingMilli = 0;
                if (downloaded != 0)
                {
                    average = ((double)overallDuration.TotalSeconds) / downloaded;
                    var timePerUnit = (long)((double)totalRuntime.ElapsedMilliseconds / downloaded);
                    remainingMilli = timePerUnit * remainingUnits;
                }
                var remainingTime = TimeSpan.FromMilliseconds(remainingMilli);

                Console.WriteLine("Progress: remaining - {0}, {1} / {2}, active {3}, avg = {4}", 
                                   remainingTime, preExistingUnits + downloaded, totalUnits, active, average);
                Thread.Sleep((int)TimeSpan.FromSeconds(1).TotalMilliseconds);
                downloaded = this.downloaded;
                changesReporter.ReportProgress((downloaded + preExistingUnits) * 100 / totalUnits, new VideoProgress() { RemainingTime = remainingTime});
                remainingUnits = totalUnits - downloaded - preExistingUnits;
            }
        }

        private void parsePlaylist(string playlistM3U8)
        {
            chunks = getChunksFile(playlistM3U8);
            var allChunkNames = parseChunksFile(chunks.Content);
            chunkNames = new List<string>();

            foreach (var chunkName in allChunkNames)
            {
                if (!cache.HasFile(chunkName, data.VideoId))
                    chunkNames.Add(chunkName);
            }

            Console.WriteLine("There exists {0}/{1} of the needed chunks, downloading {2} chunks.", 
                               allChunkNames.Count - chunkNames.Count, allChunkNames.Count, chunkNames.Count);

            totalUnits = allChunkNames.Count;
            preExistingUnits = totalUnits - chunkNames.Count;
        }

        private ChunksFile getChunksFile(string playlistM3U8)
        {
            ChunksFile chunks = new ChunksFile();

            chunks.Url = cache.GetContent(data.formatChunksFileUrl(wantedQuality), data.VideoId);
            chunks.Content = cache.GetContent(data.formatChunksFileContent(wantedQuality), data.VideoId);

            if (chunks.Content == null || chunks.Url == null)
                chunks = fetchAndCacheChunksFile(playlistM3U8);

            return chunks;
        }

        int totalUnits = 0;
        int downloaded = 0;
        int active = 0;
        int preExistingUnits = 0;
        bool success = true;
        ConcurrentBag<TimeSpan> durations;
        private BackgroundWorker changesReporter;

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
                    var req = WebRequest.Create(baseUrl + "/" + chunkName);
                    var resp = req.GetResponse();
                    var chunkData = new BinaryReader(resp.GetResponseStream()).ReadBytes((int)resp.ContentLength);

                    cache.PutBinary(chunkName, chunkData, data.VideoId);
                    success = true;
                }
                catch
                {
                    tryNum++;
                    Console.WriteLine("Failed downloading '{0}' on {1}.", chunkName, data.VideoId);
                }
            while (tryNum < 3 && success == false);

            if (!success)
            {
                // TODO   
                success = false;
            }

            Interlocked.Increment(ref downloaded);
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
                    cache.PutContent(data.formatFFmpegFile(), wr.ToString(), data.VideoId);
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

            cache.PutContent(data.formatChunksFileUrl(wantedQuality), chunksUrl, data.VideoId);
            cache.PutContent(data.formatChunksFileContent(wantedQuality), chunksFile, data.VideoId);

            return new ChunksFile { Url = chunksUrl, Content = chunksFile };
        }

        private string getChunksFileUrl(string playlistM3U8)
        {
            SortedDictionary<int, string> qualityToChunkLists = new SortedDictionary<int, string>();

            var qualityTester = new Regex(String.Format(":BANDWIDTH=([0-9]*)", wantedQuality));
            var lines = playlistM3U8.Split('\n');
            for (int i = 0; i < lines.Length; i++)
            {
                if (qualityTester.IsMatch(lines[i]))
                {
                    string _qualityS = qualityTester.Match(lines[i]).Groups[1].Value;

                    if (_qualityS == wantedQuality)
                        return lines[i + 1];

                    int quality = 0;
                    int.TryParse(_qualityS, out quality);
                    string chunkList = lines[i + 1];
                    qualityToChunkLists[quality] = chunkList;
                }
            }

            if (qualityToChunkLists.ContainsKey(int.Parse(wantedQuality2nd)))
                return qualityToChunkLists[int.Parse(wantedQuality2nd)];

            // Can't find wanted qualities... going for 2nd best
            KeyValuePair<int, string>[] sortedKeys = new KeyValuePair<int, string>[qualityToChunkLists.Count];
            qualityToChunkLists.CopyTo(sortedKeys, 0);

            // There may not be a 2nd best... 
            if (sortedKeys.Length > 1)
                return sortedKeys[sortedKeys.Length - 2].Value;

            // This is an error. TODO: throw exception.
            if (sortedKeys.Length == 0)
                return null;

            return sortedKeys[0].Value;
        }
    }
}
