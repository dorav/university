using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace openu_video_fetcher
{
    public partial class VideoChooserDialog : Form
    {
        private List<OpenuVideoData> playlist;

        public VideoChooserDialog(List<OpenuVideoData> playlist)
        {
            this.playlist = playlist;
            InitializeComponent();
        }

        public ListBox.SelectedIndexCollection ChosenToDownload { get { return downloadPicker.SelectedIndices; } }

        class ViewVideoData
        {
            public OpenuVideoData Origin { get; set; }

            public bool ShouldDownload { get; set; }
            public string Title { get { return Origin.Title; } }
            public override string ToString()
            {
                return Title;
            }
        }

        private void VideoChooserDialog_Load(object sender, EventArgs e)
        {
            foreach (var video in playlist)
            {
                downloadPicker.Items.Add(new ViewVideoData { Origin = video, ShouldDownload = false }, false);
            }

            downloadPicker.ItemCheck += DownloadPicker_ItemCheck;
        }

        private void DownloadPicker_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            ((ViewVideoData)downloadPicker.Items[e.Index]).ShouldDownload = e.NewValue == CheckState.Checked;
        }

        private void VideoChooserDialog_FormClosing(object sender, FormClosingEventArgs e)
        {
            foreach (var _item in downloadPicker.Items)
            {
                var item = (ViewVideoData)_item;

                // Remove unselected items from list
                if (!item.ShouldDownload)
                    playlist.Remove(item.Origin);
            }
        }
    }
}
