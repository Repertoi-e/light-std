// #include <dxmath/Inc/DirectXMath.h>
// using namespace DirectX;

#include "state.h"

entity *new_entity() {
    auto *result = Scene->Entities.append();
    result->Active = true;
    return result;
}

static void release_scene() {
    if (Scene->FBSizeCBID != npos) GameMemory->MainWindow->WindowFramebufferResizedEvent.disconnect(Scene->FBSizeCBID);
    Scene->~scene();
}

static void framebuffer_resized(const window_framebuffer_resized_event &e) {
    Scene->Uniforms.ProjectionMatrix = perspective(45 * TAU / 360, (f32) e.Width / e.Height, 0.1f, 1000.0f);

    if (Graphics->API == graphics_api::Direct3D) {
        Scene->Uniforms.ProjectionMatrix = T(Scene->Uniforms.ProjectionMatrix);
    }
}

// _p_ represents the center of the cuboid and _s_ is the radius in all axis, _c_ is a list of colors for each vertex
void generate_cuboid_model(model *m, v3 p, v3 s, v4 c[8]) {
    m->FilePath = file::path("No path");

    vertex vertices[] = {{v3(p.x - s.x, p.y - s.y, p.z + s.z), c[0]}, {v3(p.x + s.x, p.y - s.y, p.z + s.z), c[1]},
                         {v3(p.x + s.x, p.y + s.y, p.z + s.z), c[2]}, {v3(p.x - s.x, p.y + s.y, p.z + s.z), c[3]},
                         {v3(p.x - s.x, p.y - s.y, p.z - s.z), c[4]}, {v3(p.x + s.x, p.y - s.y, p.z - s.z), c[5]},
                         {v3(p.x + s.x, p.y + s.y, p.z - s.z), c[6]}, {v3(p.x - s.x, p.y + s.y, p.z - s.z), c[7]}};

    m->VB.release();
    m->VB.init(Graphics, buffer_type::Vertex_Buffer, buffer_usage::Immutable, sizeof(vertices),
               (const char *) vertices);

    // @Volatile :vertex
    buffer_layout layout;
    layout.add("POSITION", gtype::F32_3);
    layout.add_padding(sizeof(f32));
    layout.add("COLOR", gtype::F32_4);
    m->VB.set_input_layout(layout);

    u32 indices[] = {0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
                     4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};

    m->IB.release();
    m->IB.init(Graphics, buffer_type::Index_Buffer, buffer_usage::Immutable, sizeof(indices), (const char *) indices);

    m->PrimitiveTopology = primitive_topology::TriangleList;
}

void generate_grid_model(model *m, vec2<s32> gridSize, f32 gridSpacing) {
    m->FilePath = file::path("No path");

    v2 min = {-(f32) gridSize.x * gridSpacing, -(f32) gridSize.y * gridSpacing};
    v2 max = -min;

    size_t lineCount = (size_t)((max.x - min.x) / gridSpacing + (max.y - min.y) / gridSpacing);

    array<vertex> vertices;
    vertices.reserve(lineCount * 2);

    array<u32> indices;
    u32 index = 0;
    indices.reserve(lineCount * 2);

    for (f32 x = min.x; x <= max.x; x += gridSpacing) {
        vertices.append({v3(x, 0, min.y), v4(1, 1, 1, 1)});
        vertices.append({v3(x, 0, max.y), v4(1, 1, 1, 1)});
        indices.append(index++);
        indices.append(index++);
    }

    for (f32 z = min.y; z <= max.y; z += gridSpacing) {
        vertices.append({v3(min.x, 0, z), v4(1, 1, 1, 1)});
        vertices.append({v3(max.x, 0, z), v4(1, 1, 1, 1)});
        indices.append(index++);
        indices.append(index++);
    }

    m->VB.release();
    m->VB.init(Graphics, buffer_type::Vertex_Buffer, buffer_usage::Dynamic, vertices.Count * sizeof(vertex),
               (const char *) vertices.Data);

    // @Volatile :vertex
    buffer_layout layout;
    layout.add("POSITION", gtype::F32_3);
    layout.add_padding(sizeof(f32));
    layout.add("COLOR", gtype::F32_4);
    m->VB.set_input_layout(layout);

    m->IB.release();
    m->IB.init(Graphics, buffer_type::Index_Buffer, buffer_usage::Immutable, indices.Count * sizeof(u32),
               (const char *) indices.Data);

    m->PrimitiveTopology = primitive_topology::LineList;
}

