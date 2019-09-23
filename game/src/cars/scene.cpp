#include "state.h"

static void release_scene() {
    if (Scene->FBSizeCBID != npos) GameMemory->MainWindow->WindowFramebufferResizedEvent.disconnect(Scene->FBSizeCBID);
    Scene->~scene();
}

static void framebuffer_resized(const window_framebuffer_resized_event &e) {
    Scene->Uniforms.ProjectionMatrix = mat4::PERSPECTIVE(84, (f32) e.Width / e.Height, 0.01f, 1000.0f);
}

// _p_ represents the center of the cuboid and _s_ is the radius in all axis, _c_ is a list of colors for each vertex
void generate_cuboid_model(model *m, vec3 p, vec3 s, vec4 c[8]) {
    m->FilePath = file::path("No path");

    vertex vertices[] = {{vec3(p.x - s.x, p.y - s.y, p.z + s.z), c[0]}, {vec3(p.x + s.x, p.y - s.y, p.z + s.z), c[1]},
                         {vec3(p.x + s.x, p.y + s.y, p.z + s.z), c[2]}, {vec3(p.x - s.x, p.y + s.y, p.z + s.z), c[3]},
                         {vec3(p.x - s.x, p.y - s.y, p.z - s.z), c[4]}, {vec3(p.x + s.x, p.y - s.y, p.z - s.z), c[5]},
                         {vec3(p.x + s.x, p.y + s.y, p.z - s.z), c[6]}, {vec3(p.x - s.x, p.y + s.y, p.z - s.z), c[7]}};

    m->VB.release();
    m->VB.init(Graphics, buffer_type::Vertex_Buffer, buffer_usage::Immutable, sizeof(vertices),
               (const char *) vertices);

    buffer_layout layout;
    layout.add("POSITION", gtype::F32_3);
    layout.add("COLOR", gtype::F32_4);
    m->VB.set_input_layout(layout);

    u32 indices[] = {0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
                     4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};

    m->IB.release();
    m->IB.init(Graphics, buffer_type::Index_Buffer, buffer_usage::Immutable, sizeof(indices), (const char *) indices);

    m->PrimitiveTopology = primitive_topology::TriangleList;
}

void generate_grid_model(model *m, vec2i gridSize, f32 gridSpacing) {
    m->FilePath = file::path("No path");

    vec2 min = {-(f32) gridSize.x * gridSpacing, -(f32) gridSize.y * gridSpacing};
    vec2 max = -min;

    size_t lineCount = (size_t)((max.x - min.x) / gridSpacing + (max.y - min.y) / gridSpacing);

    array<vertex> vertices;
    vertices.reserve(lineCount * 2);

    array<u32> indices;
    u32 index = 0;
    indices.reserve(lineCount * 2);

    for (f32 x = min.x; x <= max.x; x += gridSpacing) {
        vertices.append({vec3(x, 0, min.y), vec4(1, 1, 1, 1)});
        vertices.append({vec3(x, 0, max.y), vec4(1, 1, 1, 1)});
        indices.append(index++);
        indices.append(index++);
    }

    for (f32 z = min.y; z <= max.y; z += gridSpacing) {
        vertices.append({vec3(min.x, 0, z), vec4(1, 1, 1, 1)});
        vertices.append({vec3(max.x, 0, z), vec4(1, 1, 1, 1)});
        indices.append(index++);
        indices.append(index++);
    }

    m->VB.release();
    m->VB.init(Graphics, buffer_type::Vertex_Buffer, buffer_usage::Dynamic, vertices.Count * sizeof(vertex),
               (const char *) vertices.Data);

    buffer_layout layout;
    layout.add("POSITION", gtype::F32_3);
    layout.add("COLOR", gtype::F32_4);
    m->VB.set_input_layout(layout);

    m->IB.release();
    m->IB.init(Graphics, buffer_type::Index_Buffer, buffer_usage::Immutable, indices.Count * sizeof(u32),
               (const char *) indices.Data);

    m->PrimitiveTopology = primitive_topology::LineList;
}

