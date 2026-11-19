using System.Runtime.InteropServices;
using System.Text;

namespace HadocKeyboard;

/// <summary>
/// Global keyboard hook. While enabled, letters are collected into a Latin "word"
/// buffer; after every key the word is converted to Sinhala and the already-typed
/// text is corrected by sending Backspace + Unicode characters.
/// Nothing is logged, stored or sent anywhere.
/// </summary>
internal sealed class KeyboardHook : IDisposable
{
    private const int WH_KEYBOARD_LL = 13;
    private const int WH_MOUSE_LL = 14;
    private const int WM_KEYDOWN = 0x0100;
    private const int WM_SYSKEYDOWN = 0x0104;
    private const int WM_LBUTTONDOWN = 0x0201;
    private const int WM_RBUTTONDOWN = 0x0204;
    private const int WM_MBUTTONDOWN = 0x0207;
    private const uint LLKHF_INJECTED = 0x10;
    private const uint INPUT_KEYBOARD = 1;
    private const uint KEYEVENTF_KEYUP = 0x0002;
    private const uint KEYEVENTF_UNICODE = 0x0004;

    private const int VK_BACK = 0x08, VK_CAPITAL = 0x14, VK_SPACE = 0x20;
    private const int VK_SHIFT = 0x10, VK_CONTROL = 0x11, VK_MENU = 0x12;
    private const int VK_LWIN = 0x5B, VK_RWIN = 0x5C;

    private delegate IntPtr HookProc(int nCode, IntPtr wParam, IntPtr lParam);

    [StructLayout(LayoutKind.Sequential)]
    private struct KBDLLHOOKSTRUCT { public uint vkCode, scanCode, flags, time; public UIntPtr dwExtraInfo; }

    [StructLayout(LayoutKind.Sequential)]
    private struct MOUSEINPUT { public int dx, dy; public uint mouseData, dwFlags, time; public UIntPtr dwExtraInfo; }

    [StructLayout(LayoutKind.Sequential)]
    private struct KEYBDINPUT { public ushort wVk, wScan; public uint dwFlags, time; public UIntPtr dwExtraInfo; }

    [StructLayout(LayoutKind.Explicit)]
    private struct InputUnion
    {
        [FieldOffset(0)] public MOUSEINPUT mi;
        [FieldOffset(0)] public KEYBDINPUT ki;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct INPUT { public uint type; public InputUnion u; }

