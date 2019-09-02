#include "game.h"

#if defined LE_BUILDING_GAME
#error Error
#endif

#include <lstd/dx_graphics.h>
#include <lstd/file.h>
#include <lstd/graphics/ui/imgui.h>
#include <lstd/graphics/ui/imgui_renderer.h>
#include <lstd/io/fmt.h>
#include <lstd/memory/dynamic_library.h>
#include <lstd/os.h>

static dynamic_library GameLibrary;
static game_update_and_render_func *GameUpdateAndRender = null;
static game_render_ui_func *GameRenderUI = null;

static file::handle *DLL = null,
                    *Buildlock = null;  // Allocated dynamically because we can't assign to already constructed

void setup_game_paths() {
    auto exePath = file::path(os_get_exe_name());

    file::path dllPath = exePath.directory();
    dllPath.combine_with("tetris.dll");
    DLL = new file::handle(dllPath);

    file::path buildLockPath = exePath.directory();
    buildLockPath.combine_with("buildlock");
    Buildlock = new file::handle(buildLockPath);
}

// @TODO: This fails in Dist configuration for some reason
bool reload_game_code() {
    if (GameLibrary.Handle) GameLibrary.close();

    file::path copyPath = DLL->Path.directory();
    copyPath.combine_with("loaded_game_code.dll");

    auto dllCopyHandle = file::handle(copyPath);
    assert(DLL->copy(dllCopyHandle, true));

    if (!GameLibrary.load(copyPath.UnifiedPath)) {
        fmt::print("Error: Couldn't load {} (copied from {}) as the game code for the engine\n", copyPath, *DLL);
        return false;
    }

    GameUpdateAndRender = (game_update_and_render_func *) GameLibrary.get_symbol("game_update_and_render");
    if (!GameUpdateAndRender) {
        fmt::print("Error: Couldn't load game_update_and_render\n");
        return false;
    }

    GameRenderUI = (game_render_ui_func *) GameLibrary.get_symbol("game_render_ui");
    if (!GameRenderUI) {
        fmt::print("Error: Couldn't load game_render_ui\n");
        return false;
    }
    return true;
}

// Returns true if the game was reloaded
bool check_for_dll_change() {
    static time_t checkTimer = 0, lastTime = 0;

    if (!Buildlock->exists() && (checkTimer % 20 == 0)) {
        auto writeTime = DLL->last_modification_time();
        if (writeTime != lastTime) {
            lastTime = writeTime;
            return reload_game_code();
        }
    }
    ++checkTimer;
    return false;
}

static void *new_wrapper(size_t size, void *) { return operator new(size, Malloc); }
static void delete_wrapper(void *ptr, void *) { delete ptr; }

void init_imgui_for_our_windows(window *mainWindow, graphics *g);
void imgui_for_our_windows_new_frame(window *mainWindow);

s32 main() {
    setup_game_paths();

    game_memory gameMemory;
    gameMemory.MainWindow = (new window)
                                ->init("Tetris", window::DONT_CARE, window::DONT_CARE, 1200, 600,
                                       window::SHOWN | window::RESIZABLE | window::VSYNC | window::CLOSE_ON_ALT_F4);

    dx_graphics g;
    g.init();
    g.set_blend(true);
    g.set_depth_testing(false);
    g.add_target_window(gameMemory.MainWindow);
    defer(g.remove_target_window(gameMemory.MainWindow));

    ImGui::CreateContext();
    gameMemory.ImGuiContext = ImGui::GetCurrentContext();

    ImGui::SetAllocatorFunctions(new_wrapper, delete_wrapper);

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts = new ImFontAtlas();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;
    // io.ConfigViewportsNoDefaultParent = true;
    // io.ConfigDockingAlwaysTabBar = true;
    // io.ConfigDockingTransparentPayload = true;

    ImGui::StyleColorsDark();

    // We tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Needs to get called at the end to release any imgui windows
    defer(ImGui::DestroyPlatformWindows());

    imgui_renderer imguiRenderer;
    imguiRenderer.init(&g);

    init_imgui_for_our_windows(gameMemory.MainWindow, &g);

    while (true) {
        gameMemory.ReloadedThisFrame = check_for_dll_change();

        window::update();
        if (gameMemory.MainWindow->IsDestroying) break;

        if (GameUpdateAndRender) GameUpdateAndRender(&gameMemory, &g);

        imgui_for_our_windows_new_frame(gameMemory.MainWindow);

        ImGui::NewFrame();
        if (GameRenderUI) GameRenderUI();
        ImGui::Render();

        g.set_current_target_window(gameMemory.MainWindow);
        g.set_cull_mode(cull::None);
        imguiRenderer.draw(ImGui::GetDrawData());
        g.swap();

        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault(null, &imguiRenderer);
        }
    }
}

