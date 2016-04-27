using CefSharp;
using CefSharp.WinForms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace openu_video_fetcher
{
    public partial class OpenuVidCatchingForm : Form
    {
        private const string LastViewKey = "last-viewed-page";
        private const string BrowserCache = "chrome_browser";

        public ChromiumWebBrowser browser;
        public CookieContainer jar;
        private UrlFileCache cache;
        public List<OpenuVideoData> playlist;
        private OpenuParser parser;

        public OpenuVidCatchingForm(CookieContainer _jar, UrlFileCache _cache)
        {
            jar = _jar;
            cache = _cache;

            var lastView = cache.GetContent(LastViewKey, BrowserCache);
            if (lastView == null)
                lastView = "http://openu.ac.il/sheilta";

            parser = new OpenuParser(jar, cache);

            InitializeComponent();
            AddDynamicComponents(lastView);
        }

        private void AddDynamicComponents(string url)
        {
            var settings = new CefSettings() { CachePath = cache.GetSubDir(BrowserCache), PersistSessionCookies = true };
            Cef.Initialize(settings);
            browser = new ChromiumWebBrowser(url);
            browser.LifeSpanHandler = new MainWindowRedirectHandler(browser);

            browser.Dock = DockStyle.Fill;
            browser.AddressChanged += syncAddressBarToBrowser;
            mainLayout.Controls.Add(browser, 0, 1);
        }

        private void syncAddressBarToBrowser(object sender, AddressChangedEventArgs e)
        {
            addressBar.Invoke((MethodInvoker)delegate { addressBar.Text = e.Address; });
        }

        public OpenuVidCatchingForm()
        {
            InitializeComponent();
        }

        private void addressBar_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                browser.Load(addressBar.Text);
            }
        }

        BackgroundWorker bg = new BackgroundWorker();

        private void downloadButton_Click(object sender, EventArgs e)
        {
            copyHtml();

            playlist.ForEach(x => {
                Console.WriteLine("Downloading {0} = {1}, at '{2}'", x.VideoId, x.Title, x.PlaylistUrl);
            });

            allFilesProgress.Step = 1;
            allFilesProgress.Maximum = playlist.Count;
            allFilesProgress.Value = 0;

            bg.DoWork += Bg_DoWork;
            bg.WorkerReportsProgress = true;
            bg.ProgressChanged += Bg_ProgressChanged;
            bg.RunWorkerCompleted += Bg_RunWorkerCompleted;

            try
            {
                bg.RunWorkerAsync();
            }
            catch
            {
                // TODO: prompt user, cancel or wait
            }
        }

        private void Bg_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            // TODO: this is not showing
            remainingTimeLabel.Text = "Done, Enjoy (-:";
        }

        private void Bg_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            if (e.UserState == this)
                allFilesProgress.PerformStep();
            else if (e.UserState is VideoProgress)
                remainingTimeLabel.Text = ((VideoProgress)e.UserState).RemainingTime.ToString();

            specificVideoProgress.Value = e.ProgressPercentage;
        }

        private void Bg_DoWork(object sender, DoWorkEventArgs e)
        {
            playlist.ForEach(video => {
                if (!cache.HasFile("done", video.VideoId))
                {
                    bg.ReportProgress(0, null);
                    new VideoDownloader(cache, video, bg).Download();
                }
                bg.ReportProgress(1, this);
            });
        }

        private void copyHtml()
        {
            Cef.GetGlobalCookieManager().VisitAllCookies(new CookieConverter(jar));
            cache.PutContent(LastViewKey, addressBar.Text, BrowserCache);
            var html = browser.GetMainFrame().GetSourceAsync().Result;

            playlist = parser.GetVideosInView(html);

            var videoChooser = new VideoChooserDialog(playlist);
            videoChooser.ShowDialog();
        }

        private void Browser_FrameLoadEnd(object sender, FrameLoadEndEventArgs e)
        {
            copyHtml();
        }
    }

    public class MainWindowRedirectHandler : ILifeSpanHandler
    {
        private ChromiumWebBrowser mainWindow;

        public MainWindowRedirectHandler(ChromiumWebBrowser browser)
        {
            this.mainWindow = browser;
        }

        public bool OnBeforePopup(IWebBrowser browserControl, IBrowser browser, IFrame frame, string targetUrl, string targetFrameName, WindowOpenDisposition targetDisposition, bool userGesture, IWindowInfo windowInfo, ref bool noJavascriptAccess, out IWebBrowser newBrowser)
        {
            newBrowser = null;
            mainWindow.Load(targetUrl);
            return true;
        }

        public void OnAfterCreated(IWebBrowser browserControl, IBrowser browser)
        {
        }

        public bool DoClose(IWebBrowser browserControl, IBrowser browser)
        {
            return false;
        }

        public void OnBeforeClose(IWebBrowser browserControl, IBrowser browser)
        {
        }
    }

    class CookieConverter : ICookieVisitor
    {
        private CookieContainer jar;

        public CookieConverter(CookieContainer jar)
        {
            this.jar = jar;
        }

        public bool Visit(CefSharp.Cookie cookie, int count, int total, ref bool deleteCookie)
        {
            jar.Add(new System.Net.Cookie(cookie.Name, cookie.Value, cookie.Path, cookie.Domain));
            return true;
        }
    }
}
