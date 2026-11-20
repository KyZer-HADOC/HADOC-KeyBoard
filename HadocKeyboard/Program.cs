using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Text;
using System.Runtime.InteropServices;

namespace HadocKeyboard;

internal static class Program
{
    [STAThread]
    private static void Main()
    {
        using var mutex = new Mutex(true, "HADOC-KeyBoard-SingleInstance", out bool created);
        if (!created) return;

        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new TrayContext());
    }
}

internal sealed class TrayContext : ApplicationContext
{
    private readonly KeyboardHook _hook;
    private readonly NotifyIcon _tray;
    private readonly Icon _onIcon;
    private readonly Icon _offIcon;
    private readonly ToolStripMenuItem _toggleItem;

    [DllImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool DestroyIcon(IntPtr hIcon);

    public TrayContext()
    {
        _onIcon = MakeIcon("සි", Color.FromArgb(0, 120, 215));
        _offIcon = MakeIcon("En", Color.Gray);

        try
        {
            _hook = new KeyboardHook();
        }
        catch (Exception ex)
        {
            _onIcon.Dispose();
            _offIcon.Dispose();
            MessageBox.Show(ex.Message, "HADOC KeyBoard", MessageBoxButtons.OK, MessageBoxIcon.Error);
            throw;
        }

        _toggleItem = new ToolStripMenuItem("Toggle Sinhala / English  (Win+Space / Ctrl+Space)", null, (_, _) => _hook.Toggle());
        var exitItem = new ToolStripMenuItem("Exit", null, (_, _) => ExitThread());
        var menu = new ContextMenuStrip();
        menu.Items.Add(_toggleItem);
        menu.Items.Add(new ToolStripSeparator());
        menu.Items.Add(exitItem);

        _tray = new NotifyIcon { ContextMenuStrip = menu, Visible = true };
        _tray.MouseClick += (_, e) => { if (e.Button == MouseButtons.Left) _hook.Toggle(); };
        _hook.EnabledChanged += RefreshTray;
        RefreshTray();

        _tray.ShowBalloonTip(3000, "HADOC KeyBoard",
            "Running. Press Win+Space or Ctrl+Space to switch Sinhala / English.", ToolTipIcon.Info);
    }

    private void RefreshTray()
    {
        _tray.Icon = _hook.Enabled ? _onIcon : _offIcon;
        _tray.Text = _hook.Enabled ? "HADOC KeyBoard: Sinhala" : "HADOC KeyBoard: English";
    }

    private static Icon MakeIcon(string text, Color back)
    {
        using var bmp = new Bitmap(32, 32);
        using (var g = Graphics.FromImage(bmp))
        {
            g.SmoothingMode = SmoothingMode.AntiAlias;
            g.TextRenderingHint = TextRenderingHint.AntiAlias;
            using var brush = new SolidBrush(back);
            g.FillEllipse(brush, 0, 0, 31, 31);
            using var font = new Font("Nirmala UI", text.Length > 1 && text[0] < 128 ? 12f : 15f, FontStyle.Bold, GraphicsUnit.Pixel);
            var size = g.MeasureString(text, font);
            g.DrawString(text, font, Brushes.White, (32 - size.Width) / 2f, (32 - size.Height) / 2f);
        }

        IntPtr hIcon = bmp.GetHicon();
        try
        {
            return (Icon)Icon.FromHandle(hIcon).Clone();
        }
        finally
        {
            DestroyIcon(hIcon);
        }
    }

    protected override void ExitThreadCore()
    {
        _tray.Visible = false;
        _tray.Dispose();
        _hook.Dispose();
        _onIcon.Dispose();
        _offIcon.Dispose();
        base.ExitThreadCore();
    }
}
