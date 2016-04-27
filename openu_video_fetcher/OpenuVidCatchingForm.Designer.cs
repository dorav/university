namespace openu_video_fetcher
{
    partial class OpenuVidCatchingForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.mainLayout = new System.Windows.Forms.TableLayoutPanel();
            this.controlsLayout = new System.Windows.Forms.TableLayoutPanel();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.downloadButton = new System.Windows.Forms.Button();
            this.addressLabel = new System.Windows.Forms.Label();
            this.addressBar = new System.Windows.Forms.TextBox();
            this.specificVideoProgress = new System.Windows.Forms.ProgressBar();
            this.allFilesProgress = new System.Windows.Forms.ProgressBar();
            this.remainingTimeLabel = new System.Windows.Forms.Label();
            this.mainLayout.SuspendLayout();
            this.controlsLayout.SuspendLayout();
            this.flowLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainLayout
            // 
            this.mainLayout.ColumnCount = 1;
            this.mainLayout.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.mainLayout.Controls.Add(this.controlsLayout, 0, 0);
            this.mainLayout.Controls.Add(this.specificVideoProgress, 0, 2);
            this.mainLayout.Controls.Add(this.allFilesProgress, 0, 3);
            this.mainLayout.Controls.Add(this.remainingTimeLabel, 0, 4);
            this.mainLayout.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mainLayout.Location = new System.Drawing.Point(0, 0);
            this.mainLayout.Name = "mainLayout";
            this.mainLayout.RowCount = 5;
            this.mainLayout.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 40F));
            this.mainLayout.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.mainLayout.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.mainLayout.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 40F));
            this.mainLayout.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 30F));
            this.mainLayout.Size = new System.Drawing.Size(554, 454);
            this.mainLayout.TabIndex = 0;
            // 
            // controlsLayout
            // 
            this.controlsLayout.ColumnCount = 2;
            this.controlsLayout.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 137F));
            this.controlsLayout.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.controlsLayout.Controls.Add(this.flowLayoutPanel1, 0, 0);
            this.controlsLayout.Controls.Add(this.addressBar, 1, 0);
            this.controlsLayout.Dock = System.Windows.Forms.DockStyle.Fill;
            this.controlsLayout.Location = new System.Drawing.Point(3, 3);
            this.controlsLayout.Name = "controlsLayout";
            this.controlsLayout.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.controlsLayout.RowCount = 1;
            this.controlsLayout.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.controlsLayout.Size = new System.Drawing.Size(548, 34);
            this.controlsLayout.TabIndex = 0;
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.Controls.Add(this.downloadButton);
            this.flowLayoutPanel1.Controls.Add(this.addressLabel);
            this.flowLayoutPanel1.Location = new System.Drawing.Point(415, 3);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.flowLayoutPanel1.Size = new System.Drawing.Size(130, 27);
            this.flowLayoutPanel1.TabIndex = 2;
            // 
            // downloadButton
            // 
            this.downloadButton.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.downloadButton.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            this.downloadButton.FlatAppearance.BorderColor = System.Drawing.SystemColors.ControlDark;
            this.downloadButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.downloadButton.Location = new System.Drawing.Point(52, 3);
            this.downloadButton.Name = "downloadButton";
            this.downloadButton.Size = new System.Drawing.Size(75, 23);
            this.downloadButton.TabIndex = 0;
            this.downloadButton.Text = "הורד";
            this.downloadButton.UseVisualStyleBackColor = false;
            this.downloadButton.Click += new System.EventHandler(this.downloadButton_Click);
            // 
            // addressLabel
            // 
            this.addressLabel.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.addressLabel.AutoSize = true;
            this.addressLabel.Location = new System.Drawing.Point(3, 8);
            this.addressLabel.Name = "addressLabel";
            this.addressLabel.Size = new System.Drawing.Size(43, 13);
            this.addressLabel.TabIndex = 1;
            this.addressLabel.Text = "כתובת:";
            // 
            // addressBar
            // 
            this.addressBar.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.addressBar.Location = new System.Drawing.Point(3, 7);
            this.addressBar.Name = "addressBar";
            this.addressBar.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.addressBar.Size = new System.Drawing.Size(405, 20);
            this.addressBar.TabIndex = 3;
            this.addressBar.KeyDown += new System.Windows.Forms.KeyEventHandler(this.addressBar_KeyDown);
            // 
            // specificVideoProgress
            // 
            this.specificVideoProgress.Dock = System.Windows.Forms.DockStyle.Fill;
            this.specificVideoProgress.Location = new System.Drawing.Point(3, 367);
            this.specificVideoProgress.Name = "specificVideoProgress";
            this.specificVideoProgress.Size = new System.Drawing.Size(548, 14);
            this.specificVideoProgress.TabIndex = 1;
            // 
            // allFilesProgress
            // 
            this.allFilesProgress.Dock = System.Windows.Forms.DockStyle.Fill;
            this.allFilesProgress.Location = new System.Drawing.Point(3, 387);
            this.allFilesProgress.Name = "allFilesProgress";
            this.allFilesProgress.Size = new System.Drawing.Size(548, 34);
            this.allFilesProgress.TabIndex = 2;
            // 
            // remainingTimeLabel
            // 
            this.remainingTimeLabel.AutoSize = true;
            this.remainingTimeLabel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.remainingTimeLabel.Font = new System.Drawing.Font("MS Gothic", 20F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.World, ((byte)(0)));
            this.remainingTimeLabel.Location = new System.Drawing.Point(3, 424);
            this.remainingTimeLabel.Name = "remainingTimeLabel";
            this.remainingTimeLabel.Size = new System.Drawing.Size(548, 30);
            this.remainingTimeLabel.TabIndex = 3;
            this.remainingTimeLabel.Text = "∞";
            this.remainingTimeLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // OpenuVidCatchingForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            this.ClientSize = new System.Drawing.Size(554, 454);
            this.Controls.Add(this.mainLayout);
            this.ForeColor = System.Drawing.SystemColors.ControlLightLight;
            this.Name = "OpenuVidCatchingForm";
            this.Text = "Form1";
            this.mainLayout.ResumeLayout(false);
            this.mainLayout.PerformLayout();
            this.controlsLayout.ResumeLayout(false);
            this.controlsLayout.PerformLayout();
            this.flowLayoutPanel1.ResumeLayout(false);
            this.flowLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel mainLayout;
        private System.Windows.Forms.TableLayoutPanel controlsLayout;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.Button downloadButton;
        private System.Windows.Forms.Label addressLabel;
        private System.Windows.Forms.TextBox addressBar;
        private System.Windows.Forms.ProgressBar specificVideoProgress;
        private System.Windows.Forms.ProgressBar allFilesProgress;
        private System.Windows.Forms.Label remainingTimeLabel;
    }
}