#if OS == WINDOWS
#undef MAC
#undef _MAC
#include <Windows.h>  // For IME
#endif

static graphics *Graphics = null;
static cursor MouseCursors[ImGuiMouseCursor_COUNT] = {
    cursor(OS_ARROW),     cursor(OS_IBEAM),       cursor(OS_RESIZE_ALL),  cursor(OS_RESIZE_NS),
    cursor(OS_RESIZE_WE), cursor(OS_RESIZE_NESW), cursor(OS_RESIZE_NWSE), cursor(OS_HAND)};

static bool MouseButtons[Mouse_Button_Last + 1]{};

static bool common_key_callback(const key_event &e) {
    ImGuiIO &io = ImGui::GetIO();
    if (e.Action == Key_Pressed) io.KeysDown[e.KeyCode] = true;
    if (e.Action == Key_Released) io.KeysDown[e.KeyCode] = false;

    io.KeyCtrl = io.KeysDown[Key_LeftControl] || io.KeysDown[Key_RightControl];
    io.KeyShift = io.KeysDown[Key_LeftShift] || io.KeysDown[Key_RightShift];
    io.KeyAlt = io.KeysDown[Key_LeftAlt] || io.KeysDown[Key_RightAlt];
    io.KeySuper = io.KeysDown[Key_LeftGUI] || io.KeysDown[Key_RightGUI];
    return false;
}

static void common_code_point_callback(const code_point_typed_event &e) {
    ImGuiIO &io = ImGui::GetIO();
    io.AddInputCharacter(e.CP);
}

static bool common_mouse_button_callback(const mouse_button_event &e) {
    if (e.Pressed) MouseButtons[e.Button] = true;
    return false;
}

static bool common_mouse_scrolled_callback(const mouse_scrolled_event &e) {
    ImGuiIO &io = ImGui::GetIO();
    io.MouseWheelH += e.ScrollX;
    io.MouseWheel += e.ScrollY;
    return false;
}

// We provide a Win32 implementation because this is such a common issue for IME users
#if OS == WINDOWS && !defined IMGUI_DISABLE_WIN32_FUNCTIONS && !defined IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS && \
    !defined(__GNUC__)
#define HAS_WIN32_IME 1
#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif
static void imgui_set_ime_pos(ImGuiViewport *viewport, ImVec2 pos) {
    COMPOSITIONFORM cf = {
        CFS_FORCE_POSITION, {(s32)(pos.x - viewport->Pos.x), (s32)(pos.y - viewport->Pos.y)}, {0, 0, 0, 0}};
    if (HWND hWnd = (HWND) viewport->PlatformHandleRaw)
        if (HIMC himc = ::ImmGetContext(hWnd)) {
            ::ImmSetCompositionWindow(himc, &cf);
            ::ImmReleaseContext(hWnd, himc);
        }
}
#else
#define HAS_WIN32_IME 0
#endif

static void imgui_update_monitors() {
    ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();

    platformIO.Monitors.resize(0);
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

        platformIO.Monitors.push_back(monitor);
    }
}