    [DllImport("user32.dll", SetLastError = true)]
    private static extern IntPtr SetWindowsHookEx(int idHook, HookProc lpfn, IntPtr hMod, uint dwThreadId);
    [DllImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool UnhookWindowsHookEx(IntPtr hhk);
    [DllImport("user32.dll")]
    private static extern IntPtr CallNextHookEx(IntPtr hhk, int nCode, IntPtr wParam, IntPtr lParam);
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
    private static extern IntPtr GetModuleHandle(string? lpModuleName);
    [DllImport("user32.dll")]
    private static extern short GetAsyncKeyState(int vKey);
    [DllImport("user32.dll")]
    private static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll", SetLastError = true)]
    private static extern uint SendInput(uint nInputs, INPUT[] pInputs, int cbSize);

    private readonly HookProc _kbProc;
    private readonly HookProc _mouseProc;
    private IntPtr _kbHook = IntPtr.Zero;
    private IntPtr _mouseHook = IntPtr.Zero;

    private readonly StringBuilder _buffer = new();
    private string _emitted = "";
    private IntPtr _lastWindow = IntPtr.Zero;

    public bool Enabled { get; private set; } = true;
    public event Action? EnabledChanged;

    public KeyboardHook()
    {
        _kbProc = KeyboardProc;
        _mouseProc = MouseProc;
        var module = GetModuleHandle(null);
        _kbHook = SetWindowsHookEx(WH_KEYBOARD_LL, _kbProc, module, 0);
        _mouseHook = SetWindowsHookEx(WH_MOUSE_LL, _mouseProc, module, 0);
        if (_kbHook == IntPtr.Zero)
            throw new InvalidOperationException("Could not install keyboard hook.");
    }

    public void Toggle()
    {
        Enabled = !Enabled;
        Reset();
        EnabledChanged?.Invoke();
    }

    private void Reset()
    {
        _buffer.Clear();
        _emitted = "";
    }

    private static bool Down(int vk) => (GetAsyncKeyState(vk) & 0x8000) != 0;

    private IntPtr MouseProc(int nCode, IntPtr wParam, IntPtr lParam)
    {
        if (nCode >= 0)
        {
            int msg = (int)wParam;
            if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
                Reset(); // caret moved: start a new word
        }
        return CallNextHookEx(_mouseHook, nCode, wParam, lParam);
    }

    private IntPtr KeyboardProc(int nCode, IntPtr wParam, IntPtr lParam)
    {
        if (nCode >= 0)
        {
            var k = Marshal.PtrToStructure<KBDLLHOOKSTRUCT>(lParam);
            bool injected = (k.flags & LLKHF_INJECTED) != 0;
            int msg = (int)wParam;
            if (!injected && (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN))
            {
                try
                {
                    if (HandleKeyDown((int)k.vkCode)) return (IntPtr)1; // swallow
                }
                catch
                {
                    Reset();
                }
            }
        }
        return CallNextHookEx(_kbHook, nCode, wParam, lParam);
    }

    private bool HandleKeyDown(int vk)
    {
        bool ctrl = Down(VK_CONTROL);
        bool alt = Down(VK_MENU);
        bool win = Down(VK_LWIN) || Down(VK_RWIN);

        // Ctrl+Space toggles Sinhala / English
        if (vk == VK_SPACE && ctrl && !alt && !win)
        {
            Toggle();
            return true;
        }

        if (!Enabled) return false;

        // Modifier keys alone never end a word
        if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU || vk == VK_CAPITAL ||
            (vk >= 0xA0 && vk <= 0xA5) || vk == VK_LWIN || vk == VK_RWIN)
            return false;

        var fg = GetForegroundWindow();
        if (fg != _lastWindow) { Reset(); _lastWindow = fg; }

        if (ctrl || alt || win) { Reset(); return false; }

        if (vk >= 0x41 && vk <= 0x5A) // A-Z
        {
            bool shift = Down(VK_SHIFT);
            _buffer.Append((char)(shift ? vk : vk + 32)); // Caps Lock is ignored
            Update();
            return true;
        }

        if (vk == VK_BACK && _buffer.Length > 0)
        {
            _buffer.Length--;
            Update();
            return true;
        }

        // space, enter, digits, punctuation, arrows ... end the current word
        Reset();
        return false;
    }

    private void Update()
    {
        string next = SinhalaEngine.Convert(_buffer.ToString());

        int common = 0;
        int max = Math.Min(_emitted.Length, next.Length);
        while (common < max && _emitted[common] == next[common]) common++;

        var inputs = new List<INPUT>();
        for (int i = 0; i < _emitted.Length - common; i++)
        {
            inputs.Add(Key(VK_BACK, 0, 0));
            inputs.Add(Key(VK_BACK, 0, KEYEVENTF_KEYUP));
        }
        for (int i = common; i < next.Length; i++)
        {
            inputs.Add(Key(0, next[i], KEYEVENTF_UNICODE));
            inputs.Add(Key(0, next[i], KEYEVENTF_UNICODE | KEYEVENTF_KEYUP));
        }
        if (inputs.Count > 0)
            SendInput((uint)inputs.Count, inputs.ToArray(), Marshal.SizeOf<INPUT>());

        _emitted = next;
    }

    private static INPUT Key(ushort vk, ushort scan, uint flags) => new()
    {
        type = INPUT_KEYBOARD,
        u = new InputUnion
        {
            ki = new KEYBDINPUT { wVk = vk, wScan = scan, dwFlags = flags, time = 0, dwExtraInfo = UIntPtr.Zero }
        }
    };

    public void Dispose()
    {
        if (_kbHook != IntPtr.Zero) { UnhookWindowsHookEx(_kbHook); _kbHook = IntPtr.Zero; }
        if (_mouseHook != IntPtr.Zero) { UnhookWindowsHookEx(_mouseHook); _mouseHook = IntPtr.Zero; }
    }
}
