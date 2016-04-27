using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace openu_video_fetcher
{
    public class OpenuVideoData
    {
        public string PlaylistUrl { get; set; }
        public string VideoId { get; set; }
        public string Title { get; set; }
        public string ContextId { get; internal set; }
        public string CourseId { get; internal set; }

        public string formatCacheFileName()
        {
            return string.Format("{0}", Title);
        }

        public string formatPlaylistFile()
        {
            return formatCacheFileName() + "_options.m3u8";
        }

        private string formatChunksFile(string quality)
        {
            return formatCacheFileName() + "_chunks_" + quality + ".m3u8";
        }

        internal string formatChunksFileUrl(string wantedQuality)
        {
            return formatChunksFile(wantedQuality) + "_url";
        }

        internal string formatChunksFileContent(string wantedQuality)
        {
            return "chunklist.m3u8";
        }

        internal string formatFFmpegFile()
        {
            return formatCacheFileName() + ".m3u8";
        }

        internal string formatOutputFile()
        {
            return "combined_output.mp4";
        }
    }
}
