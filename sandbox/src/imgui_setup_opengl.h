#pragma once

#include "types.h"

#include <imgui/imgui.h>

struct ImGuiOpenGLState {
    u32 FontTexture = 0;
    u32 VboHandle, IboHandle = 0;
    u32 ShaderHandle = 0, VertexHandle = 0, FragmentHandle = 0;
    s32 AttribLocationTexture = 0, AttribLocationProjMatrix = 0, AttribLocationPosition = 0, AttribLocationUV = 0, AttribLocationColor = 0;
};

void imgui_create_opengl_device_objects(ImGuiOpenGLState *state);
void imgui_destroy_opengl_device_objects(ImGuiOpenGLState *state);
void imgui_render_data_with_opengl(ImGuiOpenGLState *state, ImDrawData *drawData);
