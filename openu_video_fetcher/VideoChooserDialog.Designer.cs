namespace openu_video_fetcher
{
    partial class VideoChooserDialog
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
            this.formLayout = new System.Windows.Forms.TableLayoutPanel();
            this.viewLayout = new System.Windows.Forms.TableLayoutPanel();
            this.downloadPicker = new System.Windows.Forms.CheckedListBox();
            this.downloadPickedLabel = new System.Windows.Forms.Label();
            this.chooseButton = new System.Windows.Forms.Button();
            this.formLayout.SuspendLayout();
            this.viewLayout.SuspendLayout();
            this.SuspendLayout();
            // 
            // formLayout
            // 
            this.formLayout.AutoSize = true;
            this.formLayout.ColumnCount = 1;
            this.formLayout.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.formLayout.Controls.Add(this.viewLayout, 0, 0);
            this.formLayout.Controls.Add(this.chooseButton, 0, 1);
            this.formLayout.Dock = System.Windows.Forms.DockStyle.Fill;
            this.formLayout.Location = new System.Drawing.Point(0, 0);
            this.formLayout.Name = "formLayout";
            this.formLayout.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.formLayout.RowCount = 2;
            this.formLayout.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.formLayout.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 45F));
            this.formLayout.Size = new System.Drawing.Size(271, 362);
            this.formLayout.TabIndex = 0;
            // 
            // viewLayout
            // 
            this.viewLayout.AutoSize = true;
            this.viewLayout.ColumnCount = 2;
            this.viewLayout.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.viewLayout.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.viewLayout.Controls.Add(this.downloadPicker, 1, 0);
            this.viewLayout.Controls.Add(this.downloadPickedLabel, 0, 0);
            this.viewLayout.Dock = System.Windows.Forms.DockStyle.Fill;
            this.viewLayout.Location = new System.Drawing.Point(3, 3);
            this.viewLayout.Name = "viewLayout";
            this.viewLayout.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.viewLayout.RowCount = 1;
            this.viewLayout.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.viewLayout.Size = new System.Drawing.Size(265, 311);
            this.viewLayout.TabIndex = 1;
            // 
            // downloadPicker
            // 
            this.downloadPicker.CheckOnClick = true;
            this.downloadPicker.Dock = System.Windows.Forms.DockStyle.Fill;
            this.downloadPicker.FormattingEnabled = true;
            this.downloadPicker.Location = new System.Drawing.Point(3, 3);
            this.downloadPicker.Name = "downloadPicker";
            this.downloadPicker.Size = new System.Drawing.Size(138, 305);
            this.downloadPicker.TabIndex = 3;
            // 
            // downloadPickedLabel
            // 
            this.downloadPickedLabel.AutoSize = true;
            this.downloadPickedLabel.Location = new System.Drawing.Point(147, 10);
            this.downloadPickedLabel.Margin = new System.Windows.Forms.Padding(3, 10, 3, 0);
            this.downloadPickedLabel.Name = "downloadPickedLabel";
            this.downloadPickedLabel.Size = new System.Drawing.Size(115, 13);
            this.downloadPickedLabel.TabIndex = 0;
            this.downloadPickedLabel.Text = "בחר הרצאות להורדה:";
            // 
            // chooseButton
            // 
            this.chooseButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.chooseButton.Dock = System.Windows.Forms.DockStyle.Fill;
            this.chooseButton.Location = new System.Drawing.Point(3, 320);
            this.chooseButton.Name = "chooseButton";
            this.chooseButton.Size = new System.Drawing.Size(265, 39);
            this.chooseButton.TabIndex = 2;
            this.chooseButton.Text = "button1";
            this.chooseButton.UseVisualStyleBackColor = true;
            // 
            // VideoChooserDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(271, 362);
            this.Controls.Add(this.formLayout);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "VideoChooserDialog";
            this.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.Text = "Download Picker";
            this.TopMost = true;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.VideoChooserDialog_FormClosing);
            this.Load += new System.EventHandler(this.VideoChooserDialog_Load);
            this.formLayout.ResumeLayout(false);
            this.formLayout.PerformLayout();
            this.viewLayout.ResumeLayout(false);
            this.viewLayout.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel formLayout;
        private System.Windows.Forms.TableLayoutPanel viewLayout;
        private System.Windows.Forms.CheckedListBox downloadPicker;
        private System.Windows.Forms.Label downloadPickedLabel;
        private System.Windows.Forms.Button chooseButton;
    }
}