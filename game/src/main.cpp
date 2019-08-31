#include "game.h"

#if defined LE_BUILDING_GAME
#error Error
#endif

#if OS == WINDOWS

#include <lstd/dx_graphics.h>
#include <lstd/file.h>
#include <lstd/graphics/ui/imgui.h>
#include <lstd/graphics/ui/imgui_impl.h>
#include <lstd/graphics/ui/imgui_impl_dx11.h>
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

s32 main() {
    setup_game_paths();

    game_memory gameMemory;
    gameMemory.MainWindow.init("Tetris", window::DONT_CARE, window::DONT_CARE, 1200, 600,
                               window::SHOWN | window::RESIZABLE | window::VSYNC | window::CLOSE_ON_ALT_F4);

    dx_graphics g;
    g.init();
    g.set_blend(true);
    g.set_depth_testing(false);
    g.add_target_window(&gameMemory.MainWindow);

    ImGui::CreateContext();
    gameMemory.ImGuiContext = ImGui::GetCurrentContext();

    ImGui::SetAllocatorFunctions(new_wrapper, delete_wrapper);

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts = new ImFontAtlas();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    // io.ConfigViewportsNoAutoMerge = true;
    io.ConfigViewportsNoTaskBarIcon = true;
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
    // May get handled by the statement above, but just to be sure!
    defer(g.remove_target_window(&gameMemory.MainWindow));

    // Setup Platform/Renderer bindings
    ImGui_Impl_Init(&gameMemory.MainWindow, &g);
    ImGui_ImplDX11_Init(g.D3DDevice, g.D3DDeviceContext);

    while (true) {
        gameMemory.ReloadedThisFrame = check_for_dll_change();

        window::update();
        if (gameMemory.MainWindow.IsDestroying) break;

        if (GameUpdateAndRender) GameUpdateAndRender(&gameMemory, &g);

        ImGui_ImplDX11_NewFrame();
        ImGui_Impl_NewFrame();
        ImGui::NewFrame();

        if (GameRenderUI) GameRenderUI();

        ImGui::Render();

        g.set_current_target_window(&gameMemory.MainWindow);
        g.set_cull_mode(cull::None);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g.swap();

        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
}

#endif
