#if !defined LE_BUILDING_GAME
#error Error
#endif

#include <game.h>

#include <lstd/io/fmt.h>

static void *new_wrapper(size_t size, void *) { return operator new(size, Malloc); }
static void delete_wrapper(void *ptr, void *) { delete ptr; }

struct game_state {
    shader TriShader, TexShader;
    buffer TriVB, TriIB, TexVB, TexIB;
    texture_2D ViewportTexture, TestTex;

    vec4 ClearColor = {0.2f, 0.3f, 0.8f, 1.0f};

    bool NoGUI = false;
};

game_memory *g_GameMemory = null;

void reload(game_memory *memory, graphics *g) {
    g_GameMemory = memory;

    auto *state = (game_state *) memory->State;
    if (!state) {
        // The first time we load, initialize the allocator and game state
        auto *allocatorData = new (Malloc) free_list_allocator_data;
        allocatorData->init(128_MiB, free_list_allocator_data::Find_First);
        memory->Allocator = {free_list_allocator, allocatorData};

        memory->State = new (memory->Allocator) game_state;
        PUSH_CONTEXT(Alloc, memory->Allocator) reload(memory, g);
        return;
    }

    if (memory->ImGuiContext) {
        ImGui::SetCurrentContext((ImGuiContext *) memory->ImGuiContext);
        ImGui::SetAllocatorFunctions(new_wrapper, delete_wrapper);
    }

    state->~game_state();  // @Hack to automatically release any members of the struct
    *((game_memory *) memory->State) = game_memory();

    state->TriShader.init(g, "Triangle Shader", file::path("data/Triangle.hlsl"));
    state->TriShader.bind();

    {
        struct Vertex {
            vec3 Position;
            vec4 Color;
        };
        Vertex triangle[] = {{vec3(0.0f, 0.5f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f)},
                             {vec3(0.0f, -0.5, 0.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f)},
                             {vec3(-0.45f, -0.5f, 0.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f)}};

        state->TriVB.init(g, buffer_type::Vertex_Buffer, buffer_usage::Dynamic, sizeof(triangle));

        buffer_layout layout;
        layout.add("POSITION", gtype::F32_3);
        layout.add("COLOR", gtype::F32_4);
        state->TriVB.set_input_layout(layout);

        auto *data = state->TriVB.map(buffer_map_access::Write_Unsynchronized);
        copy_memory(data, triangle, sizeof(triangle));
        state->TriVB.unmap();

        u32 indices[] = {0, 1, 2};
        state->TriIB.init(g, buffer_type::Index_Buffer, buffer_usage::Immutable, sizeof(indices),
                          (const char *) indices);
    }

    state->TexShader.init(g, "Basic Texture Shader", file::path("data/BasicTexture.hlsl"));
    state->TexShader.bind();

    {
        struct Vertex {
            vec3 Position;
            vec4 Color;
            vec2 UV;
        };
        Vertex triangle[] = {{vec3(-0.5f, 0.5f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), vec2(0, 0)},
                             {vec3(0.5f, 0.5f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), vec2(1, 0)},
                             {vec3(0.5f, -0.5f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), vec2(1, 1)},
                             {vec3(-0.5f, -0.5f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), vec2(0, 1)}};

        state->TexVB.init(g, buffer_type::Vertex_Buffer, buffer_usage::Dynamic, sizeof(triangle));

        buffer_layout layout;
        layout.add("POSITION", gtype::F32_3);
        layout.add("COLOR", gtype::F32_4);
        layout.add("TEXCOORD", gtype::F32_2);
        state->TexVB.set_input_layout(layout);

        auto *data = state->TexVB.map(buffer_map_access::Write_Unsynchronized);
        copy_memory(data, triangle, sizeof(triangle));
        state->TexVB.unmap();

        u32 indices[] = {0, 1, 2, 2, 3, 0};
        state->TexIB.init(g, buffer_type::Index_Buffer, buffer_usage::Immutable, sizeof(indices),
                          (const char *) indices);
    }

    state->ViewportTexture.init_as_render_target(g, "Docked Viewport Render Target", 1600, 900);

    auto TestTexData = pixel_buffer(file::path("data/chocolate-pancake.bmp"));
    state->TestTex.init(g, "Test Image", TestTexData.Width, TestTexData.Height);
    state->TestTex.set_data(TestTexData);
}

LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *memory, graphics *g) {
    PUSH_CONTEXT(Alloc, memory->Allocator) {
        if (memory->ReloadedThisFrame) reload(memory, g);

        auto *state = (game_state *) memory->State;

        g->set_target_window(memory->MainWindow);

        if (!state->NoGUI) {
            g->set_custom_render_target(&state->ViewportTexture);
        }

        state->TriShader.bind();
        state->TriVB.bind_vb(primitive_topology::TriangleList);
        state->TriIB.bind_ib();

        g->clear_color(state->ClearColor);
        g->draw_indexed((u32)(state->TriIB.Size / sizeof(u32)));

        state->TexShader.bind();
        state->TestTex.bind(0);
        state->TexVB.bind_vb(primitive_topology::TriangleList);
        state->TexIB.bind_ib();
        g->draw_indexed((u32)(state->TexIB.Size / sizeof(u32)));
        state->TestTex.unbind();

        if (!state->NoGUI) {
            g->set_custom_render_target(null);
        }

        Context.TemporaryAlloc.free_all();
    }
}

LE_GAME_API GAME_RENDER_UI(game_render_ui) {
    auto *state = (game_state *) g_GameMemory->State;

    if ((ImGui::IsKeyDown(Key_LeftControl) || ImGui::IsKeyDown(Key_RightControl)) &&
        ImGui::IsKeyPressed(Key_F, false)) {
        state->NoGUI = !state->NoGUI;
    }

    if (!state->NoGUI) {
        //
        // Construct central docking location
        //
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
                if (ImGui::MenuItem("VSync", "", g_GameMemory->MainWindow->Flags & window::VSYNC))
                    g_GameMemory->MainWindow->Flags ^= window::VSYNC;
                if (ImGui::MenuItem("No GUI", "Ctrl + F", state->NoGUI)) state->NoGUI = !state->NoGUI;
                ImGui::EndMenu();
            }
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("This is the editor view of the light-std game engine.");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        ImGui::Begin("Scene Properties", null);
        { ImGui::ColorPicker3("Clear color", &state->ClearColor.x); }
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport", null);
        ImGui::PopStyleVar(3);
        {
            auto *drawList = ImGui::GetWindowDrawList();

            auto windowPos = ImGui::GetWindowPos();
            auto windowSize = ImGui::GetWindowSize();

            f32 viewportRatio = (f32) state->ViewportTexture.Width / state->ViewportTexture.Height;
            f32 windowRatio = windowSize.x / windowSize.y;

            vec2 renderableSize = windowSize, offset;
            if (viewportRatio < windowRatio) {
                renderableSize.x = (state->ViewportTexture.Width * (windowSize.y / state->ViewportTexture.Height));
                offset = {(windowSize.x - renderableSize.x) / 2, 0};
            } else if (viewportRatio > windowRatio) {
                renderableSize.y = (state->ViewportTexture.Height * (windowSize.x / state->ViewportTexture.Width));
                offset = {0, (windowSize.y - renderableSize.y) / 2};
            }
            drawList->AddImage(&state->ViewportTexture, windowPos + offset, windowPos + offset + renderableSize);
        }
        ImGui::End();

        ImGui::ShowMetricsWindow();
    }
}