static void init_imgui_for_our_windows(window *mainWindow, graphics *g) {
    Graphics = g;

    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;
    io.BackendPlatformName = "lstd";

    io.SetClipboardTextFn = [](auto, const char *text) { os_set_clipboard_content(string(text)); };
    io.GetClipboardTextFn = [](auto) {
        return os_get_clipboard_content().to_c_string();  // Leak
    };

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
    io.KeyMap[ImGuiKey_Delete] = Key_Delete;
    io.KeyMap[ImGuiKey_Backspace] = Key_Backspace;
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

    mainWindow->KeyEvent.connect(common_key_callback);
    mainWindow->CodePointTypedEvent.connect(common_code_point_callback);
    mainWindow->MouseButtonEvent.connect(common_mouse_button_callback);
    mainWindow->MouseScrolledEvent.connect(common_mouse_scrolled_callback);

    ImGuiViewport *mainViewport = ImGui::GetMainViewport();
    mainViewport->PlatformHandle = (void *) mainWindow;
#if OS == WINDOWS
    mainViewport->PlatformHandleRaw = mainWindow->PlatformData.Win32.hWnd;
#endif

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
        platformIO.Platform_CreateWindow = [](auto *viewport) {
            u32 flags = window::RESIZABLE | window::VSYNC | window::MOUSE_PASS_THROUGH;
            // @TODO: Window size render target stuff is wrong when we have borders (comment the line below)
            if (viewport->Flags & ImGuiViewportFlags_NoDecoration) flags |= window::BORDERLESS;
            if (viewport->Flags & ImGuiViewportFlags_TopMost) flags |= window::ALWAYS_ON_TOP;

            auto *win = (new (Malloc) window)
                            ->init("", window::DONT_CARE, window::DONT_CARE, (s32) viewport->Size.x,
                                   (s32) viewport->Size.y, flags);
            Graphics->add_target_window(win);

            viewport->PlatformUserData = (void *) true;
            viewport->PlatformHandle = (void *) win;
#if OS == WINDOWS
            viewport->PlatformHandleRaw = win->PlatformData.Win32.hWnd;
#endif
            win->set_pos((s32) viewport->Pos.x, (s32) viewport->Pos.y);

            win->KeyEvent.connect(common_key_callback);
            win->CodePointTypedEvent.connect(common_code_point_callback);
            win->MouseButtonEvent.connect(common_mouse_button_callback);
            win->MouseScrolledEvent.connect(common_mouse_scrolled_callback);

            win->WindowClosedEvent.connect(
                [](const auto &e) { ImGui::FindViewportByPlatformHandle(e.Window)->PlatformRequestClose = true; });
            win->WindowMovedEvent.connect(
                [](const auto &e) { ImGui::FindViewportByPlatformHandle(e.Window)->PlatformRequestMove = true; });
            win->WindowResizedEvent.connect(
                [](const auto &e) { ImGui::FindViewportByPlatformHandle(e.Window)->PlatformRequestResize = true; });
        };

        platformIO.Platform_DestroyWindow = [](auto *viewport) {
            if (viewport->PlatformUserData == (void *) true) {
                Graphics->remove_target_window((window *) viewport->PlatformHandle);
                delete (window *) viewport->PlatformHandle;
            }
            viewport->PlatformHandle = viewport->PlatformUserData = null;
        };

        platformIO.Platform_ShowWindow = [](auto *viewport) {
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
            ((window *) viewport->PlatformHandle)->show();
        };

        platformIO.Platform_SetWindowPos = [](auto *viewport, ImVec2 pos) {
            ((window *) viewport->PlatformHandle)->set_pos((s32) pos.x, (s32) pos.y);
        };

        platformIO.Platform_GetWindowPos = [](auto *viewport) {
            auto pos = ((window *) viewport->PlatformHandle)->get_pos();
            return ImVec2((f32) pos.x, (f32) pos.y);
        };

        platformIO.Platform_SetWindowSize = [](auto *viewport, ImVec2 size) {
            ((window *) viewport->PlatformHandle)->set_size((s32) size.x, (s32) size.y);
        };

        platformIO.Platform_GetWindowSize = [](auto *viewport) {
            auto size = ((window *) viewport->PlatformHandle)->get_size();
            return ImVec2((f32) size.x, (f32) size.y);
        };

        platformIO.Platform_SetWindowFocus = [](auto *viewport) { ((window *) viewport->PlatformHandle)->focus(); };
        platformIO.Platform_GetWindowFocus = [](auto *viewport) {
            return (bool) (((window *) viewport->PlatformHandle)->Flags & window::FOCUSED);
        };

        platformIO.Platform_GetWindowMinimized = [](auto *viewport) {
            return (bool) (((window *) viewport->PlatformHandle)->Flags & window::MINIMIZED);
        };

        platformIO.Platform_SetWindowTitle = [](auto *viewport, const char *title) {
            ((window *) viewport->PlatformHandle)->set_title(string(title));
        };

        platformIO.Platform_RenderWindow = [](auto *viewport, auto) {
            Graphics->set_current_target_window((window *) viewport->PlatformHandle);
        };

        platformIO.Platform_SwapBuffers = [](auto *viewport, auto) {
            Graphics->set_current_target_window((window *) viewport->PlatformHandle);  // @Redundant?
            Graphics->swap();
        };

        platformIO.Platform_SetWindowAlpha = [](auto *viewport, f32 alpha) {
            ((window *) viewport->PlatformHandle)->set_opacity(alpha);
        };
#if HAS_WIN32_IME
        platformIO.Platform_SetImeInputPos = imgui_set_ime_pos;
#endif

        imgui_update_monitors();
        g_MonitorEvent.connect([](auto) { imgui_update_monitors(); });
    }
}

