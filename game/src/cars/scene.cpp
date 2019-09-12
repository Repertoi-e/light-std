#include "state.h"

struct scene_uniforms {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

struct object_uniforms {
    mat4 ModelMatrix;
};

struct scene {
    camera Camera;

    shader SceneShader;
    buffer SceneUB, ObjectUB;

    buffer CubeVB, CubeIB;
    buffer GridVB;
	f32 GridSpacing = 1.0f;

    scene_uniforms Uniforms;
    size_t FBSizeCBID = npos;

    scene() { Camera.reinit(); }  // Only runs once
};

static scene *Scene;

static void release_scene() {
    if (Scene->FBSizeCBID != npos) GameMemory->MainWindow->WindowFramebufferResizedEvent.disconnect(Scene->FBSizeCBID);
    Scene->~scene();
}

static void framebuffer_resized(const window_framebuffer_resized_event &e) {
    Scene->Uniforms.ProjectionMatrix = mat4::PERSPECTIVE(84, (f32) e.Width / e.Height, 0.01f, 1000.0f);
}

static void reload_scene() {
    MANAGE_GLOBAL_STATE(Scene);

    release_scene();

    auto *g = Graphics;
    Scene->SceneShader.init(g, "Scene Shader", file::path("data/Scene.hlsl"));
    Scene->SceneShader.bind();

    {
        struct Vertex {
            vec3 Position;
            vec4 Color;
        };
        Vertex cube[] = {{vec3(-1.0f, -1.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f)},
                         {vec3(1.0f, -1.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f)},
                         {vec3(1.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f)},
                         {vec3(-1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f)},
                         {vec3(-1.0f, -1.0f, -1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f)},
                         {vec3(1.0f, -1.0f, -1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f)},
                         {vec3(1.0f, 1.0f, -1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f)},
                         {vec3(-1.0f, 1.0f, -1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f)}};

        Scene->CubeVB.init(g, buffer_type::Vertex_Buffer, buffer_usage::Immutable, sizeof(cube), (const char *) cube);

        buffer_layout layout;
        layout.add("POSITION", gtype::F32_3);
        layout.add("COLOR", gtype::F32_4);
        Scene->CubeVB.set_input_layout(layout);

        u32 indices[] = {0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
                         4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};
        Scene->CubeIB.init(g, buffer_type::Index_Buffer, buffer_usage::Immutable, sizeof(indices),
                           (const char *) indices);

        Scene->SceneUB.init(g, buffer_type::Shader_Uniform_Buffer, buffer_usage::Dynamic, sizeof(scene_uniforms));
        Scene->ObjectUB.init(g, buffer_type::Shader_Uniform_Buffer, buffer_usage::Dynamic, sizeof(object_uniforms));
    }

    {
        vec2 min = {-100.0f, -100.0f}, max = {100.0f, 100.0f};
        f32 distance = 1.0f;

        array<vec3> lines;
        for (f32 x = min.x; x < max.x; x += distance) {
            lines.append(vec3(x, 0, min.y));
            lines.append(vec3(1, 1, 1));
            lines.append(vec3(x, 0, max.y));
            lines.append(vec3(1, 1, 1));
        }

        for (f32 z = min.y; z < max.y; z += distance) {
            lines.append(vec3(min.x, 0, z));
            lines.append(vec3(1, 1, 1));
            lines.append(vec3(max.x, 0, z));
            lines.append(vec3(1, 1, 1));
        }

        Scene->GridVB.init(g, buffer_type::Vertex_Buffer, buffer_usage::Dynamic, lines.Count * sizeof(vec3),
                           (const char *) lines.Data);

        buffer_layout layout;
        layout.add("POSITION", gtype::F32_3);
        layout.add("COLOR", gtype::F32_3);
        Scene->GridVB.set_input_layout(layout);
    }

    vec2i windowSize = GameMemory->MainWindow->get_size();
    framebuffer_resized({GameMemory->MainWindow, windowSize.x, windowSize.y});
    Scene->FBSizeCBID = GameMemory->MainWindow->WindowFramebufferResizedEvent.connect(framebuffer_resized);
}

void update_and_render_scene() {
    if (GameMemory->ReloadedThisFrame) reload_scene();

    auto *g = Graphics;
    auto *camera = &Scene->Camera;

    if (GameMemory->MainWindow->is_visible()) {
        camera->update();

        if (State->CameraType == camera_type::Maya) {
            Scene->Uniforms.ViewMatrix = mat4::TRANSLATE(vec3(0, 0, 1));
        } else if (State->CameraType == camera_type::FPS) {
            Scene->Uniforms.ViewMatrix = mat4::IDENTITY();
        }
        auto cameraOrientation = quat::ROTATION_Y(-camera->Yaw) * quat::ROTATION_X(-camera->Pitch);
        Scene->Uniforms.ViewMatrix *= mat4::ROTATE(cameraOrientation.conjugate()) * mat4::TRANSLATE(-camera->Position);

        g->set_target_window(GameMemory->MainWindow);

        if (State->Editor) {
            g->set_custom_render_target(&State->ViewportRenderTarget);
        }
        g->set_depth_testing(true);
        g->clear_color(State->ClearColor);

        Scene->SceneShader.bind();

        auto *sceneUB = Scene->SceneUB.map(buffer_map_access::Write_Discard_Previous);
        copy_memory(sceneUB, &Scene->Uniforms, sizeof(scene_uniforms));
        Scene->SceneUB.unmap();
        Scene->SceneUB.bind_ub(shader_type::Vertex_Shader, 0);

        // Grid
        {
            object_uniforms objectUniforms;
            objectUniforms.ModelMatrix = mat4::IDENTITY();

            auto *objectUB = Scene->ObjectUB.map(buffer_map_access::Write_Discard_Previous);
            copy_memory(objectUB, &objectUniforms, sizeof(object_uniforms));
            Scene->ObjectUB.unmap();
            Scene->ObjectUB.bind_ub(shader_type::Vertex_Shader, 1);

            Scene->GridVB.bind_vb(primitive_topology::LineList);
            g->draw((u32)(Scene->GridVB.Size / sizeof(vec2)));
        }

        // Cube
        {
            object_uniforms objectUniforms;
            objectUniforms.ModelMatrix = mat4::IDENTITY();

            auto *objectUB = Scene->ObjectUB.map(buffer_map_access::Write_Discard_Previous);
            copy_memory(objectUB, &objectUniforms, sizeof(object_uniforms));
            Scene->ObjectUB.unmap();
            Scene->ObjectUB.bind_ub(shader_type::Vertex_Shader, 1);

            Scene->CubeVB.bind_vb(primitive_topology::TriangleList);
            Scene->CubeIB.bind_ib();
            g->draw_indexed((u32)(Scene->CubeIB.Size / sizeof(u32)));
        }

        g->set_depth_testing(false);
        if (State->Editor) {
            g->set_custom_render_target(null);
        }
    }

    //
    // Draw editor:
    //
    if (State->Editor) {
        ImGui::Begin("Scene Properties", null);
        ImGui::Text("Camera");
        ImGui::BeginChild("##camera", {0, 180}, true);
        {
            if (ImGui::RadioButton("Maya", (s32 *) &State->CameraType, (s32) camera_type::Maya)) camera->reinit();
            ImGui::SameLine();
            if (ImGui::RadioButton("FPS", (s32 *) &State->CameraType, (s32) camera_type::FPS)) camera->reinit();

            ImGui::Text("Position: %.3f, %.3f, %.3f", camera->Position.x, camera->Position.y, camera->Position.z);
            ImGui::Text("Rotation: %.3f, %.3f, %.3f", camera->Rotation.x, camera->Rotation.y, camera->Rotation.z);
            ImGui::Text("Pitch: %.3f, yaw: %.3f", camera->Pitch, camera->Yaw);

            if (State->CameraType == camera_type::Maya) {
                ImGui::PushItemWidth(-140);
                ImGui::SliderFloat("Pan speed", &camera->PanSpeed, 0.0005f, 0.005f);
                ImGui::PushItemWidth(-140);
                ImGui::SliderFloat("Rotation speed", &camera->RotationSpeed, 0.0005f, 0.005f);
                ImGui::PushItemWidth(-140);
                ImGui::SliderFloat("Zoom speed", &camera->ZoomSpeed, 0.05f, 0.5f);
            } else if (State->CameraType == camera_type::FPS) {
                ImGui::PushItemWidth(-140);
                ImGui::SliderFloat("Speed", &camera->Speed, 0.01f, 10);
                ImGui::PushItemWidth(-140);
                ImGui::SliderFloat("Sprint speed", &camera->SprintSpeed, 0.01f, 10);
                ImGui::PushItemWidth(-140);
                ImGui::SliderFloat("Mouse sensitivity", &camera->MouseSensitivity, 0.0001f, 0.01f);
            }
            if (ImGui::Button("Default camera constants")) camera->reset_constants();

            ImGui::EndChild();

            ImGui::ColorPicker3("Clear color", &State->ClearColor.x);
        }
        ImGui::End();
    }
}
