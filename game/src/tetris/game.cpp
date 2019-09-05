#if !defined LE_BUILDING_GAME
#error Error
#endif

#include <game.h>

#include <lstd/io/fmt.h>

static void *new_wrapper(size_t size, void *) { return operator new(size, Malloc); }
static void delete_wrapper(void *ptr, void *) { delete ptr; }

struct game_state {
    shader Shader;
    buffer VB, IB;

    vec4 ClearColor = {0.2f, 0.3f, 0.8f, 1.0f};
};

game_memory *g_GameMemory = null;

void reload(game_memory *memory, graphics *g) {
    g_GameMemory = memory;

    auto *state = (game_state *) memory->State;
    if (!state) {
        // The first time we load, initialize the allocator and game state
        auto *allocatorData = new (Malloc) free_list_allocator_data;
        allocatorData->init(512_MiB, free_list_allocator_data::Find_First);
        memory->Allocator = {free_list_allocator, allocatorData};

        memory->State = new (memory->Allocator) game_state;
        PUSH_CONTEXT(Alloc, memory->Allocator) reload(memory, g);
        return;
    }

    if (memory->ImGuiContext) {
        ImGui::SetCurrentContext((ImGuiContext *) memory->ImGuiContext);
        ImGui::SetAllocatorFunctions(new_wrapper, delete_wrapper);
    }

    state->Shader.release();
    state->VB.release();
    state->IB.release();

    state->Shader.init(g, "Triangle Shader", file::path("data/Triangle.hlsl"));
    state->Shader.bind();

    struct Vertex {
        vec3 Position;
        vec4 Color;
    };
    Vertex triangle[] = {{vec3(0.0f, 0.5f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f)},
                         {vec3(0.0f, -0.5, 0.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f)},
                         {vec3(-0.45f, -0.5f, 0.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f)}};

    state->VB.init(g, buffer_type::Vertex_Buffer, buffer_usage::Dynamic, sizeof(triangle));

    buffer_layout layout;
    layout.add("POSITION", gtype::F32_3);
    layout.add("COLOR", gtype::F32_4);
    state->VB.set_input_layout(layout);

    auto *data = state->VB.map(buffer_map_access::Write_Unsynchronized);
    copy_memory(data, triangle, sizeof(triangle));
    state->VB.unmap();

    u32 indices[] = {0, 1, 2};
    state->IB.init(g, buffer_type::Index_Buffer, buffer_usage::Immutable, sizeof(indices), (const char *) indices);
}

LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *memory, graphics *g) {
    PUSH_CONTEXT(Alloc, memory->Allocator) {
        if (memory->ReloadedThisFrame) reload(memory, g);

        auto *state = (game_state *) memory->State;

        state->Shader.bind();
        state->VB.bind_vb(primitive_topology::TriangleList);
        state->IB.bind_ib();

        g->set_target_window(memory->MainWindow);
        g->set_cull_mode(cull::Back);

        g->clear_color(state->ClearColor);
        g->draw_indexed((u32) (state->IB.Size / sizeof(u32)));

        Context.TemporaryAlloc.free_all();

        // The main window is swapped at the end of each frame in the main loop
    }
}

LE_GAME_API GAME_RENDER_UI(game_render_ui) {
    auto *state = (game_state *) g_GameMemory->State;

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
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f),
                     ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    bool show;
    ImGui::ShowDemoWindow(&show);
}
