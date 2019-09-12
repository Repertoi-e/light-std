#if !defined LE_BUILDING_GAME
#error Error
#endif

#include "state.h"

static void framebuffer_resized(const window_framebuffer_resized_event &e) {
    if (!GameMemory->MainWindow->is_visible()) return;

    State->ViewportRenderTarget.release();
    State->ViewportRenderTarget.init_as_render_target(Graphics, "Docked Viewport Render Target", e.Width, e.Height);
}

void release_state() {
    State->ViewportRenderTarget.release();
    if (State->FBSizeCBID != npos) {
        GameMemory->MainWindow->WindowFramebufferResizedEvent.disconnect(State->FBSizeCBID);
    }
}

void reload_state() {
    release_state();

    assert(GameMemory->ImGuiContext);
    ImGui::SetCurrentContext((ImGuiContext *) GameMemory->ImGuiContext);
    ImGui::SetAllocatorFunctions([](size_t size, void *) { return operator new(size, Malloc); },
                                 [](void *ptr, void *) { delete ptr; });

    vec2i windowSize = GameMemory->MainWindow->get_size();
    framebuffer_resized({GameMemory->MainWindow, windowSize.x, windowSize.y});
    State->FBSizeCBID = GameMemory->MainWindow->WindowFramebufferResizedEvent.connect(framebuffer_resized);
}

void reload(game_memory *memory, graphics *g) {
    if (Context.Alloc.Function == Malloc.Function) {  // @Hack
        const_cast<implicit_context *>(&Context)->Alloc.Function = memory->ExeMalloc;
    }
    Malloc.Function = memory->ExeMalloc;

    GameMemory = memory;
    Graphics = g;

    if (!memory->Allocator) {
        auto *allocatorData = new (Malloc) free_list_allocator_data;
        allocatorData->init(128_MiB, free_list_allocator_data::Find_First);
        memory->Allocator = {free_list_allocator, allocatorData};
    }

    MANAGE_GLOBAL_STATE(State);

    reload_state();
}

LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *memory, graphics *g) {
    if (memory->ReloadedThisFrame) reload(memory, g);

    static camera_type oldCamera = State->CameraType;

    auto *win = GameMemory->MainWindow;
    if ((win->Keys[Key_LeftControl] || win->Keys[Key_RightControl]) && win->KeysThisFrame[Key_F]) {
        State->NoGUI = !State->NoGUI;
        // Ensure we use the FPS camera when we are not in the editor
        if (State->NoGUI) {
            oldCamera = State->CameraType;
            State->CameraType = camera_type::FPS;
        } else {
            State->CameraType = oldCamera;
        }

        if (State->MouseGrabbed) {
            State->MouseGrabbed = false;
            win->set_cursor_mode(window::CURSOR_NORMAL);
        }
    }

    // @TODO The viewport window may be in an additional imgui window, and we don't handle that yet!
    win = GameMemory->MainWindow;

    if (State->MouseGrabbed && ImGui::IsKeyPressed(Key_Escape, false)) {
        State->MouseGrabbed = false;
        win->set_cursor_mode(window::CURSOR_NORMAL);
    }

    if (State->NoGUI) {
        if (win->is_hovered() && win->MouseButtons[Mouse_Button_Left]) {
            State->MouseGrabbed = true;
            win->set_cursor_mode(window::CURSOR_DISABLED);
        }
    }

    //
    // Draw editor:
    //
    if (!State->NoGUI) {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("CDock Window", null,
                     ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                         ImGuiWindowFlags_NoBackground);
        ImGui::PopStyleVar(3);

        ImGuiID dockspaceID = ImGui::GetID("CDock");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f));

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Game")) {
                if (ImGui::MenuItem("VSync", "", GameMemory->MainWindow->Flags & window::VSYNC))
                    GameMemory->MainWindow->Flags ^= window::VSYNC;
                if (ImGui::MenuItem("No GUI", "Ctrl + F", State->NoGUI)) State->NoGUI = !State->NoGUI;
                ImGui::Separator();
                if (ImGui::MenuItem("Show overlay", "", State->Overlay)) State->Overlay = !State->Overlay;
                if (ImGui::MenuItem("Show imgui metrics", "", State->Metrics)) State->Metrics = !State->Metrics;
                ImGui::EndMenu();
            }
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("This is the editor view of the light-std game engine...");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            ImGui::EndMenuBar();
        }
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport", null, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);
        ImGui::PopStyleVar(3);

        auto windowPos = ImGui::GetWindowPos();
        auto windowSize = ImGui::GetWindowSize();
        {
            auto *drawList = ImGui::GetWindowDrawList();

            f32 viewportRatio = (f32) State->ViewportRenderTarget.Width / State->ViewportRenderTarget.Height;
            f32 windowRatio = windowSize.x / windowSize.y;

            vec2 renderableSize = windowSize;
            vec2 offset;
            if (viewportRatio < windowRatio) {
                renderableSize.x =
                    (State->ViewportRenderTarget.Width * (windowSize.y / State->ViewportRenderTarget.Height));
                offset = {(windowSize.x - renderableSize.x) / 2, 0};
            } else if (viewportRatio > windowRatio) {
                renderableSize.y =
                    (State->ViewportRenderTarget.Height * (windowSize.x / State->ViewportRenderTarget.Width));
                offset = {0, (windowSize.y - renderableSize.y) / 2};
            }
            offset += vec2(6 * viewportRatio, 6);
            renderableSize -= vec2(18 * viewportRatio, 18);

            drawList->AddImage(&State->ViewportRenderTarget, windowPos + offset, windowPos + offset + renderableSize);
            if (State->MouseGrabbed) {
                drawList->AddRect(windowPos + offset, windowPos + offset + renderableSize, 0xffffffff);
            }
        }

        if (State->CameraType == camera_type::FPS && ImGui::InvisibleButton("##viewport", windowSize)) {
            State->MouseGrabbed = true;
            win->set_cursor_mode(window::CURSOR_DISABLED);
        }

        if (State->Overlay) {
            if (State->OverlayCorner != -1) {
                ImVec2 pivot =
                    ImVec2((State->OverlayCorner & 1) ? 1.0f : 0.0f, (State->OverlayCorner & 2) ? 1.0f : 0.0f);
                ImGui::SetNextWindowPos(
                    ImVec2((State->OverlayCorner & 1) ? (windowPos.x + windowSize.x - 25) : (windowPos.x + 10),
                           (State->OverlayCorner & 2) ? (windowPos.y + windowSize.y - 10) : (windowPos.y + 25)),
                    ImGuiCond_Always, pivot);
            }

            ImGui::SetNextWindowBgAlpha(0.35f);
            if (ImGui::Begin("Overlay", &State->Overlay,
                             (State->OverlayCorner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDocking |
                                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                            ImGui::GetIO().Framerate);
                if (ImGui::BeginPopupContextWindow()) {
                    if (ImGui::MenuItem("Custom", null, State->OverlayCorner == -1)) State->OverlayCorner = -1;
                    if (ImGui::MenuItem("Top-left", null, State->OverlayCorner == 0)) State->OverlayCorner = 0;
                    if (ImGui::MenuItem("Top-right", null, State->OverlayCorner == 1)) State->OverlayCorner = 1;
                    if (ImGui::MenuItem("Bottom-left", null, State->OverlayCorner == 2)) State->OverlayCorner = 2;
                    if (ImGui::MenuItem("Bottom-right", null, State->OverlayCorner == 3)) State->OverlayCorner = 3;
                    if (State->Overlay && ImGui::MenuItem("Close")) State->Overlay = false;
                    ImGui::EndPopup();
                }
            }
            ImGui::End();
        }
        ImGui::End();

        if (State->Metrics) ImGui::ShowMetricsWindow(&State->Metrics);
    }

    update_and_render_scene();

    Context.TemporaryAlloc.free_all();
}
