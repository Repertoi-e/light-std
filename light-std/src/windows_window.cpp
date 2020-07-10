#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/file.h"
#include "lstd/io/fmt.h"
#include "lstd/os.h"
#include "lstd/video/window.h"

// @Hack
#undef TRANSPARENT

// @Hack: Define macros that some Windows.h variants don't
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif
#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDATA 0x0049
#endif
#ifndef WM_UNICHAR
#define WM_UNICHAR 0x0109
#endif
#ifndef UNICODE_NOCHAR
#define UNICODE_NOCHAR 0xFFFF
#endif
#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif
#ifndef GET_XBUTTON_WPARAM
#define GET_XBUTTON_WPARAM(w) (HIWORD(w))
#endif
#ifndef EDS_ROTATEDMODE
#define EDS_ROTATEDMODE 0x00000004
#endif
#ifndef DISPLAY_DEVICE_ACTIVE
#define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif
#ifndef _WIN32_WINNT_WINBLUE
#define _WIN32_WINNT_WINBLUE 0x0602
#endif
#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8 0x0602
#endif
#ifndef WM_GETDPISCALEDSIZE
#define WM_GETDPISCALEDSIZE 0x02e4
#endif
#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI 96
#endif

LSTD_BEGIN_NAMESPACE

extern BOOL is_windows_10_build_or_greater(WORD build);
#define IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER() is_windows_10_build_or_greater(14393)
#define IS_WINDOWS_10_CREATORS_UPDATE_OR_GREATER() is_windows_10_build_or_greater(15063)

extern void win32_poll_monitors();

static s32 AcquiredMonitorCount = 0;
static u32 MouseTrailSize = 0;

static window *DisabledCursorWindow = null;
static vec2<s32> RestoreCursorPos = {no_init};

static window *WindowsList = null;
cursor *CursorsList = null;

wchar_t *g_Win32WindowClassName = null;

void win32_window_init() {
    g_MonitorEvent.connect([](const monitor_event &e) {
        if (e.Action == monitor_event::CONNECTED) return;

        window *win = WindowsList;
        while (win) {
            if (win->Monitor == e.Monitor) {
                vec2<s32> size = win->get_size();
                win->set_fullscreen(null, size.x, size.y);
            }
            win = win->Next;
        }
    });
}

void win32_window_uninit() {
    auto *win = WindowsList;
    while (win) {
        win->release();
        win = win->Next;
    }
    g_MonitorEvent.release();
}

static DWORD get_window_style(window *win) {
    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (win->Monitor) {
        style |= WS_POPUP;
    } else {
        style |= WS_SYSMENU | WS_MINIMIZEBOX;

        if (win->Flags & window::BORDERLESS) {
            style |= WS_POPUP;

        } else {
            style |= WS_CAPTION;
            if (win->Flags & window::RESIZABLE) style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
        }
    }
    return style;
}

static DWORD get_window_ex_style(window *win) {
    DWORD style = WS_EX_APPWINDOW;
    if (win->Monitor || win->Flags & window::ALWAYS_ON_TOP) style |= WS_EX_TOPMOST;
    return style;
}

static void update_framebuffer_transparency(window *win) {
    BOOL enabled;
    if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled) {
        HRGN region = CreateRectRgn(0, 0, -1, -1);

        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = true;

        if (SUCCEEDED(DwmEnableBlurBehindWindow(win->PlatformData.Win32.hWnd, &bb))) {
            // Decorated windows don't repaint the transparent background leaving a trail behind animations.
            // @Hack: Making the window layered with a transparency color key
            //        seems to fix this.  Normally, when specifying
            //        a transparency color key to be used when composing the
            //        layered window, all pixels painted by the window in this
            //        color will be transparent.  That doesn't seem to be the
            //        case anymore, at least when used with blur behind window
            //        plus negative region.
            LONG exStyle = GetWindowLongW(win->PlatformData.Win32.hWnd, GWL_EXSTYLE);
            exStyle |= WS_EX_LAYERED;
            SetWindowLongW(win->PlatformData.Win32.hWnd, GWL_EXSTYLE, exStyle);

            // Using a color key not equal to black to fix the trailing
            // issue.  When set to black, something is making the hit test
            // not resize with the window frame.
            SetLayeredWindowAttributes(win->PlatformData.Win32.hWnd, RGB(0, 193, 48), 255, LWA_COLORKEY);
        }
        DeleteObject(region);
    } else {
        LONG exStyle = GetWindowLongW(win->PlatformData.Win32.hWnd, GWL_EXSTYLE);
        exStyle &= ~WS_EX_LAYERED;
        SetWindowLongW(win->PlatformData.Win32.hWnd, GWL_EXSTYLE, exStyle);
        RedrawWindow(win->PlatformData.Win32.hWnd, null, null, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
    }
}

static vec2<s32> get_full_window_size(DWORD style, DWORD exStyle, s32 contentWidth, s32 contentHeight, u32 dpi) {
    RECT rect = {0, 0, contentWidth, contentHeight};

    if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) {
        AdjustWindowRectExForDpi(&rect, style, false, exStyle, dpi);
    } else {
        AdjustWindowRectEx(&rect, style, false, exStyle);
    }
    return {rect.right - rect.left, rect.bottom - rect.top};
}

window *window::init(const string &title, s32 x, s32 y, s32 width, s32 height, u32 flags) {
    DisplayMode.Width = width;
    DisplayMode.Height = height;
    DisplayMode.RedBits = DisplayMode.GreenBits = DisplayMode.BlueBits = 8;
    DisplayMode.RefreshRate = DONT_CARE;

    Flags = flags & CREATION_FLAGS;

    DWORD style = get_window_style(this);
    DWORD exStyle = get_window_ex_style(this);

    vec2<s32> fullSize = get_full_window_size(style, exStyle, width, height, USER_DEFAULT_SCREEN_DPI);

    s32 xpos = x == DONT_CARE ? CW_USEDEFAULT : x;
    s32 ypos = y == DONT_CARE ? CW_USEDEFAULT : y;
    if (x == CENTERED) xpos = (os_get_primary_monitor()->CurrentMode.Width - fullSize.x) / 2;
    if (y == CENTERED) ypos = (os_get_primary_monitor()->CurrentMode.Height - fullSize.y) / 2;

    PlatformData.Win32.hWnd = CreateWindowExW(exStyle, g_Win32WindowClassName, L"", style, xpos, ypos, fullSize.x,
                                              fullSize.y, null, null, GetModuleHandleW(null), null);

    if (!PlatformData.Win32.hWnd) {
        fmt::print("(windows_window.cpp): Failed to create window\n");
        return null;
    }

    set_title(title);

    SetPropW(PlatformData.Win32.hWnd, L"LSTD", this);

    ChangeWindowMessageFilterEx(PlatformData.Win32.hWnd, WM_DROPFILES, MSGFLT_ALLOW, null);
    ChangeWindowMessageFilterEx(PlatformData.Win32.hWnd, WM_COPYDATA, MSGFLT_ALLOW, null);
    ChangeWindowMessageFilterEx(PlatformData.Win32.hWnd, WM_COPYGLOBALDATA, MSGFLT_ALLOW, null);
    DragAcceptFiles(PlatformData.Win32.hWnd, true);

    RECT rect = {0, 0, width, height};
    ClientToScreen(PlatformData.Win32.hWnd, (POINT *) &rect.left);
    ClientToScreen(PlatformData.Win32.hWnd, (POINT *) &rect.right);

    if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) {
        AdjustWindowRectExForDpi(&rect, style, false, exStyle, GetDpiForWindow(PlatformData.Win32.hWnd));
    } else {
        AdjustWindowRectEx(&rect, style, false, exStyle);
    }

    WINDOWPLACEMENT wp = {sizeof(wp)};
    GetWindowPlacement(PlatformData.Win32.hWnd, &wp);
    wp.rcNormalPosition = rect;
    wp.showCmd = SW_HIDE;
    SetWindowPlacement(PlatformData.Win32.hWnd, &wp);

    if (Flags & window::ALPHA) update_framebuffer_transparency(this);
    if (Flags & SHOWN) show();

    // If we couldn't get transparent, remove the flag
    BOOL enabled;
    if (FAILED(DwmIsCompositionEnabled(&enabled)) || !enabled) {
        Flags &= ~window::ALPHA;
    }

    zero_memory(Keys.Data, sizeof(Keys.Data));
    zero_memory(LastFrameKeys.Data, sizeof(LastFrameKeys.Data));
    zero_memory(KeysThisFrame.Data, sizeof(KeysThisFrame.Data));
    zero_memory(MouseButtons.Data, sizeof(MouseButtons.Data));
    zero_memory(LastFrameMouseButtons.Data, sizeof(LastFrameMouseButtons.Data));
    zero_memory(MouseButtonsThisFrame.Data, sizeof(MouseButtonsThisFrame.Data));

    ID = s_NextID;
    atomic_inc(&s_NextID);

    Next = WindowsList;
    WindowsList = this;

    return this;
}

