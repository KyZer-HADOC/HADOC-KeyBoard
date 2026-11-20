using System.Diagnostics;

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
    private readonly NotifyIcon _tray;
    private readonly ToolStripMenuItem _statusItem;

    public TrayContext()
    {
        _statusItem = new ToolStripMenuItem("HADOC KeyBoard is installed") { Enabled = false };

        var settingsItem = new ToolStripMenuItem("Windows language settings", null, (_, _) => OpenLanguageSettings());
        var helpItem = new ToolStripMenuItem("How to switch keyboards", null, (_, _) =>
            MessageBox.Show(
                "Press Win + Space to open the Windows input switcher.\n\n" +
                "Select “Sinhala - HADOC KeyBoard” to type Singlish as Sinhala.\n" +
                "Select Default or Helakuru in the same switcher to change input method.",
                "HADOC KeyBoard", MessageBoxButtons.OK, MessageBoxIcon.Information));
        var exitItem = new ToolStripMenuItem("Close tray icon", null, (_, _) => ExitThread());

        var menu = new ContextMenuStrip();
        menu.Items.Add(_statusItem);
        menu.Items.Add(new ToolStripSeparator());
        menu.Items.Add(settingsItem);
        menu.Items.Add(helpItem);
        menu.Items.Add(new ToolStripSeparator());
        menu.Items.Add(exitItem);

        _tray = new NotifyIcon
        {
            Icon = SystemIcons.Application,
            Text = "HADOC KeyBoard - switch with Win + Space",
            ContextMenuStrip = menu,
            Visible = true
        };
        _tray.MouseClick += (_, e) =>
        {
            if (e.Button == MouseButtons.Left)
                OpenLanguageSettings();
        };

        _tray.ShowBalloonTip(5000, "HADOC KeyBoard",
            "Installed as a Windows input method. Press Win + Space and select Sinhala - HADOC KeyBoard.",
            ToolTipIcon.Info);
    }

    private static void OpenLanguageSettings()
    {
        try
        {
            Process.Start(new ProcessStartInfo("ms-settings:regionlanguage") { UseShellExecute = true });
        }
        catch (Exception ex)
        {
            MessageBox.Show(ex.Message, "HADOC KeyBoard", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }
    }

    protected override void ExitThreadCore()
    {
        _tray.Visible = false;
        _tray.Dispose();
        base.ExitThreadCore();
    }
}