void reload_scene() {
    release_scene();

    Scene->Entities.reserve(0, 16);  // Allocate aligned entities

    auto *g = Graphics;

    Scene->SceneUB.init(g, buffer_type::Shader_Uniform_Buffer, buffer_usage::Dynamic, sizeof(scene_uniforms));
    Scene->EntityUB.init(g, buffer_type::Shader_Uniform_Buffer, buffer_usage::Dynamic, sizeof(entity_uniforms));

    AssetCatalog->load(
        {file::path("Scene.hlsl")},
        [&](array<file::path> f) { Shaders->get_or_create("Scene Shader")->init(g, file::handle(f[0])); }, true);

    auto *sceneShader = Shaders->get_or_create("Scene Shader");

    //
    // Cuboid:
    //
    {
        auto *cuboid = new_entity();
        cuboid->Mesh.Shader = sceneShader;

        cuboid->Mesh.Model = Models->get_or_create("Cuboid Model");
        v4 verticesColors[] = {
            v4(1.0f, 0.0f, 0.0f, 1.0f), v4(0.0f, 1.0f, 0.0f, 1.0f), v4(0.0f, 0.0f, 1.0f, 1.0f),
            v4(1.0f, 1.0f, 1.0f, 1.0f), v4(1.0f, 0.0f, 0.0f, 1.0f), v4(0.0f, 1.0f, 0.0f, 1.0f),
            v4(0.0f, 0.0f, 1.0f, 1.0f), v4(1.0f, 1.0f, 1.0f, 1.0f),
        };

        cuboid->Mesh.Shader->bind();
        generate_cuboid_model(cuboid->Mesh.Model, {0, 3, 0}, {4, 1, 5}, verticesColors);
    }

    //
    // Grid:
    //
    {
        auto *grid = Scene->Grid = new_entity();
        grid->Mesh.Shader = sceneShader;

        grid->Mesh.Model = Models->get_or_create("Grid Model");
        grid->Mesh.Shader->bind();
        generate_grid_model(grid->Mesh.Model, Scene->GridSize, Scene->GridSpacing);
    }

    //
    // Textured quad
    //
    /*
    AssetCatalog->load(
        {file::path("BasicTexture.hlsl")},
        [&](array<file::path> f) { Shaders->get_or_create("Basic Texture Shader")->init(g, file::handle(f[0])); },
        true);

    auto *texShader = Shaders->get_or_create("Basic Texture Shader");
    auto *tquad = Scene->Entities.append();
    tquad->Position = zero();
    tquad->Orientation = identity();
    tquad->Mesh.Shader = texShader;

    tquad->Mesh.Model = Models->get_or_create("Grid Model");
    tquad->Mesh.Shader->bind();

    {
        struct textured_vertex {
            v3 Position;
            v4 Color;
            v2 UV;
        };
        textured_vertex vertices[] = {{v3(-0.5f, 0.5f, 0.0f), v4(1.0f, 1.0f, 1.0f, 1.0f), v2(0, 0)},
                                      {v3(0.5f, 0.5f, 0.0f), v4(1.0f, 1.0f, 1.0f, 1.0f), v2(1, 0)},
                                      {v3(0.5f, -0.5f, 0.0f), v4(1.0f, 1.0f, 1.0f, 1.0f), v2(1, 1)},
                                      {v3(-0.5f, -0.5f, 0.0f), v4(1.0f, 1.0f, 1.0f, 1.0f), v2(0, 1)}};

        auto *model = tquad->Mesh.Model;

        model->VB.release();
        model->VB.init(g, buffer_type::Vertex_Buffer, buffer_usage::Dynamic, sizeof(vertices));

        buffer_layout layout;
        layout.add("POSITION", gtype::F32_3);
        layout.add_padding(sizeof(f32));
        layout.add("COLOR", gtype::F32_4);
        layout.add("TEXCOORD", gtype::F32_2);
        layout.add_padding(sizeof(f32) * 2);
        model->VB.set_input_layout(layout);

        auto *data = model->VB.map(buffer_map_access::Write_Unsynchronized);
        copy_memory(data, vertices, sizeof(vertices));
        model->VB.unmap();

        u32 indices[] = {0, 1, 2, 2, 3, 0};

        model->IB.release();
        model->IB.init(g, buffer_type::Index_Buffer, buffer_usage::Immutable, sizeof(indices), (const char *)
    indices);

        model->PrimitiveTopology = primitive_topology::TriangleList;
    }

    auto *tex = Texture2Ds->get_or_create("Cake Texture");

    AssetCatalog->load(
        {file::path("chocolate-pancake.bmp")},
        [&](array<file::path> f) {
            auto texData = pixel_buffer(f[0]);
            tex->init(g, texData.Width, texData.Height);
            tex->set_data(texData);
        },
        true);
    */

    vec2<s32> windowSize = GameMemory->MainWindow->get_size();
    framebuffer_resized({GameMemory->MainWindow, windowSize.x, windowSize.y});
    Scene->FBSizeCBID = GameMemory->MainWindow->WindowFramebufferResizedEvent.connect(framebuffer_resized);
}

