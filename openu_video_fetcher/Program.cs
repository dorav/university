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
using System.Windows.Forms;
using System.Threading;
using CefSharp.WinForms;
using CefSharp;

namespace openu_video_fetcher
{
    public class ProgramArgs
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

        private static List<OpenuVideoData> runBrowserThread(UrlFileCache cache)
        {
            List<OpenuVideoData> toDownload = null;
            var th = new Thread(() => {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);

                OpenuVidCatchingForm simpleWebForm = new OpenuVidCatchingForm(cookieJar, cache);
                simpleWebForm.Show();
                Application.Run(simpleWebForm);

                toDownload = simpleWebForm.playlist;

                Console.WriteLine("Done showing");
            });
            th.SetApartmentState(ApartmentState.STA);
            th.Start();
            th.Join();

            return toDownload;
        }

        static CookieContainer cookieJar = new CookieContainer();

        [STAThread]
        static void Main(string[] args)
        {
            var videos = Environment.GetFolderPath(Environment.SpecialFolder.MyVideos);
            var directoryInfo = new DirectoryInfo(Path.Combine(videos, "openu-vid-fetcher"));
            UrlFileCache cache = new UrlFileCache(directoryInfo);

            runBrowserThread(cache);
        }
    }
}
