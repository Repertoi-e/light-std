#include "le/window.h"

#if OS == WINDOWS

#include <lstd/io/fmt.h>
#include <lstd/os.h>

#undef MAC
#undef _MAC
#include <Windows.h>
#include <Windowsx.h>

namespace le {

// Returns the last Win32 error, in string format. Returns an empty string if there is no error.
string get_last_error_as_string() {
    DWORD errorMessageID = GetLastError();
    if (errorMessageID == 0) return "";

    LPSTR messageBuffer = null;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, null,
        errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, null);

    string message(messageBuffer, size);

    // Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

struct windows_data {
    HWND hWnd;
    bool MouseInClient = true;

    // Used when handling text input
    u16 Surrogate;
};
#define PDATA ((windows_data *) PlatformData)

#define KEY_EVENT_GET_MODS                                                                                            \
    (((GetKeyState(VK_CONTROL) & 0x8000) ? Modifier_Control : 0) |                                                    \
     ((GetKeyState(VK_SHIFT) & 0x8000) ? Modifier_Shift : 0) | ((GetKeyState(VK_MENU) & 0x8000) ? Modifier_Alt : 0) | \
     (((GetKeyState(VK_RWIN) & 0x8000) || (GetKeyState(VK_LWIN) & 0x8000)) ? Modifier_Super : 0))

#define MOUSE_EVENT_GET_MODS(x)                                                          \
    (((x & MK_CONTROL) ? Modifier_Control : 0) | ((x & MK_SHIFT) ? Modifier_Shift : 0) | \
     ((GetKeyState(VK_MENU) & 0x8000) ? Modifier_Alt : 0) |                              \
     (((GetKeyState(VK_RWIN) & 0x8000) || (GetKeyState(VK_LWIN) & 0x8000)) ? Modifier_Super : 0))
#define MOUSE_EVENT_GET_BUTTONS_DOWN(x)                                                          \
    (((x & MK_LBUTTON) ? Mouse_Button_Left : 0) | ((x & MK_MBUTTON) ? Mouse_Button_Middle : 0) | \
     ((x & MK_RBUTTON) ? Mouse_Button_Right : 0) | ((x & MK_XBUTTON1) ? Mouse_Button_X1 : 0) |   \
     ((x & MK_XBUTTON2) ? Mouse_Button_X2 : 0))

ptr_t __stdcall WndProc(HWND hWnd, u32 message, uptr_t wParam, ptr_t lParam) {
    auto *wind = (window *) GetWindowLongPtrW(hWnd, 0);
    auto *data = (windows_data *) wind->PlatformData;

    switch (message) {
        case WM_NCCREATE:
            wind = (window *) ((CREATESTRUCT *) lParam)->lpCreateParams;
            SetWindowLongPtrW(hWnd, 0, (ptr_t) wind);
            SetWindowPos(hWnd, null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
            return DefWindowProc(hWnd, message, wParam, lParam);
        case WM_CLOSE:
            wind->WindowClosedEvent.emit(null, {wind});
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            if (wParam == SIZE_RESTORED) {
                wind->WindowResizedEvent.emit(null, {wind, (u32) LOWORD(lParam), (u32) HIWORD(lParam)});
            }
        case WM_SETFOCUS:
            wind->WindowGainedFocusEvent.emit(null, {wind});
            break;
        case WM_KILLFOCUS:
            wind->WindowLostFocusEvent.emit(null, {wind});
            break;
        case WM_MOVE:
            wind->WindowMovedEvent.emit(null, {wind, (s32)(s16) LOWORD(lParam), (s32)(s16) HIWORD(lParam)});
            break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            wind->KeyPressedEvent.emit(
                null, {wind, g_KeycodeNativeToHid[wParam], KEY_EVENT_GET_MODS, (bool) (lParam & 0x40000000)});
            break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
            wind->KeyReleasedEvent.emit(null, {wind, g_KeycodeNativeToHid[wParam], KEY_EVENT_GET_MODS});
            break;
        case WM_CHAR: {
            auto cp = (char32_t) wParam;
            if ((cp >= 0xD800) && (cp <= 0xDBFF)) {
                // First part of a surrogate pair: store it and wait for the second one
                data->Surrogate = (u16) cp;
            } else {
                if ((cp >= 0xDC00) && (cp <= 0xDFFF)) {
                    cp = ((data->Surrogate - 0xD800) << 10) + (cp - 0xDC00) + 0x0010000;
                    data->Surrogate = 0;
                }
                wind->KeyTypedEvent.emit(null, {wind, cp});
            }
			break;
        }
        case WM_UNICHAR:
            // We return 1 the first time to tell Windows we support utf-32 characters
            if (wParam == UNICODE_NOCHAR) return 1;
            wind->KeyTypedEvent.emit(null, {wind, (char32_t) wParam});
            break;
        case WM_LBUTTONDOWN:
            wind->MouseButtonPressedEvent.emit(null, {wind, Mouse_Button_Left, MOUSE_EVENT_GET_MODS(wParam),
                                                      GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_LBUTTONUP:
            wind->MouseButtonReleasedEvent.emit(null, {wind, Mouse_Button_Left, MOUSE_EVENT_GET_MODS(wParam),
                                                       GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_MBUTTONDOWN:
            wind->MouseButtonPressedEvent.emit(null, {wind, Mouse_Button_Middle, MOUSE_EVENT_GET_MODS(wParam),
                                                      GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_MBUTTONUP:
            wind->MouseButtonReleasedEvent.emit(null, {wind, Mouse_Button_Middle, MOUSE_EVENT_GET_MODS(wParam),
                                                       GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_RBUTTONDOWN:
            wind->MouseButtonPressedEvent.emit(null, {wind, Mouse_Button_Right, MOUSE_EVENT_GET_MODS(wParam),
                                                      GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_RBUTTONUP:
            wind->MouseButtonReleasedEvent.emit(null, {wind, Mouse_Button_Right, MOUSE_EVENT_GET_MODS(wParam),
                                                       GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_XBUTTONDOWN:
            wind->MouseButtonPressedEvent.emit(
                null, {wind, HIWORD(wParam) == XBUTTON1 ? Mouse_Button_X1 : Mouse_Button_X2,
                       MOUSE_EVENT_GET_MODS(LOWORD(wParam)), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_XBUTTONUP:
            wind->MouseButtonReleasedEvent.emit(
                null, {wind, HIWORD(wParam) == XBUTTON1 ? Mouse_Button_X1 : Mouse_Button_X2,
                       MOUSE_EVENT_GET_MODS(LOWORD(wParam)), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_MOUSEHWHEEL:
            wind->MouseScrolledEvent.emit(
                null, {wind, GET_WHEEL_DELTA_WPARAM(wParam), 0, MOUSE_EVENT_GET_MODS(LOWORD(wParam)),
                       MOUSE_EVENT_GET_BUTTONS_DOWN(LOWORD(wParam)), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_MOUSEWHEEL:
            wind->MouseScrolledEvent.emit(
                null, {wind, 0, GET_WHEEL_DELTA_WPARAM(wParam), MOUSE_EVENT_GET_MODS(LOWORD(wParam)),
                       MOUSE_EVENT_GET_BUTTONS_DOWN(LOWORD(wParam)), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_MOUSEMOVE:
            if (!data->MouseInClient) {
                data->MouseInClient = true;
                wind->MouseEnteredEvent.emit(null, {wind});

                TRACKMOUSEEVENT tme;
                fill_memory(&tme, 0, sizeof(tme));

                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);
            }

            wind->MouseMovedEvent.emit(null, {wind, MOUSE_EVENT_GET_MODS(wParam), MOUSE_EVENT_GET_BUTTONS_DOWN(wParam),
                                              GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_MOUSELEAVE:
            data->MouseInClient = false;
            wind->MouseLeftEvent.emit(null, {wind});
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

extern "C" IMAGE_DOS_HEADER __ImageBase;

window *window::init(string title, u32 width, u32 height) {
    // Pray that people aren't insane and going to put such long window titles
    Context.init_temporary_allocator(500_KiB);

    static_assert(sizeof(PlatformData) >= sizeof(windows_data));  // Sanity

    constexpr wchar_t CLASS_NAME[] = L"Le engine window class";

    HINSTANCE hInstance = ((HINSTANCE) &__ImageBase);

    WNDCLASSEXW wcex;
    fill_memory(&wcex, 0, sizeof(wcex));

    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbWndExtra = sizeof(window *);
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIconW(null, IDI_WINLOGO);
    wcex.hCursor = LoadCursorW(null, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wcex.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wcex)) {
        fmt::print("INTERNAL PLATFORM ERROR (Windows): Couldn't register window class. ({})\n",
                   get_last_error_as_string());
        os_exit(-1);
    }

    RECT rect = {0, 0, (s32) width, (s32) height};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, false,
                       WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);

    PDATA->hWnd = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, wcex.lpszClassName, L"",
                                  WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
                                  rect.right - rect.left, rect.bottom - rect.top, null, null, hInstance, this);
    if (!PDATA->hWnd) {
        fmt::print("INTERNAL PLATFORM ERROR (Windows): Couldn't create window. ({})\n", get_last_error_as_string());
        os_exit(-1);
    }

    GetWindowRect(PDATA->hWnd, &rect);
    Left = (s32) rect.left;
    Top = (s32) rect.top;
    Width = (u32) rect.right - rect.left;
    Height = (u32) rect.bottom - rect.top;

    clone(&Title, title);
    update_title();

    ShowWindow(PDATA->hWnd, SW_SHOW);
    SetFocus(PDATA->hWnd);

    WindowResizedEvent.connect({this, &window::on_window_resized});
    WindowMovedEvent.connect({this, &window::on_window_moved});

    return this;
}

void window::update() {
    MSG message;
    while (PeekMessageW(&message, null, 0, 0, PM_REMOVE) > 0) {
        if (message.message == WM_QUIT) {
            Closed = true;
            return;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    Context.TemporaryAlloc.free_all();
}

void window::update_title() {
    auto *titleUtf16 = new (Context.TemporaryAlloc) wchar_t[Title.Length];
    utf8_to_utf16(Title.Data, Title.Length, titleUtf16);

    SetWindowTextW(PDATA->hWnd, titleUtf16);
}

void window::update_bounds() {
    // SetWindowPos wants the total size of the window (including title bar and borders), so we have to compute it
    RECT rectangle = {0, 0, (s32) Width, (s32) Height};
    AdjustWindowRect(&rectangle, GetWindowLong(PDATA->hWnd, GWL_STYLE), false);

    s32 adjustedWidth = rectangle.right - rectangle.left;
    s32 adjustedHeight = rectangle.bottom - rectangle.top;
    SetWindowPos(PDATA->hWnd, null, Left, Top, adjustedWidth, adjustedHeight, SWP_NOZORDER);
}
}  // namespace le

#endif