static void do_key_input_event(window *win, u32 key, bool pressed, bool asyncMods = false) {
    assert(key <= Key_Last);

    if (!pressed && !win->Keys[key]) return;

    bool repeated = false;

    bool wasPressed = win->Keys[key];
    win->Keys[key] = pressed;
    if (pressed && wasPressed) repeated = true;

    event e;
    e.Window = win;
    if (pressed) {
        e.Type = repeated ? event::Keyboard_Repeated : event::Keyboard_Pressed;
    } else {
        e.Type = event::Keyboard_Released;
    }
    e.KeyCode = key;
    win->Event.emit(null, e);
}

static void do_mouse_input_event(window *win, u32 button, bool pressed, bool doubleClick = false) {
    assert(button <= Mouse_Button_Last);
    win->MouseButtons[button] = pressed;

    vec2<s32> pos = win->get_cursor_pos();

    event e;
    e.Window = win;
    e.Type = pressed ? event::Mouse_Button_Pressed : event::Mouse_Button_Released;
    e.Button = button;
    e.DoubleClicked = doubleClick;
    win->Event.emit(null, e);
}

static void do_mouse_move(window *win, vec2<s32> pos) {
    if (win->VirtualCursorPos == pos) return;

    vec2<s32> delta = pos - win->VirtualCursorPos;
    win->VirtualCursorPos = pos;

    event e;
    e.Window = win;
    e.Type = event::Mouse_Moved;
    e.X = pos.x;
    e.Y = pos.y;
    e.DX = delta.x;
    e.DY = delta.y;
    win->Event.emit(null, e);
}

