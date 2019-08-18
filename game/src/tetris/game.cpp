#if !defined LE_BUILDING_GAME
#error Error
#endif

#include <lstd/dx_graphics.h>
#include <lstd/io/fmt.h>

#include <game.h>

struct game_state {
    g::shader *Shader;
    g::buffer *VB, *IB;
};

game_memory *g_GameMemory = null;

struct Vertex {
    vec3 Position;
    vec4 Color;
};

LE_GAME_API GAME_UPDATE_AND_RENDER(game_update_and_render, game_memory *memory, g::graphics *g) {
    if (memory->ReloadedThisFrame) {
        g_GameMemory = memory;

        auto *state = (game_state *) memory->State;
        if (!state) {
            // The first time we load, initialize the allocator and game state
            auto *allocatorData = new (Malloc) free_list_allocator_data;
            allocatorData->init(512_MiB, free_list_allocator_data::Find_First);
            memory->Allocator = {free_list_allocator, allocatorData};

            PUSH_CONTEXT(Alloc, g_GameMemory->Allocator) memory->State = state = new game_state;
        } else {
            delete state->Shader;
            delete state->VB;
            delete state->IB;
        }

        PUSH_CONTEXT(Alloc, g_GameMemory->Allocator) {
            state->Shader = new g::dx_shader;
            g->create_shader(state->Shader, file::path("data/Triangle.hlsl"));
            state->Shader->bind();

            Vertex triangle[] = {{vec3(0.0f, 0.5f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f)},
                                 {vec3(0.0f, -0.5, 0.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f)},
                                 {vec3(-0.45f, -0.5f, 0.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f)}};

            state->VB = new g::dx_buffer;
            g->create_buffer(state->VB, g::buffer::type::VERTEX_BUFFER, g::buffer::usage::DYNAMIC, sizeof(triangle));

            auto *layout = new g::buffer_layout;
            layout->add("POSITION", g::gtype::F32_3);
            layout->add("COLOR", g::gtype::F32_4);
            state->VB->set_input_layout(layout);

            auto *data = state->VB->map(g::buffer::map_access::WRITE);
            copy_memory(data, triangle, sizeof(triangle));
            state->VB->unmap();

            u32 indices[] = {0, 1, 2};
            state->IB = new g::dx_buffer;
            g->create_buffer(state->IB, g::buffer::type::INDEX_BUFFER, g::buffer::usage::IMMUTABLE,
                             (const char *) indices, sizeof(indices));
        }

        Context.init_temporary_allocator(1_MiB);
    }

    auto *state = (game_state *) memory->State;

    g->clear_color(vec4(0.2f, 0.3f, 0.8f, 1.0f));

    g::buffer::bind_data data;
    data.Topology = g::primitive_topology::TriangleList;
    state->VB->bind(data);

    state->IB->bind({});

    g->draw_indexed(state->IB->Size / sizeof(u32));

    Context.TemporaryAlloc.free_all();
}