void update_and_render_scene() {
    auto *g = Graphics;
    auto *cam = &Scene->Camera;

    if (GameMemory->MainWindow->is_visible()) {
        cam->update();

        // Move the grid to match the camera's position
        if (Scene->GridFollowCamera) {
            entity *grid = Scene->Grid;
            assert(grid);

            f32 s = Scene->GridSpacing;
            grid->Position.x = (f32)(s64)(cam->Position.x / s) * s + s / 2;
            grid->Position.z = (f32)(s64)(cam->Position.z / s) * s + s / 2;
        }

        g->set_target_window(GameMemory->MainWindow);

        if (GameState->Editor) g->set_custom_render_target(&GameState->ViewportRenderTarget);
        g->set_depth_testing(true);

        // @Bug: This culls the front of the cuboid (the indices are most probably wrong)
        // g->set_cull_mode(cull::Back);

        g->clear_color(GameState->ClearColor);

        {
            auto *su = &Scene->Uniforms;
            su->ViewMatrix = (m44) look_at(cam->Position, cam->FocalPoint, v3(0, 1, 0), false, false, false);
            su->ViewMatrix = dot((m44) translation(0, 0, 1), su->ViewMatrix);

            if (g->API == graphics_api::Direct3D) su->ViewMatrix = T(su->ViewMatrix);

            auto *sceneUB = Scene->SceneUB.map(buffer_map_access::Write_Discard_Previous);
            copy_memory(sceneUB, &Scene->Uniforms, sizeof(scene_uniforms));
            Scene->SceneUB.unmap();
            Scene->SceneUB.bind_ub(shader_type::Vertex_Shader, 0);
        }

        For(Scene->Entities) {
            if (!it.Active) continue;
            if (!it.Mesh.Shader || !it.Mesh.Model) continue;

            it.Mesh.Shader->bind();

            entity_uniforms uniforms;
            uniforms.ModelMatrix = translation(it.Position);

            if (g->API == graphics_api::Direct3D) uniforms.ModelMatrix = T(uniforms.ModelMatrix);

            auto *objectUB = Scene->EntityUB.map(buffer_map_access::Write_Discard_Previous);
            copy_memory(objectUB, &uniforms, sizeof(uniforms));
            Scene->EntityUB.unmap();
            Scene->EntityUB.bind_ub(shader_type::Vertex_Shader, 1);

            it.Mesh.Model->VB.bind_vb(it.Mesh.Model->PrimitiveTopology);
            it.Mesh.Model->IB.bind_ib();
            g->draw_indexed((u32)(it.Mesh.Model->IB.Size / sizeof(u32)));
        }

        g->set_depth_testing(false);
        if (GameState->Editor) g->set_custom_render_target(null);
    }

    if (GameState->Editor) {
        editor_scene_properties(cam);
        editor_assets();
    }
}