void window::update() {
    MSG message;
    while (PeekMessageW(&message, null, 0, 0, PM_REMOVE) > 0) {
        if (message.message == WM_QUIT) {
            auto *win = WindowsList;
            while (win) {
                win->release();
                win = win->Next;
            }
        } else {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    HWND handle = GetActiveWindow();
    if (handle) {
        // @Hack: Shift keys on Windows tend to "stick" when both are pressed as
        //        no key up message is generated by the first key release
        //        The other half of this is in the handling of WM_KEYUP
        // :ShiftHack The other half of this is in WM_SYSKEYUP
        auto *win = (window *) GetPropW(handle, L"LSTD");
        if (win) {
            bool lshift = (GetAsyncKeyState(VK_LSHIFT) >> 15) & 1;
            bool rshift = (GetAsyncKeyState(VK_RSHIFT) >> 15) & 1;

            if (!lshift && win->Keys[Key_LeftShift]) {
                do_key_input_event(win, Key_LeftShift, false, true);
            } else if (!rshift && win->Keys[Key_RightShift]) {
                do_key_input_event(win, Key_RightShift, false, true);
            }
        }
    }

    auto *win = WindowsList;
    while (win) {
        For(range(Key_Last + 1)) { win->KeysThisFrame[it] = win->Keys[it] && !win->LastFrameKeys[it]; }
        For(range(Mouse_Button_Last + 1)) {
            win->MouseButtonsThisFrame[it] = win->MouseButtons[it] && !win->LastFrameMouseButtons[it];
        }
        win->LastFrameKeys = win->Keys;
        win->LastFrameMouseButtons = win->MouseButtons;

        win = win->Next;
    }

    win = DisabledCursorWindow;
    if (win) {
        vec2<s32> size = win->get_size();
        if (win->PlatformData.Win32.LastCursorPos != size / 2) win->set_cursor_pos(size / 2);
    }

    Context.TemporaryAlloc.free_all();
}

static void acquire_monitor(window *win) {
    if (!AcquiredMonitorCount) {
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);

        SystemParametersInfo(SPI_GETMOUSETRAILS, 0, &MouseTrailSize, 0);
        SystemParametersInfo(SPI_SETMOUSETRAILS, 0, 0, 0);
    }

    if (!win->Monitor->Window) ++AcquiredMonitorCount;

    os_set_display_mode(win->Monitor, win->DisplayMode);
    win->Monitor->Window = win;
}

// Remove the window and restore the original video mode
static void release_monitor(window *win) {
    if (win->Monitor->Window != win) return;

    --AcquiredMonitorCount;
    if (!AcquiredMonitorCount) {
        SetThreadExecutionState(ES_CONTINUOUS);

        // @Hack: Restore mouse trail length saved in acquireMonitor
        SystemParametersInfo(SPI_SETMOUSETRAILS, MouseTrailSize, 0, 0);
    }

    win->Monitor->Window = null;
    os_restore_display_mode(win->Monitor);
}

void window::release() {
    if (ID == INVALID_ID) return;

    IsDestroying = true;

    event e;
    e.Window = this;
    e.Type = event::Window_Closed;
    Event.emit(null, e);

    if (Monitor) release_monitor(this);
    if (DisabledCursorWindow == this) DisabledCursorWindow = null;

    if (PlatformData.Win32.hWnd) {
        RemovePropW(PlatformData.Win32.hWnd, L"LSTD");
        DestroyWindow(PlatformData.Win32.hWnd);
        PlatformData.Win32.hWnd = null;
    }

    if (PlatformData.Win32.BigIcon) DestroyIcon(PlatformData.Win32.BigIcon);
    if (PlatformData.Win32.SmallIcon) DestroyIcon(PlatformData.Win32.SmallIcon);

    window **prev = &WindowsList;
    while (*prev != this) prev = &((*prev)->Next);
    *prev = this->Next;

    ID = INVALID_ID;
}

string window::get_title() {
    constexpr s64 tempLength = 30;

    auto *titleUtf16 = new wchar_t[tempLength];
    s32 length = GetWindowTextW(PlatformData.Win32.hWnd, titleUtf16, tempLength);
    if (length >= tempLength - 1) {
        titleUtf16 = new wchar_t[length + 1];
        GetWindowTextW(PlatformData.Win32.hWnd, titleUtf16, tempLength);
    }

    auto result = string(length * 2);  // @Bug length * 2 is not enough
    utf16_to_utf8(titleUtf16, const_cast<char *>(result.Data), &result.ByteLength);
    result.Length = utf8_length(result.Data, result.ByteLength);
    return result;
}

void window::set_title(const string &title) {
    auto *titleUtf16 = new (Context.TemporaryAlloc) wchar_t[title.Length + 1];  // @Bug title.Length is not enough
    utf8_to_utf16(title.Data, title.Length, titleUtf16);

    SetWindowTextW(PlatformData.Win32.hWnd, titleUtf16);
}

static void fit_to_monitor(window *win) {
    MONITORINFO mi = {sizeof(mi)};
    GetMonitorInfo(win->Monitor->PlatformData.Win32.hMonitor, &mi);
    SetWindowPos(win->PlatformData.Win32.hWnd, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
                 mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

void window::set_fullscreen(monitor *mon, s32 width, s32 height, s32 refreshRate) {
    DisplayMode.Width = width;
    DisplayMode.Height = height;
    DisplayMode.RefreshRate = refreshRate;

    if (Monitor == mon) {
        if (mon) {
            if (mon->Window == this) {
                acquire_monitor(this);
                fit_to_monitor(this);
            }
        } else {
            RECT rect = {0, 0, width, height};

            if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) {
                AdjustWindowRectExForDpi(&rect, get_window_style(this), false, get_window_ex_style(this),
                                         GetDpiForWindow(PlatformData.Win32.hWnd));
            } else {
                AdjustWindowRectEx(&rect, get_window_style(this), false, get_window_ex_style(this));
            }

            SetWindowPos(PlatformData.Win32.hWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left,
                         rect.bottom - rect.top, SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOZORDER);
        }
        return;
    }

    if (Monitor) release_monitor(this);
    Monitor = mon;

    if (Monitor) {
        u32 flags = SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS;
        if (!(Flags & BORDERLESS)) {
            DWORD style = GetWindowLongW(PlatformData.Win32.hWnd, GWL_STYLE);
            style &= ~WS_OVERLAPPEDWINDOW;
            style |= get_window_style(this);
            SetWindowLongW(PlatformData.Win32.hWnd, GWL_STYLE, style);

            flags |= SWP_FRAMECHANGED;
        }

        acquire_monitor(this);

        MONITORINFO mi = {sizeof(mi)};
        GetMonitorInfoW(Monitor->PlatformData.Win32.hMonitor, &mi);
        SetWindowPos(PlatformData.Win32.hWnd, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, flags);
    } else {
        UINT flags = SWP_NOACTIVATE | SWP_NOCOPYBITS;
        if (!(Flags & BORDERLESS)) {
            DWORD style = GetWindowLongW(PlatformData.Win32.hWnd, GWL_STYLE);
            style &= ~WS_POPUP;
            style |= get_window_style(this);
            SetWindowLongW(PlatformData.Win32.hWnd, GWL_STYLE, style);

            flags |= SWP_FRAMECHANGED;
        }

        HWND after;
        if (Flags & ALWAYS_ON_TOP) {
            after = HWND_TOPMOST;
        } else {
            after = HWND_NOTOPMOST;
        }

        RECT rect = {0, 0, width, height};
        if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) {
            AdjustWindowRectExForDpi(&rect, get_window_style(this), false, get_window_ex_style(this),
                                     GetDpiForWindow(PlatformData.Win32.hWnd));
        } else {
            AdjustWindowRectEx(&rect, get_window_style(this), false, get_window_ex_style(this));
        }

        SetWindowPos(PlatformData.Win32.hWnd, after, rect.left, rect.top, rect.right - rect.left,
                     rect.bottom - rect.top, flags);
    }
}

// Creates an RGBA icon or cursor
static HICON create_icon(const pixel_buffer &image, int xhot, int yhot, bool icon) {
    BITMAPV5HEADER bi;
    zero_memory(&bi, sizeof(bi));
    {
        bi.bV5Size = sizeof(bi);
        bi.bV5Width = image.Width;
        bi.bV5Height = -(s32)(image.Height);
        bi.bV5Planes = 1;
        bi.bV5BitCount = 32;
        bi.bV5Compression = BI_BITFIELDS;
        bi.bV5RedMask = 0x00ff0000;
        bi.bV5GreenMask = 0x0000ff00;
        bi.bV5BlueMask = 0x000000ff;
        bi.bV5AlphaMask = 0xff000000;
    }

    u8 *target = null;

    HDC dc = GetDC(null);
    HBITMAP color = CreateDIBSection(dc, (BITMAPINFO *) &bi, DIB_RGB_COLORS, (void **) &target, null, (DWORD) 0);
    ReleaseDC(null, dc);

    if (!color) {
        fmt::print("(windows_window.cpp): Failed to create RGBA bitmap\n");
        return null;
    }
    defer(DeleteObject(color));

    HBITMAP mask = CreateBitmap(image.Width, image.Height, 1, 1, null);
    if (!mask) {
        fmt::print("(windows_window.cpp): Failed to create mask bitmap\n");
        return null;
    }
    defer(DeleteObject(mask));

    u8 *source = image.Pixels;
    For(range(image.Width * image.Height)) {
        target[0] = source[2];
        target[1] = source[1];
        target[2] = source[0];
        target[3] = source[3];
        target += 4;
        source += 4;
    }

    ICONINFO ii;
    ZeroMemory(&ii, sizeof(ii));
    {
        ii.fIcon = icon;
        ii.xHotspot = xhot;
        ii.yHotspot = yhot;
        ii.hbmMask = mask;
        ii.hbmColor = color;
    }

    HICON handle = CreateIconIndirect(&ii);
    if (!handle) {
        if (icon) {
            fmt::print("(windows_window.cpp): Failed to create RGBA icon\n");
        } else {
            fmt::print("(windows_window.cpp): Failed to create RGBA cursor\n");
        }
    }
    return handle;
}

static s64 choose_icon(array<pixel_buffer> icons, s32 width, s32 height) {
    s32 leastDiff = numeric_info<s32>::max();

    s64 closest = -1;
    For(range(icons.Count)) {
        auto icon = icons[it];
        s32 diff = abs((s32)(icon.Width * icon.Height - width * height));
        if (diff < leastDiff) {
            closest = it;
            leastDiff = diff;
        }
    }
    return closest;
}

void window::set_icon(array<pixel_buffer> icons) {
    HICON bigIcon = null, smallIcon = null;

    if (icons.Count) {
        s64 closestBig = choose_icon(icons, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
        s64 closestSmall = choose_icon(icons, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));

        bigIcon = create_icon(icons[closestBig], 0, 0, true);
        smallIcon = create_icon(icons[closestSmall], 0, 0, true);
    } else {
        bigIcon = (HICON) GetClassLongPtrW(PlatformData.Win32.hWnd, GCLP_HICON);
        smallIcon = (HICON) GetClassLongPtrW(PlatformData.Win32.hWnd, GCLP_HICONSM);
    }

    SendMessage(PlatformData.Win32.hWnd, WM_SETICON, ICON_BIG, (LPARAM) bigIcon);
    SendMessage(PlatformData.Win32.hWnd, WM_SETICON, ICON_SMALL, (LPARAM) smallIcon);

    if (PlatformData.Win32.BigIcon) DestroyIcon(PlatformData.Win32.BigIcon);
    if (PlatformData.Win32.SmallIcon) DestroyIcon(PlatformData.Win32.SmallIcon);

    if (icons.Count) {
        PlatformData.Win32.BigIcon = bigIcon;
        PlatformData.Win32.SmallIcon = smallIcon;
    }
}

// Updates the cursor clip rect
static void update_clip_rect(window *win) {
    if (win) {
        RECT clipRect;
        GetClientRect(win->PlatformData.Win32.hWnd, &clipRect);
        ClientToScreen(win->PlatformData.Win32.hWnd, (POINT *) &clipRect.left);
        ClientToScreen(win->PlatformData.Win32.hWnd, (POINT *) &clipRect.right);
        ClipCursor(&clipRect);
    } else {
        ClipCursor(null);
    }
}

// Updates the cursor image according to its cursor mode
static void update_cursor_image(window *win) {
    if (win->CursorMode == window::CURSOR_NORMAL) {
        if (win->Cursor) {
            SetCursor(win->Cursor->PlatformData.Win32.hCursor);
        } else {
            SetCursor(LoadCursorW(null, IDC_ARROW));
        }
    } else {
        SetCursor(null);  // We get here when the cursor mode is window::CURSOR_HIDDEN
    }
}

// Enables WM_INPUT messages for the mouse for the specified window
static void enable_raw_mouse_motion(window *win) {
    RAWINPUTDEVICE rid = {0x01, 0x02, 0, win->PlatformData.Win32.hWnd};
    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        fmt::print("(windows_window.cpp): Failed to register raw input device. Raw mouse input may be unsupported.\n");
    }
}