static void reload_scene() {
    MANAGE_GLOBAL_STATE(Scene);
    MANAGE_GLOBAL_STATE(Models);

    release_scene();

    auto *g = Graphics;

    Scene->SceneUB.init(g, buffer_type::Shader_Uniform_Buffer, buffer_usage::Dynamic, sizeof(scene_uniforms));
    Scene->EntityUB.init(g, buffer_type::Shader_Uniform_Buffer, buffer_usage::Dynamic, sizeof(entity_uniforms));

    auto *sceneShader = Shaders->get("Scene Shader");
    if (!sceneShader->Graphics) {
        sceneShader->init(g, file::path("data/Scene.hlsl"));
    }

    //
    // Cuboid:
    //
    {
        auto *cuboid = Scene->Entities.append();
        cuboid->Mesh.Shader = sceneShader;

        cuboid->Mesh.Model = Models->get("Cuboid Model");
        if (!cuboid->Mesh.Model->VB.Graphics) {
            vec4 verticesColors[] = {
                vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f),
                vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f),
                vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f),
            };

            cuboid->Mesh.Shader->bind();
            generate_cuboid_model(cuboid->Mesh.Model, {0, 3, 0}, {4, 1, 5}, verticesColors);
        }
    }

    //
    // Grid:
    //
    {
        auto *grid = Scene->Entities.append();
        grid->Mesh.Shader = sceneShader;

        grid->Mesh.Model = Models->get("Grid Model");
        if (!grid->Mesh.Model->VB.Graphics) {
            grid->Mesh.Shader->bind();
            generate_grid_model(grid->Mesh.Model, Scene->GridSize, Scene->GridSpacing);
        }
    }

    vec2i windowSize = GameMemory->MainWindow->get_size();
    framebuffer_resized({GameMemory->MainWindow, windowSize.x, windowSize.y});
    Scene->FBSizeCBID = GameMemory->MainWindow->WindowFramebufferResizedEvent.connect(framebuffer_resized);
}

void update_and_render_scene() {
    if (GameMemory->ReloadedThisFrame) reload_scene();

    auto *g = Graphics;
    auto *cam = &Scene->Camera;

    if (GameMemory->MainWindow->is_visible()) {
        cam->update();

        // Move the grid to match the camera's position
        if (Scene->GridFollowCamera) {
            entity *grid = null;  // @Speed
            For_as(it_index, range(Scene->Entities.Count)) {
                auto *it = &Scene->Entities[it_index];
                if (it->Mesh.Model->Name == "Grid Model") {
                    grid = it;
                    break;
                }
            }
            assert(grid);

            f32 s = Scene->GridSpacing;
            grid->Position.x = (f32)(s64)(cam->Position.x / s) * s + s / 2;
            grid->Position.z = (f32)(s64)(cam->Position.z / s) * s + s / 2;
        }

        if (State->CameraType == camera_type::Maya) {
            Scene->Uniforms.ViewMatrix = mat4::TRANSLATE(vec3(0, 0, 1));
        } else if (State->CameraType == camera_type::FPS) {
            Scene->Uniforms.ViewMatrix = mat4::IDENTITY();
        }
        auto cameraOrientation = quat::ROTATION_Y(-cam->Yaw) * quat::ROTATION_X(-cam->Pitch);
        Scene->Uniforms.ViewMatrix *= mat4::ROTATE(cameraOrientation.conjugate()) * mat4::TRANSLATE(-cam->Position);

        g->set_target_window(GameMemory->MainWindow);

        if (State->Editor) g->set_custom_render_target(&State->ViewportRenderTarget);
        g->set_depth_testing(true);
        g->clear_color(State->ClearColor);

        auto *sceneUB = Scene->SceneUB.map(buffer_map_access::Write_Discard_Previous);
        copy_memory(sceneUB, &Scene->Uniforms, sizeof(scene_uniforms));
        Scene->SceneUB.unmap();
        Scene->SceneUB.bind_ub(shader_type::Vertex_Shader, 0);

        For(Scene->Entities) {
            if (it.Mesh.Shader && it.Mesh.Model) {
                it.Mesh.Shader->bind();

                entity_uniforms uniforms;
                uniforms.ModelMatrix = mat4::ROTATE(it.Orientation) * mat4::TRANSLATE(it.Position);

                auto *objectUB = Scene->EntityUB.map(buffer_map_access::Write_Discard_Previous);
                copy_memory(objectUB, &uniforms, sizeof(uniforms));
                Scene->EntityUB.unmap();
                Scene->EntityUB.bind_ub(shader_type::Vertex_Shader, 1);

                it.Mesh.Model->VB.bind_vb(it.Mesh.Model->PrimitiveTopology);
                it.Mesh.Model->IB.bind_ib();
                g->draw_indexed((u32)(it.Mesh.Model->IB.Size / sizeof(u32)));
            }
        }

        g->set_depth_testing(false);
        if (State->Editor) g->set_custom_render_target(null);
    }

    if (State->Editor) {
        editor_scene_properties(cam);
        editor_assets();
    }
}
