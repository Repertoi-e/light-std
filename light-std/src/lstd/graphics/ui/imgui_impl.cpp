#include "imgui_impl.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"

#include <Windows.h>

// Data
static window *g_Window = null;
static graphics *g_Graphics = null;

static time_t g_Time = 0;
static bool g_MouseJustPressed[Mouse_Button_Last + 1]{};
static cursor g_MouseCursors[ImGuiMouseCursor_COUNT]{};
static bool g_WantUpdateMonitors = true;

// Forward Declarations
static void ImGui_Impl_InitPlatformInterface();
static void ImGui_Impl_UpdateMonitors();

static const char *ImGui_Impl_GetClipboardText(void *user_data) {
    return os_get_clipboard_content().to_c_string();  // @Leak
}

static void ImGui_Impl_SetClipboardText(void *user_data, const char *text) { os_set_clipboard_content(string(text)); }

bool ImGui_Impl_Init(window *win, graphics *g) {
    g_Window = win;
    g_Graphics = g;
    g_Time = 0;

    // Setup back-end capabilities flags
    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;  // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |=
        ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |=
        ImGuiBackendFlags_PlatformHasViewports;  // We can create multi-viewports on the Platform side (optional)
    io.BackendFlags |=
        ImGuiBackendFlags_HasMouseHoveredViewport;  // We can set io.MouseHoveredViewport correctly (optional, not easy)
    io.BackendPlatformName = "imgui_impl_glfw";

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = Key_Tab;
    io.KeyMap[ImGuiKey_LeftArrow] = Key_Left;
    io.KeyMap[ImGuiKey_RightArrow] = Key_Right;
    io.KeyMap[ImGuiKey_UpArrow] = Key_Up;
    io.KeyMap[ImGuiKey_DownArrow] = Key_Down;
    io.KeyMap[ImGuiKey_PageUp] = Key_PageUp;
    io.KeyMap[ImGuiKey_PageDown] = Key_PageDown;
    io.KeyMap[ImGuiKey_Home] = Key_Home;
    io.KeyMap[ImGuiKey_End] = Key_End;
    io.KeyMap[ImGuiKey_Insert] = Key_Insert;
    io.KeyMap[ImGuiKey_Delete] = Key_DeleteForward;
    io.KeyMap[ImGuiKey_Backspace] = Key_Delete;  // @TODO: Really?
    io.KeyMap[ImGuiKey_Space] = Key_Space;
    io.KeyMap[ImGuiKey_Enter] = Key_Enter;
    io.KeyMap[ImGuiKey_Escape] = Key_Escape;
    io.KeyMap[ImGuiKey_KeyPadEnter] = KeyPad_Enter;
    io.KeyMap[ImGuiKey_A] = Key_A;
    io.KeyMap[ImGuiKey_C] = Key_C;
    io.KeyMap[ImGuiKey_V] = Key_V;
    io.KeyMap[ImGuiKey_X] = Key_X;
    io.KeyMap[ImGuiKey_Y] = Key_Y;
    io.KeyMap[ImGuiKey_Z] = Key_Z;

    io.SetClipboardTextFn = ImGui_Impl_SetClipboardText;
    io.GetClipboardTextFn = ImGui_Impl_GetClipboardText;

    g_MouseCursors[ImGuiMouseCursor_Arrow] = cursor(OS_ARROW);
    g_MouseCursors[ImGuiMouseCursor_TextInput] = cursor(OS_IBEAM);
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = cursor(OS_RESIZE_ALL);
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] = cursor(OS_RESIZE_NS);
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] = cursor(OS_RESIZE_WE);
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = cursor(OS_RESIZE_NESW);
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = cursor(OS_RESIZE_NWSE);
    g_MouseCursors[ImGuiMouseCursor_Hand] = cursor(OS_HAND);

    win->KeyEvent.connect([](const key_event &e) {
        ImGuiIO &io = ImGui::GetIO();
        if (e.Action == Key_Pressed) io.KeysDown[e.KeyCode] = true;
        if (e.Action == Key_Released) io.KeysDown[e.KeyCode] = false;

        io.KeyCtrl = io.KeysDown[Key_LeftControl] || io.KeysDown[Key_RightControl];
        io.KeyShift = io.KeysDown[Key_LeftShift] || io.KeysDown[Key_RightShift];
        io.KeyAlt = io.KeysDown[Key_LeftAlt] || io.KeysDown[Key_RightAlt];
        io.KeySuper = io.KeysDown[Key_LeftGUI] || io.KeysDown[Key_RightGUI];
        return false;
    });

    win->CodePointTypedEvent.connect([](const code_point_typed_event &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.AddInputCharacter(e.CP);
    });

    win->MouseButtonEvent.connect([](const mouse_button_event &e) {
        if (e.Pressed) g_MouseJustPressed[e.Button] = true;
        return false;
    });

    win->MouseScrolledEvent.connect([](const mouse_scrolled_event &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseWheelH += e.ScrollX;
        io.MouseWheel += e.ScrollY;
        return false;
    });

    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = (void *) g_Window;