// Disables WM_INPUT messages for the mouse
static void disable_raw_mouse_motion(window *win) {
    RAWINPUTDEVICE rid = {0x01, 0x02, RIDEV_REMOVE, null};
    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        fmt::print("(windows_window.cpp): Failed to remove raw input device\n");
    }
}

// Apply disabled cursor mode to a focused window
static void disable_cursor(window *win) {
    DisabledCursorWindow = win;
    RestoreCursorPos = win->get_cursor_pos();
    update_cursor_image(win);
    win->set_cursor_pos(win->get_size() / 2);
    update_clip_rect(win);

    if (win->RawMouseMotion) enable_raw_mouse_motion(win);
}

// Exit disabled cursor mode for the specified window
static void enable_cursor(window *win) {
    if (win->RawMouseMotion) disable_raw_mouse_motion(win);

    DisabledCursorWindow = null;
    update_clip_rect(null);
    win->set_cursor_pos(RestoreCursorPos);
    update_cursor_image(win);
}

void window::set_cursor(cursor *curs) {
    Cursor = curs;
    if (is_hovered()) update_cursor_image(this);
}

vec2<s32> window::get_cursor_pos() {
    POINT pos;
    if (GetCursorPos(&pos)) {
        ScreenToClient(PlatformData.Win32.hWnd, &pos);
        return {pos.x, pos.y};
    }
    assert(false);
    return {no_init};
}

void window::set_cursor_pos(vec2<s32> pos) {
    if (pos == get_cursor_pos()) return;

    PlatformData.Win32.LastCursorPos = pos;

    POINT point = {pos.x, pos.y};
    ClientToScreen(PlatformData.Win32.hWnd, &point);
    SetCursorPos(point.x, point.y);
}

vec2<s32> window::get_pos() {
    POINT pos = {0, 0};
    ClientToScreen(PlatformData.Win32.hWnd, &pos);
    return {pos.x, pos.y};
}

void window::set_pos(vec2<s32> pos) {
    if (pos == get_pos()) return;

    RECT rect = {pos.x, pos.y, pos.x, pos.y};

    if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) {
        AdjustWindowRectExForDpi(&rect, get_window_style(this), false, get_window_ex_style(this),
                                 GetDpiForWindow(PlatformData.Win32.hWnd));
    } else {
        AdjustWindowRectEx(&rect, get_window_style(this), false, get_window_ex_style(this));
    }
    SetWindowPos(PlatformData.Win32.hWnd, null, rect.left, rect.top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

vec2<s32> window::get_size() {
    RECT area;
    GetClientRect(PlatformData.Win32.hWnd, &area);
    return {area.right, area.bottom};
}

void window::set_size(vec2<s32> size) {
    DisplayMode.Width = size.x;
    DisplayMode.Height = size.y;

    if (size == get_size()) return;

    if (Monitor) {
        if (Monitor->Window == this) {
            acquire_monitor(this);
            fit_to_monitor(this);
        }
    } else {
        RECT rect = {0, 0, size.x, size.y};

        if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) {
            AdjustWindowRectExForDpi(&rect, get_window_style(this), false, get_window_ex_style(this),
                                     GetDpiForWindow(PlatformData.Win32.hWnd));
        } else {
            AdjustWindowRectEx(&rect, get_window_style(this), false, get_window_ex_style(this));
        }

        SetWindowPos(PlatformData.Win32.hWnd, HWND_TOP, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
    }
}

vec2<s32> window::get_framebuffer_size() { return get_size(); }

rect window::get_adjusted_bounds() {
    vec2<s32> size = get_size();

    RECT rect;
    SetRect(&rect, 0, 0, size.x, size.y);

    if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) {
        AdjustWindowRectExForDpi(&rect, get_window_style(this), false, get_window_ex_style(this),
                                 GetDpiForWindow(PlatformData.Win32.hWnd));
    } else {
        AdjustWindowRectEx(&rect, get_window_style(this), false, get_window_ex_style(this));
    }

    return {-rect.left, -rect.top, rect.right, rect.bottom};
}

void window::set_size_limits(vec2<s32> minDimension, vec2<s32> maxDimension) {
    if (minDimension.x != DONT_CARE && minDimension.y != DONT_CARE) {
        if (minDimension.x < 0 || minDimension.y < 0) {
            fmt::print("(windows_window.cpp): Invalid window minimum size ({}x{})\n", minDimension.x, minDimension.y);
            return;
        }
    }

    if (maxDimension.x != DONT_CARE && maxDimension.y != DONT_CARE) {
        if (maxDimension.x < 0 || maxDimension.y < 0 || maxDimension.x < minDimension.x ||
            maxDimension.y < minDimension.y) {
            fmt::print("(windows_window.cpp): Invalid window maximum size ({}x{})\n", maxDimension.x, maxDimension.y);
            return;
        }
    }

    MinW = minDimension.x;
    MinH = minDimension.y;
    MaxW = maxDimension.x;
    MaxH = maxDimension.y;

    if (Monitor || !(Flags & RESIZABLE)) return;

    RECT area;
    GetWindowRect(PlatformData.Win32.hWnd, &area);
    MoveWindow(PlatformData.Win32.hWnd, area.left, area.top, area.right - area.left, area.bottom - area.top, true);
}

