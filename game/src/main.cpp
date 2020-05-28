#include "game.h"

#if defined LE_BUILDING_GAME
#error Error
#endif

#include <lstd/file.h>
#include <lstd/graphics.h>
#include <lstd/io/fmt.h>
#include <lstd/memory/dynamic_library.h>
#include <lstd/os.h>

// These can be modified with command line arguments.
u32 GameMemoryInMiB = 128;
u32 GameWidth = 1200, GameHeight = 600, GameFPS = 60;
string GameFileName = "cars.dll";

allocator GameAlloc;  // Imgui uses this, it needs to be global

dynamic_library GameLibrary;
game_update_and_render_func *GameUpdateAndRender = null;
game_main_window_event_func *GameMainWindowEvent = null;

file::handle *DLL = null, *Buildlock = null;  // file::handle is not assignable

void setup_game_paths() {
    assert(GameFileName != "");

    auto exePath = file::path(os_get_exe_name());

    file::path dllPath = exePath.directory();
    dllPath.combine_with(GameFileName);
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
    if (!DLL->copy(dllCopyHandle, true)) {
        fmt::print("Error: Couldn't write to {!YELLOW}\"{}\"{!}. Game already running?\n", dllCopyHandle);
        return false;
    }

    if (!GameLibrary.load(copyPath.UnifiedPath)) {
        fmt::print(
            "Error: Couldn't load {!YELLOW}\"{}\"{!} (copied from {!GRAY}\"{}\"{}) as the game code for the engine.\n",
            copyPath, *DLL);
        return false;
    }

    GameUpdateAndRender = (game_update_and_render_func *) GameLibrary.get_symbol("game_update_and_render");
    if (!GameUpdateAndRender) {
        fmt::print("Error: Couldn't load game_update_and_render\n");
        return false;
    }

    GameMainWindowEvent = (game_main_window_event_func *) GameLibrary.get_symbol("game_main_window_event");
    if (!GameMainWindowEvent) {
        fmt::print("Error: Couldn't load game_main_window_event\n");
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

void init_imgui_for_our_windows(window *mainWindow);
void imgui_for_our_windows_new_frame(window *mainWindow);

void parse_arguments() {
    array<string> usage;
    usage.append("Usage:\n");
    usage.append(
        "    {!YELLOW}-dll <name>{!GRAY}    "
        "Specifies which dll to hot load in the engine. By default it searches for cars.dll{!}\n\n");
    usage.append(
        "    {!YELLOW}-memory <amount>{!GRAY}    "
        "Specifies the amount of memory (in MiB) which gets reserved for the game (default is 128 MiB)."
        "Usage must never cross this.{!}\n\n");
    usage.append(
        "    {!YELLOW}-width <value>{!GRAY}    "
        "Specifies the width of the game window (default is 1200).{!}\n\n");
    usage.append(
        "    {!YELLOW}-heigth <value>{!GRAY}    "
        "Specifies the height of the game window (default is 600).\n\n");
    usage.append(
        "    {!YELLOW}-fps <value>{!GRAY}    "
        "Specifies the target fps (default is 60).\n\n");

    bool seekFileName = false, seekMemory = false, seekWidth = false, seekHeight = false, seekFPS = false;
    auto args = os_get_command_line_arguments();
    For(args) {
        if (seekFileName) {
            GameFileName = it;
            seekFileName = false;
            continue;
        }
        if (seekMemory) {
            auto old = GameMemoryInMiB;
            io::string_reader parser(it);
            parser.read(&GameMemoryInMiB);
            if (parser.LastFailed) {
                GameMemoryInMiB = old;
                fmt::to_writer(&io::cerr, ">>> {!RED}Invalid use of \"-memory\" argument. \n");
                fmt::to_writer(&io::cerr, "Expected a whole number as a parameter (instead got \"{}\").{!}\n", it);
            }
            seekMemory = false;
            continue;
        }
        if (seekWidth) {
            auto old = GameWidth;
            io::string_reader parser(it);
            parser.read(&GameWidth);
            if (parser.LastFailed) {
                GameWidth = old;
                fmt::to_writer(&io::cerr, ">>> {!RED}Invalid use of \"-width\" argument. \n");
                fmt::to_writer(&io::cerr, "Expected a whole number as a parameter (instead got \"{}\").{!}\n", it);
            }
            seekWidth = false;
            continue;
        }
        if (seekHeight) {
            auto old = GameHeight;
            io::string_reader parser(it);
            parser.read(&GameHeight);
            if (parser.LastFailed) {
                GameHeight = old;
                fmt::to_writer(&io::cerr, ">>> {!RED}Invalid use of \"-height\" argument. \n");
                fmt::to_writer(&io::cerr, "Expected a whole number as a parameter (instead got \"{}\").{!}\n", it);
            }
            seekHeight = false;
            continue;
        }
        if (seekFPS) {
            auto old = GameFPS;
            io::string_reader parser(it);
            parser.read(&GameFPS);
            if (parser.LastFailed) {
                GameFPS = old;
                fmt::to_writer(&io::cerr, ">>> {!RED}Invalid use of \"-fps\" argument. \n");
                fmt::to_writer(&io::cerr, "Expected a whole number as a parameter (instead got \"{}\").{!}\n", it);
            }
            seekFPS = false;
            continue;
        }
        if (it == "-dll") {
            seekFileName = true;
        } else if (it == "-memory") {
            seekMemory = true;
        } else if (it == "-width") {
            seekWidth = true;
        } else if (it == "-height") {
            seekHeight = true;
        } else if (it == "-fps") {
            seekFPS = true;
        } else {
            fmt::to_writer(&io::cerr, ">>> {!RED}Encountered invalid argument (\"{}\").{!}\n", it);
            For(usage) fmt::to_writer(&io::cerr, it);
            break;
        }
    }
    if (seekFileName) {
        fmt::to_writer(&io::cerr, ">>> {!RED}Invalid use of \"-dll\" argument. Expected a parameter.{!}\n");
        For(usage) fmt::to_writer(&io::cerr, it);
    }
    if (seekMemory) {
        fmt::to_writer(&io::cerr, ">>> {!RED}Invalid use of \"-memory\" argument. Expected a parameter.{!}\n");
        For(usage) fmt::to_writer(&io::cerr, it);
    }
    if (seekWidth) {
        fmt::to_writer(&io::cerr, ">>> {!RED}Invalid use of \"-width\" argument. Expected a parameter.{!}\n");
        For(usage) fmt::to_writer(&io::cerr, it);
    }
    if (seekHeight) {
        fmt::to_writer(&io::cerr, ">>> {!RED}Invalid use of \"-height\" argument. Expected a parameter.{!}\n");
        For(usage) fmt::to_writer(&io::cerr, it);
    }
    if (seekFPS) {
        fmt::to_writer(&io::cerr, ">>> {!RED}Invalid use of \"-fps\" argument. Expected a parameter.{!}\n");
        For(usage) fmt::to_writer(&io::cerr, it);
    }
}

s32 main() {
    parse_arguments();

    game_memory gameMemory;

    auto *allocData = new (Malloc) free_list_allocator_data;
    allocData->init(GameMemoryInMiB * 1024 * 1024, free_list_allocator_data::Find_First);
    gameMemory.AllocData = allocData;
    gameMemory.Alloc = GameAlloc = {free_list_allocator, allocData};

    // We tell imgui to use our allocator (by default it uses raw malloc, not operator new)
    ImGui::SetAllocatorFunctions([](size_t size, void *) { return operator new(size, GameAlloc); },
                                 [](void *ptr, void *) { delete ptr; });

    setup_game_paths();

    WITH_ALLOC(GameAlloc) {
        string windowTitle;
        fmt::sprint(&windowTitle, "Graphics Engine | {}", GameFileName);

        auto windowFlags =
            window::SHOWN | window::RESIZABLE | window::VSYNC | window::FOCUS_ON_SHOW | window::CLOSE_ON_ALT_F4;
        gameMemory.MainWindow =
            (new window)->init(windowTitle, window::DONT_CARE, window::DONT_CARE, GameWidth, GameHeight, windowFlags);

        gameMemory.MainWindow->Event.connect([](const event &e) {
            if (GameMainWindowEvent) return GameMainWindowEvent(e);
            return false;
        });

        graphics g;
        Graphics = &g;
        g.init(graphics_api::Direct3D);
        g.set_blend(true);
        g.set_depth_testing(false);

        init_imgui_for_our_windows(gameMemory.MainWindow);
        gameMemory.ImGuiContext = ImGui::GetCurrentContext();

        // Needs to get called at the end of execution to release any imgui windows
        defer(ImGui::DestroyPlatformWindows());

        imgui_renderer imguiRenderer;
        imguiRenderer.init(&g);

        // @TODO: GameFPS currently does nothing, we rely on g.swap() and vsync to hit target monitor refresh rate
        // but we should do that ourselves. I don't remember why we changed it (maybe simplicity?) but we should
        // definetely allow the user to set the target fps.

        // This is affecting any physics time steps though
        gameMemory.FrameDelta = 1.0f / GameFPS;

        while (true) {
            gameMemory.ReloadedThisFrame = check_for_dll_change();
            if (gameMemory.RequestReloadNextFrame) {
                gameMemory.ReloadedThisFrame = reload_game_code();
                gameMemory.RequestReloadNextFrame = false;
            }

            window::update();
            if (gameMemory.MainWindow->IsDestroying) break;

            imgui_for_our_windows_new_frame(gameMemory.MainWindow);
            ImGui::NewFrame();
            if (GameUpdateAndRender) GameUpdateAndRender(&gameMemory, &g);
            ImGui::Render();

            if (gameMemory.MainWindow->is_visible()) {
                g.set_target_window(gameMemory.MainWindow);
                g.set_cull_mode(cull::None);
                imguiRenderer.draw(ImGui::GetDrawData());
                g.swap();
            }

            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault(null, &imguiRenderer);
            }
        }
    }
    GameFileName.release();
}

#if OS == WINDOWS
#undef MAC
#undef _MAC
#include <Windows.h>  // For IME
#endif

static cursor MouseCursors[ImGuiMouseCursor_COUNT] = {
    cursor(OS_ARROW),     cursor(OS_IBEAM),       cursor(OS_RESIZE_ALL),  cursor(OS_RESIZE_NS),
    cursor(OS_RESIZE_WE), cursor(OS_RESIZE_NESW), cursor(OS_RESIZE_NWSE), cursor(OS_HAND)};

static bool MouseButtons[Mouse_Button_Last + 1]{};

static void update_modifiers() {
    ImGuiIO &io = ImGui::GetIO();
    io.KeyCtrl = io.KeysDown[Key_LeftControl] || io.KeysDown[Key_RightControl];
    io.KeyShift = io.KeysDown[Key_LeftShift] || io.KeysDown[Key_RightShift];
    io.KeyAlt = io.KeysDown[Key_LeftAlt] || io.KeysDown[Key_RightAlt];
    io.KeySuper = io.KeysDown[Key_LeftGUI] || io.KeysDown[Key_RightGUI];
}

static bool common_event_callback(const event &e) {
    ImGuiIO &io = ImGui::GetIO();
    if (e.Type == event::Keyboard_Pressed) {
        io.KeysDown[e.KeyCode] = true;
        update_modifiers();
    } else if (e.Type == event::Keyboard_Released) {
        io.KeysDown[e.KeyCode] = false;
        update_modifiers();
    } else if (e.Type == event::Code_Point_Typed) {
        ImGuiIO &io = ImGui::GetIO();
        io.AddInputCharacter(e.CP);
    } else if (e.Type == event::Mouse_Button_Pressed) {
        MouseButtons[e.Button] = true;
    } else if (e.Type == event::Mouse_Wheel_Scrolled) {
        io.MouseWheelH += e.ScrollX;
        io.MouseWheel += e.ScrollY;
    }
    return false;
}

static bool platform_event_callback(const event &e) {
    ImGuiIO &io = ImGui::GetIO();

    auto *viewport = ImGui::FindViewportByPlatformHandle(e.Window);
    if (e.Type == event::Window_Closed) {
        viewport->PlatformRequestClose = true;
    } else if (e.Type == event::Window_Moved) {
        viewport->PlatformRequestMove = true;
    } else if (e.Type == event::Window_Resized) {
        viewport->PlatformRequestResize = true;
    }
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
    if (HWND hWnd = (HWND) viewport->PlatformHandleRaw) {
        if (HIMC himc = ImmGetContext(hWnd)) {
            ImmSetCompositionWindow(himc, &cf);
            ImmReleaseContext(hWnd, himc);
        }
    }
}
#else
#define HAS_WIN32_IME 0
#endif

static void imgui_update_monitors() {
    ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();

    platformIO.Monitors.resize(0);
    For(os_get_monitors()) {
        vec2<s32> pos = os_get_monitor_pos(it);
        auto displayMode = os_get_current_display_mode(it);

        ImGuiPlatformMonitor monitor;
        monitor.MainPos = ImVec2((f32) pos.x, (f32) pos.y);
        monitor.MainSize = ImVec2((f32) displayMode.Width, (f32) displayMode.Height);

        rect workArea = os_get_work_area(it);
        monitor.WorkPos = ImVec2((f32) workArea.Top, (f32) workArea.Left);
        monitor.WorkSize = ImVec2((f32) workArea.width(), (f32) workArea.height());

        v2 scale = os_get_monitor_content_scale(it);
        monitor.DpiScale = scale.x;

        platformIO.Monitors.push_back(monitor);
    }
}

// Slightly modified version of "Photoshop" theme by @Derydoca (https://github.com/ocornut/imgui/issues/707)
static void imgui_init_photoshop_style() {
    ImGuiStyle *style = &ImGui::GetStyle();

    ImVec4 *colors = style->Colors;
    colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
    colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.000f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.118f, 0.118f, 0.118f, 1.000f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
    colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
    colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
    colors[ImGuiCol_DockingPreview] = ImVec4(1.000f, 0.391f, 0.000f, 0.781f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

    style->ChildRounding = 4.0f;
    style->FrameBorderSize = 1.0f;
    style->FrameRounding = 2.0f;
    style->GrabMinSize = 7.0f;
    style->PopupRounding = 2.0f;
    style->ScrollbarRounding = 12.0f;
    style->ScrollbarSize = 13.0f;
    style->TabBorderSize = 1.0f;
    style->TabRounding = 0.0f;
    style->WindowRounding = 4.0f;
}

static void init_imgui_for_our_windows(window *mainWindow) {
    ImGui::SetAllocatorFunctions([](size_t size, void *) { return operator new(size, Malloc); },
                                 [](void *ptr, void *) { delete ptr; });
    ImGui::CreateContext();

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

    // We tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui::StyleColorsDark();
    imgui_init_photoshop_style();

    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;
    io.BackendPlatformName = "lstd";

    io.SetClipboardTextFn = [](auto, const char *text) { os_set_clipboard_content(string(text)); };
    io.GetClipboardTextFn = [](auto) {
        static array<char> buffer;

        auto content = os_get_clipboard_content();
        buffer.reserve(content.ByteLength + 1);
        copy_memory(buffer.Data, content.Data, content.ByteLength);
        buffer[content.ByteLength] = 0;

        return (const char *) buffer.Data;
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

    mainWindow->Event.connect(common_event_callback);

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

            auto width = (s32) viewport->Size.x;
            auto height = (s32) viewport->Size.y;

            auto *win = (new window)->init("", window::DONT_CARE, window::DONT_CARE, width, height, flags);
            viewport->PlatformUserData = (void *) true;
            viewport->PlatformHandle = (void *) win;
#if OS == WINDOWS
            viewport->PlatformHandleRaw = win->PlatformData.Win32.hWnd;
#endif
            win->set_pos((s32) viewport->Pos.x, (s32) viewport->Pos.y);

            win->Event.connect(common_event_callback);
            win->Event.connect(platform_event_callback);
        };

        platformIO.Platform_DestroyWindow = [](auto *viewport) {
            if (viewport->PlatformUserData == (void *) true) {
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
            Graphics->set_target_window((window *) viewport->PlatformHandle);
        };

        platformIO.Platform_SwapBuffers = [](auto *viewport, auto) { Graphics->swap(); };

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

    vec2<s32> windowSize = mainWindow->get_size();
    vec2<s32> frameBufferSize = mainWindow->get_framebuffer_size();
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
                vec2<s32> mouse = win->get_cursor_pos();
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the
                    // mouse is on the upper-left of the primary monitor)
                    vec2<s32> windowPos = win->get_pos();
                    io.MousePos = ImVec2((f32)(mouse.x + windowPos.x), (f32)(mouse.y + windowPos.y));
                } else {
                    // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when
                    // the mouse is on the upper-left corner of the app window)
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

        if (win->Flags & window::MOUSE_PASS_THROUGH && !mousePassThrough) win->Flags ^= window::MOUSE_PASS_THROUGH;
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