#if OS == WINDOWS
    main_viewport->PlatformHandleRaw = g_Window->PlatformData.Win32.hWnd;
#endif
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) ImGui_Impl_InitPlatformInterface();
    return true;
}

static void ImGui_Impl_UpdateMousePosAndButtons() {
    // Update buttons
    ImGuiIO &io = ImGui::GetIO();

    io.MouseDown[0] = g_MouseJustPressed[Mouse_Button_Left] || g_Window->MouseButtons[Mouse_Button_Left];
    io.MouseDown[1] = g_MouseJustPressed[Mouse_Button_Right] || g_Window->MouseButtons[Mouse_Button_Right];
    io.MouseDown[2] = g_MouseJustPressed[Mouse_Button_Middle] || g_Window->MouseButtons[Mouse_Button_Middle];
    io.MouseDown[3] = g_MouseJustPressed[Mouse_Button_X1] || g_Window->MouseButtons[Mouse_Button_X1];
    io.MouseDown[4] = g_MouseJustPressed[Mouse_Button_X2] || g_Window->MouseButtons[Mouse_Button_X2];
    zero_memory(g_MouseJustPressed, sizeof(g_MouseJustPressed));

    // Update mouse position
    const ImVec2 mouse_pos_backup = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    io.MouseHoveredViewport = 0;
    ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
    for (int n = 0; n < platform_io.Viewports.Size; n++) {
        ImGuiViewport *viewport = platform_io.Viewports[n];
        window *win = (window *) viewport->PlatformHandle;
        if (win->Flags & window::FOCUSED) {
            if (io.WantSetMousePos) {
                win->set_cursor_pos((s32)(mouse_pos_backup.x - viewport->Pos.x),
                                    (s32)(mouse_pos_backup.y - viewport->Pos.y));
            } else {
                vec2i mouse = win->get_cursor_pos();
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the
                    // mouse is on the upper-left of the primary monitor)
                    vec2i windowPos = win->get_pos();
                    io.MousePos = ImVec2((f32)(mouse.x + windowPos.x), (f32)(mouse.y + windowPos.y));
                } else {
                    // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the
                    // mouse is on the upper-left corner of the app window)
                    io.MousePos = ImVec2((f32) mouse.x, (f32) mouse.y);
                }
            }

            io.MouseDown[0] = win->MouseButtons[Mouse_Button_Left];
            io.MouseDown[1] = win->MouseButtons[Mouse_Button_Right];
            io.MouseDown[2] = win->MouseButtons[Mouse_Button_Middle];
            io.MouseDown[3] = win->MouseButtons[Mouse_Button_X1];
            io.MouseDown[4] = win->MouseButtons[Mouse_Button_X2];
        }

        if (win->is_hovered() && !(viewport->Flags & ImGuiViewportFlags_NoInputs)) {
            io.MouseHoveredViewport = viewport->ID;
        }
    }
}

static void ImGui_Impl_UpdateMouseCursor() {
    ImGuiIO &io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || g_Window->CursorMode == window::CURSOR_DISABLED)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
    for (int n = 0; n < platform_io.Viewports.Size; n++) {
        window *win = (window *) platform_io.Viewports[n]->PlatformHandle;
        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            win->set_cursor_mode(window::CURSOR_HIDDEN);
        } else {
            // Show OS mouse cursor
            win->set_cursor(&g_MouseCursors[imgui_cursor]);
            win->set_cursor_mode(window::CURSOR_NORMAL);
        }
    }
}