static void apply_aspect_ratio(window *win, s32 edge, RECT *area) {
    f32 ratio = (f32) win->AspectRatioNumerator / (f32) win->AspectRatioDenominator;

    u32 dpi = USER_DEFAULT_SCREEN_DPI;
    if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) dpi = GetDpiForWindow(win->PlatformData.Win32.hWnd);

    vec2<s32> off = get_full_window_size(get_window_style(win), get_window_ex_style(win), 0, 0, dpi);

    if (edge == WMSZ_LEFT || edge == WMSZ_BOTTOMLEFT || edge == WMSZ_RIGHT || edge == WMSZ_BOTTOMRIGHT) {
        area->bottom = area->top + off.y + (s32)((area->right - area->left - off.x) / ratio);
    } else if (edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT) {
        area->top = area->bottom - off.y - (s32)((area->right - area->left - off.x) / ratio);
    } else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM) {
        area->right = area->left + off.x + (s32)((area->bottom - area->top - off.y) * ratio);
    }
}

void window::set_forced_aspect_ratio(s32 numerator, s32 denominator) {
    if (numerator != DONT_CARE && denominator != DONT_CARE) {
        if (numerator <= 0 || denominator <= 0) {
            fmt::print("(windows_window.cpp): Invalid window aspect ratio ({}:{})\n", numerator, denominator);
            return;
        }
    }

    AspectRatioNumerator = numerator;
    AspectRatioDenominator = denominator;

    if (numerator == DONT_CARE || denominator == DONT_CARE) return;

    RECT area;
    GetWindowRect(PlatformData.Win32.hWnd, &area);
    apply_aspect_ratio(this, WMSZ_BOTTOMRIGHT, &area);
    MoveWindow(PlatformData.Win32.hWnd, area.left, area.top, area.right - area.left, area.bottom - area.top, true);
}

void window::set_raw_mouse(bool enabled) {
    if (RawMouseMotion == enabled) return;

    if (DisabledCursorWindow != this) {
        RawMouseMotion = enabled;
        if (enabled) {
            enable_raw_mouse_motion(this);
        } else {
            disable_raw_mouse_motion(this);
        }
    }
}

void window::set_cursor_mode(cursor_mode mode) {
    if (CursorMode == mode) return;

    CursorMode = mode;
    VirtualCursorPos = get_cursor_pos();

    if (mode == CURSOR_DISABLED) {
        if (Flags & FOCUSED) disable_cursor(this);
    } else if (DisabledCursorWindow == this) {
        enable_cursor(this);
    } else if (is_hovered()) {
        update_cursor_image(this);
    }
}

f32 window::get_opacity() {
    BYTE alpha;
    DWORD flags;

    if ((GetWindowLongW(PlatformData.Win32.hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) &&
        GetLayeredWindowAttributes(PlatformData.Win32.hWnd, null, &alpha, &flags)) {
        if (flags & LWA_ALPHA) return alpha / 255.f;
    }
    return 1.f;
}

void window::set_opacity(f32 opacity) {
    assert(opacity >= 0 && opacity <= 1.0f);

    if (opacity < 1.0f) {
        BYTE alpha = (BYTE)(255 * opacity);
        DWORD style = GetWindowLongW(PlatformData.Win32.hWnd, GWL_EXSTYLE);
        style |= WS_EX_LAYERED;
        SetWindowLongW(PlatformData.Win32.hWnd, GWL_EXSTYLE, style);
        SetLayeredWindowAttributes(PlatformData.Win32.hWnd, 0, alpha, LWA_ALPHA);
    } else {
        DWORD style = GetWindowLongW(PlatformData.Win32.hWnd, GWL_EXSTYLE);
        style &= ~WS_EX_LAYERED;
        SetWindowLongW(PlatformData.Win32.hWnd, GWL_EXSTYLE, style);
    }
}

static void update_window_style(window *win) {
    DWORD style = GetWindowLongW(win->PlatformData.Win32.hWnd, GWL_STYLE);
    style &= ~(WS_OVERLAPPEDWINDOW | WS_POPUP);
    style |= get_window_style(win);

    RECT rect;
    GetClientRect(win->PlatformData.Win32.hWnd, &rect);

    if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) {
        AdjustWindowRectExForDpi(&rect, style, false, get_window_ex_style(win),
                                 GetDpiForWindow(win->PlatformData.Win32.hWnd));
    } else {
        AdjustWindowRectEx(&rect, style, false, get_window_ex_style(win));
    }

    ClientToScreen(win->PlatformData.Win32.hWnd, (POINT *) &rect.left);
    ClientToScreen(win->PlatformData.Win32.hWnd, (POINT *) &rect.right);
    SetWindowLongW(win->PlatformData.Win32.hWnd, GWL_STYLE, style);
    SetWindowPos(win->PlatformData.Win32.hWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left,
                 rect.bottom - rect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOZORDER);
}

void window::set_borderless(bool enabled) {
    set_bit(&Flags, (u32) window::BORDERLESS, enabled);
    if (!Monitor) update_window_style(this);
}

void window::set_resizable(bool enabled) {
    set_bit(&Flags, (u32) window::RESIZABLE, enabled);
    if (!Monitor) update_window_style(this);
}