static void imgui_for_our_windows_new_frame(window *mainWindow) {
    ImGuiIO &io = ImGui::GetIO();
    assert(io.Fonts->IsBuilt());

    vec2i windowSize = mainWindow->get_size();
    vec2i frameBufferSize = mainWindow->get_framebuffer_size();
    io.DisplaySize = ImVec2((f32) windowSize.x, (f32) windowSize.y);
    io.DisplayFramebufferScale = ImVec2((f32) frameBufferSize.x / windowSize.x, (f32) frameBufferSize.y / windowSize.y);

    // Setup time step
    static time_t lastTime = 0;

    time_t now = os_get_time();
    io.DeltaTime = lastTime > 0 ? (f32) os_time_to_seconds(now - lastTime) : (1.0f / 60.0f);
    lastTime = now;

    io.MouseDown[0] = MouseButtons[Mouse_Button_Left] || mainWindow->MouseButtons[Mouse_Button_Left];
    io.MouseDown[1] = MouseButtons[Mouse_Button_Right] || mainWindow->MouseButtons[Mouse_Button_Right];
    io.MouseDown[2] = MouseButtons[Mouse_Button_Middle] || mainWindow->MouseButtons[Mouse_Button_Middle];
    io.MouseDown[3] = MouseButtons[Mouse_Button_X1] || mainWindow->MouseButtons[Mouse_Button_X1];
    io.MouseDown[4] = MouseButtons[Mouse_Button_X2] || mainWindow->MouseButtons[Mouse_Button_X2];
    zero_memory(MouseButtons, sizeof(MouseButtons));

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    ImVec2 mousePosBackup = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    io.MouseHoveredViewport = 0;

    ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();

    For(platformIO.Viewports) {
        window *win = (window *) it->PlatformHandle;
        if (win->Flags & window::FOCUSED) {
            if (io.WantSetMousePos) {
                win->set_cursor_pos((s32)(mousePosBackup.x - it->Pos.x), (s32)(mousePosBackup.y - it->Pos.y));
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

        bool mousePassThrough = it->Flags & ImGuiViewportFlags_NoInputs;
        if (mousePassThrough) {
            win->Flags |= window::MOUSE_PASS_THROUGH;
        } else {
            win->Flags &= ~window::MOUSE_PASS_THROUGH;
        }
        if (win->is_hovered() && !mousePassThrough) io.MouseHoveredViewport = it->ID;
    }

    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || mainWindow->CursorMode == window::CURSOR_DISABLED)
        return;

    For(platformIO.Viewports) {
        window *win = (window *) it->PlatformHandle;
        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            win->set_cursor_mode(window::CURSOR_HIDDEN);
        } else {
            // Show OS mouse cursor
            win->set_cursor(&MouseCursors[imgui_cursor]);
            win->set_cursor_mode(window::CURSOR_NORMAL);
        }
    }
}
