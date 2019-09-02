#if !defined LE_BUILDING_GAME
#error Error
#endif

#include <lstd/video/monitor.h>

#include <lstd/dx_graphics.h>
#include <lstd/graphics/ui/imgui.h>
#include <lstd/io/fmt.h>

#include <game.h>

static void *new_wrapper(size_t size, void *) { return operator new(size, Malloc); }
static void delete_wrapper(void *ptr, void *) { delete ptr; }

struct game_state {
    shader *Shader = null;
    buffer *VB = null, *IB = null;

    vec4 ClearColor = {0.2f, 0.3f, 0.8f, 1.0f};

    // window *TestWindow = null;

    bool ShowDemoWindow = true;
};

game_memory *g_GameMemory = null;

#define SAFE_DELETE(x) \
    if (x) delete x

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

    SAFE_DELETE(state->Shader);
    SAFE_DELETE(state->VB);
    SAFE_DELETE(state->IB);
    // SAFE_DELETE(state->TestWindow);

    state->Shader = new dx_shader;
    g->create_shader(state->Shader, "Triangle Shader", file::path("data/Triangle.hlsl"));
    state->Shader->bind();

    struct Vertex {
        vec3 Position;
        vec4 Color;
    };
    Vertex triangle[] = {{vec3(0.0f, 0.5f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f)},
                         {vec3(0.0f, -0.5, 0.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f)},
                         {vec3(-0.45f, -0.5f, 0.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f)}};

    state->VB = new dx_buffer;
    g->create_buffer(state->VB, buffer::type::VERTEX_BUFFER, buffer::usage::DYNAMIC, sizeof(triangle));

    buffer_layout layout;
    layout.add("POSITION", gtype::F32_3);
    layout.add("COLOR", gtype::F32_4);
    state->VB->set_input_layout(layout);

    auto *data = state->VB->map(buffer::map_access::WRITE_UNSYNCHRONIZED);
    copy_memory(data, triangle, sizeof(triangle));
    state->VB->unmap();

    u32 indices[] = {0, 1, 2};
    state->IB = new dx_buffer;
    g->create_buffer(state->IB, buffer::type::INDEX_BUFFER, buffer::usage::IMMUTABLE, (const char *) indices,
                     sizeof(indices));

    // state->TestWindow =
    //     (new window)
    //         ->init("Test window", window::DONT_CARE, window::DONT_CARE, 600, 600, window::RESIZABLE | window::SHOWN);
    // g->add_target_window(state->TestWindow);
}

LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *memory, graphics *g) {
    PUSH_CONTEXT(Alloc, memory->Allocator) {
        if (memory->ReloadedThisFrame) reload(memory, g);

        auto *state = (game_state *) memory->State;

        state->Shader->bind();

        buffer::bind_data data;
        data.Topology = primitive_topology::TriangleList;
        state->VB->bind(data);

        state->IB->bind({});

        // g->set_current_target_window(state->TestWindow);
        // g->clear_color(state->ClearColor);
        // g->draw_indexed(state->IB->Size / sizeof(u32));
        // g->swap();

        g->set_current_target_window(memory->MainWindow);
        g->set_cull_mode(cull::Back);

        g->clear_color(state->ClearColor);
        g->draw_indexed(state->IB->Size / sizeof(u32));

        // The main window is swapped at the end of each frame after we draw imgui stuff in the main loop

        Context.TemporaryAlloc.free_all();
    }
}

LE_GAME_API GAME_RENDER_UI(game_render_ui) {
    auto *state = (game_state *) g_GameMemory->State;
    if (state->ShowDemoWindow) ImGui::ShowDemoWindow(&state->ShowDemoWindow);
}