void ImGui_Impl_NewFrame() {
    ImGuiIO &io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() &&
              "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer "
              "_NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    vec2i windowSize = g_Window->get_size();
    vec2i frameBufferSize = g_Window->get_framebuffer_size();
    io.DisplaySize = ImVec2((f32) windowSize.x, (f32) windowSize.y);
    io.DisplayFramebufferScale = ImVec2((f32) frameBufferSize.x / windowSize.x, (f32) frameBufferSize.y / windowSize.y);
    if (g_WantUpdateMonitors) ImGui_Impl_UpdateMonitors();

    // Setup time step
    time_t current_time = os_get_time();
    io.DeltaTime = g_Time > 0 ? (f32) os_time_to_seconds(current_time - g_Time) : (1.0f / 60.0f);
    g_Time = current_time;

    ImGui_Impl_UpdateMousePosAndButtons();
    ImGui_Impl_UpdateMouseCursor();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports
// simultaneously. If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you
// completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

struct ImGuiViewportDataGlfw {
    window *Window = null;
    bool WindowOwned = false;
};

static void ImGui_Impl_CreateWindow(ImGuiViewport *viewport) {
    ImGuiViewportDataGlfw *data = IM_NEW(ImGuiViewportDataGlfw)();
    viewport->PlatformUserData = data;

    u32 flags = window::RESIZABLE | window::VSYNC | window::MOUSE_PASS_THROUGH;
    // @TODO: Window size render target stuff is wrong when we have borders (comment the line below)
    if (viewport->Flags & ImGuiViewportFlags_NoDecoration) flags |= window::BORDERLESS;
    if (viewport->Flags & ImGuiViewportFlags_TopMost) flags |= window::ALWAYS_ON_TOP;

    data->Window =
        (new (Malloc) window)
            ->init("", window::DONT_CARE, window::DONT_CARE, (s32) viewport->Size.x, (s32) viewport->Size.y, flags);
    data->WindowOwned = true;
    g_Graphics->add_target_window(data->Window);

    viewport->PlatformHandle = (void *) data->Window;
#if OS == WINDOWS
    viewport->PlatformHandleRaw = data->Window->PlatformData.Win32.hWnd;
#endif

    data->Window->set_pos((s32) viewport->Pos.x, (s32) viewport->Pos.y);

    data->Window->KeyEvent.connect([](const key_event &e) {
        ImGuiIO &io = ImGui::GetIO();
        if (e.Action == Key_Pressed) io.KeysDown[e.KeyCode] = true;
        if (e.Action == Key_Released) {
            io.KeysDown[e.KeyCode] = false;
        }
        io.KeyCtrl = io.KeysDown[Key_LeftControl] || io.KeysDown[Key_RightControl];
        io.KeyShift = io.KeysDown[Key_LeftShift] || io.KeysDown[Key_RightShift];
        io.KeyAlt = io.KeysDown[Key_LeftAlt] || io.KeysDown[Key_RightAlt];
        io.KeySuper = io.KeysDown[Key_LeftGUI] || io.KeysDown[Key_RightGUI];
        return false;
    });

    data->Window->CodePointTypedEvent.connect([](const code_point_typed_event &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.AddInputCharacter(e.CP);
    });

    data->Window->MouseButtonEvent.connect([](const mouse_button_event &e) {
        if (e.Pressed) g_MouseJustPressed[e.Button] = true;
        return false;
    });

    data->Window->MouseScrolledEvent.connect([](const mouse_scrolled_event &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseWheelH += e.ScrollX;
        io.MouseWheel += e.ScrollY;
        return false;
    });

    data->Window->WindowClosedEvent.connect([](const window_closed_event &e) {
        if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(e.Window))
            viewport->PlatformRequestClose = true;
    });

    data->Window->WindowMovedEvent.connect([](const window_moved_event &e) {
        if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(e.Window)) {
            viewport->PlatformRequestMove = true;
        }
    });

    data->Window->WindowFramebufferResizedEvent.connect([](const window_framebuffer_resized_event &e) {
        if (ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(e.Window)) {
            viewport->PlatformRequestResize = true;
        }
    });
}

static void ImGui_Impl_DestroyWindow(ImGuiViewport *viewport) {
    if (ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData) {
        g_Graphics->remove_target_window(data->Window);
        IM_DELETE(data->Window);
        IM_DELETE(data);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = null;
}

static void ImGui_Impl_ShowWindow(ImGuiViewport *viewport) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;

#if OS == WINDOWS
    // @Hack Hide icon from task bar
    HWND hwnd = (HWND) viewport->PlatformHandleRaw;
    if (viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon) {
        LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ex_style &= ~WS_EX_APPWINDOW;
        ex_style |= WS_EX_TOOLWINDOW;
        ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
    }
#endif

    data->Window->show();
}

static ImVec2 ImGui_Impl_GetWindowPos(ImGuiViewport *viewport) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    vec2i pos = data->Window->get_pos();
    return ImVec2((f32) pos.x, (f32) pos.y);
}

static void ImGui_Impl_SetWindowPos(ImGuiViewport *viewport, ImVec2 pos) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    data->Window->set_pos((s32) pos.x, (s32) pos.y);
}

static ImVec2 ImGui_Impl_GetWindowSize(ImGuiViewport *viewport) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    vec2i size = data->Window->get_size();
    return ImVec2((f32) size.x, (f32) size.y);
}

static void ImGui_Impl_SetWindowSize(ImGuiViewport *viewport, ImVec2 size) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    data->Window->set_size((s32) size.x, (s32) size.y);
}

static void ImGui_Impl_SetWindowTitle(ImGuiViewport *viewport, const char *title) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    data->Window->set_title(string(title));
}

static void ImGui_Impl_SetWindowFocus(ImGuiViewport *viewport) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    data->Window->focus();
}

