#include "le/window/window.hpp"

#if OS == WINDOWS

#include <lstd/fmt.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Windowsx.h>

namespace le {

struct Windows_Data {
    HWND hWnd;
    wchar_t *LastTitle;
    Allocator_Closure LastTitleAllocator;
    bool MouseInClient = true;
};
#define PDATA ((Windows_Data *) PlatformData)

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
    auto *window = (Window *) GetWindowLongPtrW(hWnd, 0);
    auto *data = (Windows_Data *) window->PlatformData;

    switch (message) {
        case WM_NCCREATE:
            window = (Window *) ((CREATESTRUCT *) lParam)->lpCreateParams;
            SetWindowLongPtrW(hWnd, 0, (ptr_t) window);
            SetWindowPos(hWnd, null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
            break;
        case WM_CLOSE:
            window->WindowClosedEvent.emit({window});
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            if (wParam == SIZE_RESTORED) {
                window->WindowResizedEvent.emit({window, (u32) LOWORD(lParam), (u32) HIWORD(lParam)});
            }
        case WM_SETFOCUS:
            window->WindowGainedFocusEvent.emit({window});
            break;
        case WM_KILLFOCUS:
            window->WindowLostFocusEvent.emit({window});
            break;
        case WM_MOVE:
            window->WindowMovedEvent.emit({window, (u32) LOWORD(lParam), (u32) HIWORD(lParam)});
            break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            window->KeyPressedEvent.emit(
                {window, KEYCODE_NATIVE_TO_HID[wParam], KEY_EVENT_GET_MODS, (bool) (lParam & 0xffff)});
            break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
            window->KeyReleasedEvent.emit({window, KEYCODE_NATIVE_TO_HID[wParam], KEY_EVENT_GET_MODS});
            break;
        case WM_UNICHAR:
            // We return 1 the first time to tell Windows we support utf-32 characters
            if (wParam == UNICODE_NOCHAR) return 1;
            window->KeyTypedEvent.emit({window, (char32_t) wParam});
            break;
        case WM_LBUTTONDOWN:
            window->MouseButtonPressedEvent.emit(
                {window, Mouse_Button_Left, MOUSE_EVENT_GET_MODS(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_LBUTTONUP:
            window->MouseButtonReleasedEvent.emit(
                {window, Mouse_Button_Left, MOUSE_EVENT_GET_MODS(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_MBUTTONDOWN:
            window->MouseButtonPressedEvent.emit({window, Mouse_Button_Middle, MOUSE_EVENT_GET_MODS(wParam),
                                                  GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_MBUTTONUP:
            window->MouseButtonReleasedEvent.emit({window, Mouse_Button_Middle, MOUSE_EVENT_GET_MODS(wParam),
                                                   GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_RBUTTONDOWN:
            window->MouseButtonPressedEvent.emit(
                {window, Mouse_Button_Right, MOUSE_EVENT_GET_MODS(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_RBUTTONUP:
            window->MouseButtonReleasedEvent.emit(
                {window, Mouse_Button_Right, MOUSE_EVENT_GET_MODS(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_XBUTTONDOWN:
            window->MouseButtonPressedEvent.emit(
                {window, HIWORD(wParam) == XBUTTON1 ? Mouse_Button_X1 : Mouse_Button_X2,
                 MOUSE_EVENT_GET_MODS(LOWORD(wParam)), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_XBUTTONUP:
            window->MouseButtonReleasedEvent.emit(
                {window, HIWORD(wParam) == XBUTTON1 ? Mouse_Button_X1 : Mouse_Button_X2,
                 MOUSE_EVENT_GET_MODS(LOWORD(wParam)), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            break;
        case WM_MOUSEHWHEEL:
            window->MouseScrolledEvent.emit({window, GET_WHEEL_DELTA_WPARAM(wParam), 0, GET_X_LPARAM(lParam),
                                             GET_Y_LPARAM(lParam), MOUSE_EVENT_GET_MODS(LOWORD(wParam)),
                                             MOUSE_EVENT_GET_BUTTONS_DOWN(LOWORD(wParam))});
            break;
        case WM_MOUSEWHEEL:
            window->MouseScrolledEvent.emit({window, 0, GET_WHEEL_DELTA_WPARAM(wParam), GET_X_LPARAM(lParam),
                                             GET_Y_LPARAM(lParam), MOUSE_EVENT_GET_MODS(LOWORD(wParam)),
                                             MOUSE_EVENT_GET_BUTTONS_DOWN(LOWORD(wParam))});
            break;
        case WM_MOUSEMOVE:
            if (!data->MouseInClient) {
                data->MouseInClient = true;
                window->MouseEnteredEvent.emit({window});

                TRACKMOUSEEVENT tme;
                zero_memory(&tme, sizeof(tme));

                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);
            }

            window->MouseMovedEvent.emit({window, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
                                          MOUSE_EVENT_GET_MODS(wParam), MOUSE_EVENT_GET_BUTTONS_DOWN(wParam)});
            break;
        case WM_MOUSELEAVE:
            data->MouseInClient = false;
            window->MouseLeftEvent.emit({window});
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

extern "C" IMAGE_DOS_HEADER __ImageBase;

Window::Window(const string &title, u32 width, u32 height) : Title(title) {
    constexpr wchar_t CLASS_NAME[] = L"Le engine window class";

    HINSTANCE hInstance = ((HINSTANCE) &__ImageBase);

    WNDCLASSEXW wcex;
    zero_memory(&wcex, sizeof(wcex));

    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbWndExtra = sizeof(Window *);
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIconW(NULL, IDI_WINLOGO);
    wcex.hCursor = LoadCursorW(hInstance, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wcex.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wcex)) {
        fmt::print("INTERNAL PLATFORM ERROR (Windows): Couldn't register window class. \n");
        os_exit_program(-1);
    }

    RECT rect = {0, 0, (s32) width, (s32) height};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, false,
                       WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);

    PDATA->hWnd = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, wcex.lpszClassName, title.to_utf16(),
                                  WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CW_USEDEFAULT, 0,
                                  rect.right - rect.left, rect.bottom - rect.top, null, null, hInstance, this);

    if (!PDATA->hWnd) {
        fmt::print("INTERNAL PLATFORM ERROR (Windows): Couldn't create window. \n");
        os_exit_program(-1);
    }

    GetWindowRect(PDATA->hWnd, &rect);
    Left = (u32) rect.left;
    Top = (u32) rect.top;
    Width = (u32) rect.right - rect.left;
    Height = (u32) rect.bottom - rect.top;

    ShowWindow(PDATA->hWnd, SW_SHOW);
    SetFocus(PDATA->hWnd);

    WindowResizedEvent.connect({this, &Window::on_window_resized});
    WindowMovedEvent.connect({this, &Window::on_window_moved});
}

void Window::update() {
    MSG message;
    while (PeekMessageW(&message, null, 0, 0, PM_REMOVE) > 0) {
        if (message.message == WM_QUIT) {
            Closed = true;
            return;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

void Window::set_title(const string &title) {
    Title = title;

    if (PDATA->LastTitle) {
        Delete(PDATA->LastTitle, PDATA->LastTitleAllocator);
    }

    PDATA->LastTitle = title.to_utf16();
    PDATA->LastTitleAllocator = title.Allocator;
    SetWindowTextW(PDATA->hWnd, PDATA->LastTitle);
}

void Window::set_vsync(bool enabled) { VSyncEnabled = enabled; }

void Window::set_left(u32 left) { SetWindowPos(PDATA->hWnd, null, left, Top, 0, 0, SWP_NOZORDER | SWP_NOSIZE); }
void Window::set_top(u32 top) { SetWindowPos(PDATA->hWnd, null, Left, top, 0, 0, SWP_NOZORDER | SWP_NOSIZE); }
void Window::set_width(u32 width) { SetWindowPos(PDATA->hWnd, null, 0, 0, width, Height, SWP_NOZORDER | SWP_NOMOVE); }
void Window::set_height(u32 height) { SetWindowPos(PDATA->hWnd, null, 0, 0, Width, height, SWP_NOZORDER | SWP_NOMOVE); }

}  // namespace le

#endif