void window::set_always_on_top(bool enabled) {
    set_bit(&Flags, (u32) window::ALWAYS_ON_TOP, enabled);
    if (!Monitor) {
        HWND after = enabled ? HWND_TOPMOST : HWND_NOTOPMOST;
        SetWindowPos(PlatformData.Win32.hWnd, after, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }
}

bool window::is_hovered() {
    POINT pos;
    if (!GetCursorPos(&pos)) return false;
    if (WindowFromPoint(pos) != PlatformData.Win32.hWnd) return false;

    RECT area;
    GetClientRect(PlatformData.Win32.hWnd, &area);
    ClientToScreen(PlatformData.Win32.hWnd, (POINT *) &area.left);
    ClientToScreen(PlatformData.Win32.hWnd, (POINT *) &area.right);

    return PtInRect(&area, pos);
}

bool window::is_visible() {
    if ((Flags & MINIMIZED) || (Flags & HIDDEN)) return false;
    if (get_size().x == 0 || get_size().y == 0) return false;
    return true;
}

void window::show() {
    ShowWindow(PlatformData.Win32.hWnd, SW_SHOWNA);
    if (Flags & FOCUS_ON_SHOW) focus();
}
void window::hide() { ShowWindow(PlatformData.Win32.hWnd, SW_HIDE); }
void window::minimize() { ShowWindow(PlatformData.Win32.hWnd, SW_MINIMIZE); }
void window::restore() { ShowWindow(PlatformData.Win32.hWnd, SW_RESTORE); }
void window::maximize() { ShowWindow(PlatformData.Win32.hWnd, SW_MAXIMIZE); }

void window::focus() {
    BringWindowToTop(PlatformData.Win32.hWnd);
    SetForegroundWindow(PlatformData.Win32.hWnd);
    SetFocus(PlatformData.Win32.hWnd);
}

void window::request_attention() { FlashWindow(PlatformData.Win32.hWnd, true); }

static LRESULT __stdcall wnd_proc(HWND hWnd, u32 message, WPARAM wParam, LPARAM lParam) {
    window *win = (window *) GetPropW(hWnd, L"LSTD");
    if (!win) {
        // This is the message handling for the hidden helper window
        // and for a regular window during its initial creation
        switch (message) {
            case WM_NCCREATE:
                if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) EnableNonClientDpiScaling(hWnd);
                break;
            case WM_DISPLAYCHANGE:
                win32_poll_monitors();
                break;
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    event e;
    e.Window = win;
    e.Type = event::Window_Platform_Message_Sent;
    e.Message = message;
    e.Param1 = wParam;
    e.Param2 = lParam;
    win->Event.emit(null, e);

    switch (message) {
        case WM_MOUSEACTIVATE:
            if (HIWORD(lParam) == WM_LBUTTONDOWN) {
                if (LOWORD(lParam) != HTCLIENT) win->PlatformData.Win32.FrameAction = true;
            }
            break;
        case WM_CAPTURECHANGED:
            // @Hack: Disable the cursor once the caption button action has been completed or cancelled
            if (lParam == 0 && win->PlatformData.Win32.FrameAction) {
                if (win->CursorMode == window::CURSOR_DISABLED) disable_cursor(win);
                win->PlatformData.Win32.FrameAction = false;
            }
            break;
        case WM_SETFOCUS:
            win->Flags |= window::FOCUSED;

            {
                event e;
                e.Window = win;
                e.Type = event::Window_Focused;
                e.Focused = true;
                win->Event.emit(null, e);
            }

            // @Hack: Do not disable cursor while the user is interacting with a caption button
            if (win->PlatformData.Win32.FrameAction) break;
            if (win->CursorMode == window::CURSOR_DISABLED) disable_cursor(win);
            return 0;
        case WM_KILLFOCUS:
            win->Flags &= ~window::FOCUSED;
            if (win->CursorMode == window::CURSOR_DISABLED) enable_cursor(win);
            if (win->Monitor && win->Flags & window::AUTO_MINIMIZE) win->minimize();

            {
                event e;
                e.Window = win;
                e.Type = event::Window_Focused;
                e.Focused = false;
                win->Event.emit(null, e);
            }

            For(range(Key_Last + 1)) {
                if (win->Keys[it]) do_key_input_event(win, (u32) it, false);
            }

            For(range(Mouse_Button_Last + 1)) {
                if (win->MouseButtons[it]) do_mouse_input_event(win, (u32) it, false);
            }
            return 0;
        case WM_SYSCOMMAND:
            switch (wParam & 0xfff0) {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER: {
                    if (win->Monitor) {
                        // We are running in full screen mode, so disallow screen saver and screen blanking
                        return 0;
                    } else
                        break;
                }
                case SC_KEYMENU:
                    return 0;
            }
            break;
        case WM_CLOSE:
            win->IsDestroying = true;
            {
                event e;
                e.Window = win;
                e.Type = event::Window_Closed;
                win->Event.emit(null, e);
            }
            win->release();
            return 0;
        case WM_CHAR:
        case WM_UNICHAR: {
            if (message == WM_UNICHAR && wParam == UNICODE_NOCHAR) return true;

            auto cp = (char32_t) wParam;
            if (cp < 32 || (cp > 126 && cp < 160)) return 0;

            if ((cp >= 0xD800) && (cp <= 0xDBFF)) {
                // First part of a surrogate pair: store it and wait for the second one
                win->PlatformData.Win32.Surrogate = (u16) cp;
            } else {
                if ((cp >= 0xDC00) && (cp <= 0xDFFF)) {
                    cp = ((win->PlatformData.Win32.Surrogate - 0xD800) << 10) + (cp - 0xDC00) + 0x0010000;
                    win->PlatformData.Win32.Surrogate = 0;
                }
                event e;
                e.Window = win;
                e.Type = event::Code_Point_Typed;
                e.CP = cp;
                win->Event.emit(null, e);
            }
            return 0;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            u32 key = ((lParam >> 16) & 0x7f) | ((lParam & (1 << 24)) != 0 ? 0x80 : 0);
            if (key >= 256) break;

            u32 keyHid = internal::g_KeycodeNativeToHid[key];
            bool pressed = ((lParam >> 31) & 1) ? false : true;

            if (!pressed && wParam == VK_SHIFT) {
                // @Hack: Release both Shift keys on Shift up event, as when both
                //        are pressed the first release does not emit any event
                // :ShiftHack The other half of this is in window::update
                do_key_input_event(win, Key_LeftShift, false);
                do_key_input_event(win, Key_RightShift, false);
            } else if (wParam == VK_SNAPSHOT) {
                // @Hack: Key down is not reported for the Print Screen key
                do_key_input_event(win, Key_PrintScreen, true);
                do_key_input_event(win, Key_PrintScreen, false);
            } else {
                do_key_input_event(win, keyHid, pressed);
            }

            if (win->Flags & window::CLOSE_ON_ALT_F4 && message == WM_SYSKEYDOWN && keyHid == Key_F4) {
                SendMessageW(win->PlatformData.Win32.hWnd, WM_CLOSE, 0, 0);
            }
            break;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP: {
            u32 button;
            if (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_LBUTTONDBLCLK) {
                button = Mouse_Button_Left;
            } else if (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP || message == WM_RBUTTONDBLCLK) {
                button = Mouse_Button_Right;
            } else if (message == WM_MBUTTONDOWN || message == WM_MBUTTONUP || message == WM_MBUTTONDBLCLK) {
                button = Mouse_Button_Middle;
            } else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
                button = Mouse_Button_X1;
            } else {
                button = Mouse_Button_X2;
            }

            bool pressed = message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK || message == WM_RBUTTONDOWN ||
                           message == WM_RBUTTONDBLCLK || message == WM_MBUTTONDOWN || message == WM_MBUTTONDBLCLK ||
                           message == WM_XBUTTONDOWN || message == WM_XBUTTONDBLCLK;

            bool anyPressed = false;
            For(range(Mouse_Button_Last + 1)) {
                if (win->MouseButtons[it]) {
                    anyPressed = true;
                    break;
                }
            }
            if (!anyPressed) SetCapture(hWnd);

            do_mouse_input_event(win, button, pressed,
                                 message == WM_LBUTTONDBLCLK || message == WM_RBUTTONDBLCLK ||
                                     message == WM_MBUTTONDBLCLK || message == WM_XBUTTONDBLCLK);

            anyPressed = false;
            For(range(Mouse_Button_Last + 1)) {
                if (win->MouseButtons[it]) {
                    anyPressed = true;
                    break;
                }
            }
            if (!anyPressed) ReleaseCapture();

            if (message == WM_XBUTTONDOWN || message == WM_XBUTTONUP) return true;

            return 0;
        }
        case WM_MOUSEMOVE: {
            vec2<s32> pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

            if (!win->PlatformData.Win32.CursorTracked) {
                TRACKMOUSEEVENT tme;
                zero_memory(&tme, sizeof(tme));
                {
                    tme.cbSize = sizeof(tme);
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = win->PlatformData.Win32.hWnd;
                }
                TrackMouseEvent(&tme);

                win->PlatformData.Win32.CursorTracked = true;

                event e;
                e.Window = win;
                e.Type = event::Mouse_Entered_Window;
                win->Event.emit(null, e);
            }

            if (win->CursorMode == window::CURSOR_DISABLED) {
                if (DisabledCursorWindow != win) break;
                if (win->RawMouseMotion) break;

                vec2<s32> delta = pos - win->PlatformData.Win32.LastCursorPos;
                do_mouse_move(win, win->VirtualCursorPos + delta);
            } else {
                do_mouse_move(win, pos);
            }
            win->PlatformData.Win32.LastCursorPos = pos;

            return 0;
        }
        case WM_INPUT: {
            if (DisabledCursorWindow != win) break;
            if (!win->RawMouseMotion) break;

            HRAWINPUT ri = (HRAWINPUT) lParam;

            u32 size = 0;
            GetRawInputData(ri, RID_INPUT, null, &size, sizeof(RAWINPUTHEADER));

            auto *rawInput = (RAWINPUT *) new (Context.TemporaryAlloc) char[size];
            if (GetRawInputData(ri, RID_INPUT, rawInput, &size, sizeof(RAWINPUTHEADER)) == (u32) -1) {
                fmt::print("(windows_window.cpp): Failed to retrieve raw input data\n");
                break;
            }

            s32 dx, dy;
            if (rawInput->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
                dx = rawInput->data.mouse.lLastX - win->PlatformData.Win32.LastCursorPos.x;
                dy = rawInput->data.mouse.lLastY - win->PlatformData.Win32.LastCursorPos.y;
            } else {
                dx = rawInput->data.mouse.lLastX;
                dy = rawInput->data.mouse.lLastY;
            }
            do_mouse_move(win, win->VirtualCursorPos + vec2<s32>(dx, dy));

            win->PlatformData.Win32.LastCursorPos.x += dx;
            win->PlatformData.Win32.LastCursorPos.y += dy;
            break;
        }
        case WM_MOUSELEAVE:
            win->PlatformData.Win32.CursorTracked = false;
            {
                event e;
                e.Window = win;
                e.Type = event::Mouse_Left_Window;
                win->Event.emit(null, e);
            }
            return 0;
        case WM_MOUSEWHEEL: {
            event e;
            e.Window = win;
            e.Type = event::Mouse_Wheel_Scrolled;
            e.ScrollY = (u32)(GET_WHEEL_DELTA_WPARAM(wParam) / (f32) WHEEL_DELTA);
            win->Event.emit(null, e);
            return 0;
        }
        case WM_MOUSEHWHEEL:
            // NOTE: The X-axis is inverted for consistency with macOS and X11
            {
                event e;
                e.Window = win;
                e.Type = event::Mouse_Wheel_Scrolled;
                e.ScrollX = (u32)(-(GET_WHEEL_DELTA_WPARAM(wParam) / (f32) WHEEL_DELTA));
                win->Event.emit(null, e);
            }
            return 0;
        case WM_ENTERSIZEMOVE:
        case WM_ENTERMENULOOP:
            if (win->PlatformData.Win32.FrameAction) break;
            if (win->CursorMode == window::CURSOR_DISABLED) enable_cursor(win);
            break;
        case WM_EXITSIZEMOVE:
        case WM_EXITMENULOOP:
            if (win->PlatformData.Win32.FrameAction) break;
            if (win->CursorMode == window::CURSOR_DISABLED) disable_cursor(win);
            break;
        case WM_SIZE: {
            bool minimized = wParam == SIZE_MINIMIZED;
            bool maximized = wParam == SIZE_MAXIMIZED || (win->Flags & window::MAXIMIZED && wParam != SIZE_RESTORED);

            if (DisabledCursorWindow == win) update_clip_rect(win);

            bool windowMinimized = win->Flags & window::MINIMIZED;
            bool windowMaximized = win->Flags & window::MAXIMIZED;

            if (minimized && !windowMinimized) {
                win->Flags |= window::MINIMIZED;
                win->minimize();

                event e;
                e.Window = win;
                e.Type = event::Window_Minimized;
                e.Minimized = true;
                win->Event.emit(null, e);
            }
            if (windowMinimized && !minimized) {
                win->Flags &= ~window::MINIMIZED;

                event e;
                e.Window = win;
                e.Type = event::Window_Minimized;
                e.Minimized = false;
                win->Event.emit(null, e);
            }

            if (maximized && !windowMaximized) {
                win->Flags |= window::MAXIMIZED;
                win->maximize();

                event e;
                e.Window = win;
                e.Type = event::Window_Maximized;
                e.Maximized = true;
                win->Event.emit(null, e);
            }
            if (windowMaximized && !maximized) {
                win->Flags &= ~window::MAXIMIZED;

                event e;
                e.Window = win;
                e.Type = event::Window_Maximized;
                e.Maximized = false;
                win->Event.emit(null, e);
            }

            event e;
            e.Window = win;
            e.Type = event::Window_Framebuffer_Resized;
            e.Width = LOWORD(lParam);
            e.Height = HIWORD(lParam);
            win->Event.emit(null, e);
            e.Type = event::Window_Resized;
            win->Event.emit(null, e);

            if (win->Monitor && windowMinimized != minimized) {
                if (minimized) {
                    release_monitor(win);
                } else {
                    acquire_monitor(win);
                    fit_to_monitor(win);
                }
            }
            return 0;
        }
        case WM_SHOWWINDOW:
            set_bit(&win->Flags, (u32) window::SHOWN, wParam);
            set_bit(&win->Flags, (u32) window::HIDDEN, !wParam);
            break;
        case WM_MOVE:
            if (DisabledCursorWindow == win) update_clip_rect(win);
            {
                event e;
                e.Window = win;
                e.Type = event::Window_Moved;
                e.X = GET_X_LPARAM(lParam);
                e.Y = GET_Y_LPARAM(lParam);
                win->Event.emit(null, e);
            }
            return 0;

        case WM_SIZING:
            if (win->AspectRatioNumerator == window::DONT_CARE || win->AspectRatioDenominator == window::DONT_CARE) {
                break;
            }
            apply_aspect_ratio(win, (s32) wParam, (RECT *) lParam);
            return true;
        case WM_GETMINMAXINFO: {
            if (win->Monitor) break;

            UINT dpi = USER_DEFAULT_SCREEN_DPI;
            if (IS_WINDOWS_10_ANNIVERSARY_UPDATE_OR_GREATER()) dpi = GetDpiForWindow(win->PlatformData.Win32.hWnd);

            auto *mmi = (MINMAXINFO *) lParam;

            vec2<s32> off = get_full_window_size(get_window_style(win), get_window_ex_style(win), 0, 0, dpi);
            if (win->MinH != window::DONT_CARE) mmi->ptMinTrackSize.x = win->MinW + off.x;
            if (win->MinW != window::DONT_CARE) mmi->ptMinTrackSize.y = win->MinH + off.y;

            if (win->MaxW != window::DONT_CARE) mmi->ptMaxTrackSize.x = win->MaxW + off.x;
            if (win->MinW != window::DONT_CARE) mmi->ptMaxTrackSize.y = win->MaxH + off.y;

            if (win->Flags & window::BORDERLESS) {
                HMONITOR mh = MonitorFromWindow(win->PlatformData.Win32.hWnd, MONITOR_DEFAULTTONEAREST);

                MONITORINFO mi;
                zero_memory(&mi, sizeof(mi));
                mi.cbSize = sizeof(mi);
                GetMonitorInfo(mh, &mi);

                mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
                mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
                mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
                mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
            }
            return 0;
        }
        case WM_PAINT: {
            event e;
            e.Window = win;
            e.Type = event::Window_Refreshed;
            win->Event.emit(null, e);
        } break;
        case WM_ERASEBKGND:
            return true;
        case WM_NCACTIVATE:
        case WM_NCPAINT:
            // Prevent title bar from being drawn after restoring a minimized undecorated window
            if (win->Flags & window::BORDERLESS) return true;
            break;
        case WM_NCHITTEST:
            if (win->Flags & window::MOUSE_PASS_THROUGH) return HTTRANSPARENT;
            break;
        case WM_DWMCOMPOSITIONCHANGED:
            if (win->Flags & window::ALPHA) update_framebuffer_transparency(win);
            return 0;
        case WM_GETDPISCALEDSIZE: {
            // Adjust the window size to keep the content area size constant
            if (IS_WINDOWS_10_CREATORS_UPDATE_OR_GREATER()) {
                RECT source{};
                RECT target{};
                SIZE *size = (SIZE *) lParam;

                AdjustWindowRectExForDpi(&source, get_window_style(win), false, get_window_ex_style(win),
                                         GetDpiForWindow(win->PlatformData.Win32.hWnd));
                AdjustWindowRectExForDpi(&target, get_window_style(win), false, get_window_ex_style(win),
                                         LOWORD(wParam));

                size->cx += (target.right - target.left) - (source.right - source.left);
                size->cy += (target.bottom - target.top) - (source.bottom - source.top);
                return true;
            }
            break;
        }
        case WM_DPICHANGED: {
            f32 xscale = HIWORD(wParam) / (f32) USER_DEFAULT_SCREEN_DPI;
            f32 yscale = LOWORD(wParam) / (f32) USER_DEFAULT_SCREEN_DPI;

            // Only apply the suggested size if the OS is new enough to have
            // sent a WM_GETDPISCALEDSIZE before this
            if (IS_WINDOWS_10_CREATORS_UPDATE_OR_GREATER()) {
                RECT *suggested = (RECT *) lParam;
                SetWindowPos(win->PlatformData.Win32.hWnd, HWND_TOP, suggested->left, suggested->top,
                             suggested->right - suggested->left, suggested->bottom - suggested->top,
                             SWP_NOACTIVATE | SWP_NOZORDER);
            }
            event e;
            e.Window = win;
            e.Type = event::Window_Content_Scale_Changed;
            e.Scale = {xscale, yscale};
            win->Event.emit(null, e);
            break;
        }
        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT) {
                update_cursor_image(win);
                return true;
            }
            break;
        case WM_DROPFILES: {
            HDROP drop = (HDROP) wParam;

            // Move the mouse to the position of the drop
            POINT pt;
            DragQueryPoint(drop, &pt);
            do_mouse_move(win, {pt.x, pt.y});

            array<file::path> paths;
            s32 count = DragQueryFileW(drop, 0xffffffff, null, 0);
            For(range(count)) {
                u32 length = DragQueryFileW(drop, (u32) it, null, 0);
                wchar_t *buffer = new (Context.TemporaryAlloc) wchar_t[length + 1];
                DragQueryFileW(drop, (u32) it, buffer, length + 1);

                string utf8Buffer;
                utf8Buffer.reserve(length * 2);  // @Bug length * 2 is not enough
                utf16_to_utf8(buffer, const_cast<char *>(utf8Buffer.Data), &utf8Buffer.ByteLength);
                utf8Buffer.Length = utf8_length(utf8Buffer.Data, utf8Buffer.ByteLength);

                auto path = file::path(utf8Buffer);
                move(paths.append(), &path);
            }

            event e;
            e.Window = win;
            e.Type = event::Window_Files_Dropped;
            e.Paths = paths;
            win->Event.emit(null, e);

            DragFinish(drop);
            return 0;
        }
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

void win32_register_window_class() {
    GUID guid;
    CHECKHR(CoCreateGuid(&guid));
    CHECKHR(StringFromCLSID(guid, &g_Win32WindowClassName));

    WNDCLASSEXW wc;
    zero_memory(&wc, sizeof(wc));
    {
        wc.cbSize = sizeof(wc);
        wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = (WNDPROC) wnd_proc;
        wc.hInstance = GetModuleHandleW(null);
        wc.hCursor = LoadCursorW(null, IDC_ARROW);
        wc.lpszClassName = g_Win32WindowClassName;

        // Load user-provided icon if available
        wc.hIcon =
            (HICON) LoadImageW(GetModuleHandleW(null), L"WINDOW ICON", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        if (!wc.hIcon) {
            // No user-provided icon found, load default icon
            wc.hIcon = (HICON) LoadImageW(null, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        }
    }

    if (!RegisterClassExW(&wc)) {
        fmt::print("(windows_window.cpp): Failed to register window class\n");
        assert(false);
    }
}

cursor::cursor(const pixel_buffer &image, vec2<s32> hot) {
    PlatformData.Win32.hCursor = (HCURSOR) create_icon(image, hot.x, hot.y, false);
    if (!PlatformData.Win32.hCursor) return;
    PlatformData.Win32.ShouldDestroy = true;

    Next = CursorsList;
    CursorsList = this;
}

cursor::cursor(os_cursor osCursor) {
    const wchar_t *id = null;
    if (osCursor == OS_APPSTARTING) id = IDC_APPSTARTING;
    if (osCursor == OS_ARROW) id = IDC_ARROW;
    if (osCursor == OS_IBEAM) id = IDC_IBEAM;
    if (osCursor == OS_CROSSHAIR) id = IDC_CROSS;
    if (osCursor == OS_HAND) id = IDC_HAND;
    if (osCursor == OS_HELP) id = IDC_HELP;
    if (osCursor == OS_NO) id = IDC_NO;
    if (osCursor == OS_RESIZE_ALL) id = IDC_SIZEALL;
    if (osCursor == OS_RESIZE_NESW) id = IDC_SIZENESW;
    if (osCursor == OS_RESIZE_NS) id = IDC_SIZENS;
    if (osCursor == OS_RESIZE_NWSE) id = IDC_SIZENWSE;
    if (osCursor == OS_RESIZE_WE) id = IDC_SIZEWE;
    if (osCursor == OS_UP_ARROW) id = IDC_UPARROW;
    if (osCursor == OS_WAIT) id = IDC_WAIT;
    assert(id);

    PlatformData.Win32.hCursor = (HICON) LoadImageW(null, id, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    if (!PlatformData.Win32.hCursor) {
        fmt::print("(windows_window.cpp): Failed to create os cursor\n");
    }
    PlatformData.Win32.ShouldDestroy = false;

    Next = CursorsList;
    CursorsList = this;
}

void cursor::release() {
    if (PlatformData.Win32.ShouldDestroy) DestroyCursor(PlatformData.Win32.hCursor);

    cursor **prev = &CursorsList;
    while (*prev != this) prev = &((*prev)->Next);
    *prev = this->Next;
}

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