static bool ImGui_Impl_GetWindowFocus(ImGuiViewport *viewport) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    return data->Window->Flags & window::FOCUSED;
}

static bool ImGui_Impl_GetWindowMinimized(ImGuiViewport *viewport) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    return data->Window->Flags & window::MINIMIZED;
}

static void ImGui_Impl_SetWindowAlpha(ImGuiViewport *viewport, f32 alpha) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    data->Window->set_opacity(alpha);
}

static void ImGui_Impl_RenderWindow(ImGuiViewport *viewport, void *) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    g_Graphics->set_current_target_window(data->Window);
}

static void ImGui_Impl_SwapBuffers(ImGuiViewport *viewport, void *) {
    ImGuiViewportDataGlfw *data = (ImGuiViewportDataGlfw *) viewport->PlatformUserData;
    g_Graphics->set_current_target_window(data->Window);  // @Redundant ?
    g_Graphics->swap();
}

//--------------------------------------------------------------------------------------------------------
// IME (Input Method Editor) basic support for e.g. Asian language users
//--------------------------------------------------------------------------------------------------------

// We provide a Win32 implementation because this is such a common issue for IME users
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && \
    !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS) && !defined(__GNUC__)
#define HAS_WIN32_IME 1
#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif
static void ImGui_ImplWin32_SetImeInputPos(ImGuiViewport *viewport, ImVec2 pos) {
    COMPOSITIONFORM cf = {
        CFS_FORCE_POSITION, {(LONG)(pos.x - viewport->Pos.x), (LONG)(pos.y - viewport->Pos.y)}, {0, 0, 0, 0}};
    if (HWND hwnd = (HWND) viewport->PlatformHandleRaw)
        if (HIMC himc = ::ImmGetContext(hwnd)) {
            ::ImmSetCompositionWindow(himc, &cf);
            ::ImmReleaseContext(hwnd, himc);
        }
}
#else
#define HAS_WIN32_IME 0
#endif

static void ImGui_Impl_UpdateMonitors() {
    ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();

    platform_io.Monitors.resize(0);
    For(os_get_monitors()) {
        vec2i pos = os_get_monitor_pos(it);
        auto displayMode = os_get_current_display_mode(it);

        ImGuiPlatformMonitor monitor;
        monitor.MainPos = ImVec2((f32) pos.x, (f32) pos.y);
        monitor.MainSize = ImVec2((f32) displayMode.Width, (f32) displayMode.Height);

        rect workArea = os_get_work_area(it);
        monitor.WorkPos = ImVec2((f32) workArea.X, (f32) workArea.Y);
        monitor.WorkSize = ImVec2((f32) workArea.Width, (f32) workArea.Height);

        vec2 scale = os_get_monitor_content_scale(it);
        monitor.DpiScale = scale.x;

        platform_io.Monitors.push_back(monitor);
    }
    g_WantUpdateMonitors = false;
}

static void ImGui_Impl_InitPlatformInterface() {
    // Register platform interface (will be coupled with a renderer interface)
    ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_CreateWindow = ImGui_Impl_CreateWindow;
    platform_io.Platform_DestroyWindow = ImGui_Impl_DestroyWindow;
    platform_io.Platform_ShowWindow = ImGui_Impl_ShowWindow;
    platform_io.Platform_SetWindowPos = ImGui_Impl_SetWindowPos;
    platform_io.Platform_GetWindowPos = ImGui_Impl_GetWindowPos;
    platform_io.Platform_SetWindowSize = ImGui_Impl_SetWindowSize;
    platform_io.Platform_GetWindowSize = ImGui_Impl_GetWindowSize;
    platform_io.Platform_SetWindowFocus = ImGui_Impl_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = ImGui_Impl_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = ImGui_Impl_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = ImGui_Impl_SetWindowTitle;
    platform_io.Platform_RenderWindow = ImGui_Impl_RenderWindow;
    platform_io.Platform_SwapBuffers = ImGui_Impl_SwapBuffers;
    platform_io.Platform_SetWindowAlpha = ImGui_Impl_SetWindowAlpha;
#if HAS_WIN32_IME
    platform_io.Platform_SetImeInputPos = ImGui_ImplWin32_SetImeInputPos;
#endif

    // Note: monitor callback are broken GLFW 3.2 and earlier (see github.com/glfw/glfw/issues/784)
    ImGui_Impl_UpdateMonitors();
    g_MonitorEvent.connect([](auto) { g_WantUpdateMonitors = true; });

    // Register main window handle (which is owned by the main application, not by us)
    ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ImGuiViewportDataGlfw *data = IM_NEW(ImGuiViewportDataGlfw)();
    data->Window = g_Window;
    data->WindowOwned = false;
    main_viewport->PlatformUserData = data;
    main_viewport->PlatformHandle = (void *) g_Window;
}
