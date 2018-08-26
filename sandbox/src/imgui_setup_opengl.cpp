#include "imgui_setup_opengl.h"

#include <GL/glew.h>
#include <SDL_log.h>

static void check_shader(u32 handle, char const *description) {
    s32 status = 0, logLength = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);

    if (status == GL_FALSE) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "ERROR: imgui_create_opengl_device_objects: failed to compile %s!\n", description);
    }
    if (logLength > 0) {
        ImVector<char> buffer;
        buffer.resize((s32) (logLength + 1));
        glGetShaderInfoLog(handle, logLength, 0, (char *) buffer.begin());
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "    %s\n", buffer.begin());
    }
}

static void check_program(u32 handle, char const *description) {
    GLint status = 0, logLength = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);

    if (status == GL_FALSE) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "ERROR: imgui_create_opengl_device_objects: failed to link %s!\n", description);
    }
    if (logLength > 0) {
        ImVector<char> buffer;
        buffer.resize((s32) (logLength + 1));
        glGetProgramInfoLog(handle, logLength, 0, (char *) buffer.begin());
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "    %s\n", buffer.begin());
    }
}

void imgui_create_opengl_device_objects(ImGuiOpenGLState *state) {
    s32 lastTexture, lastVAO, lastVBO;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastVAO);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVBO);

    char const *vertexShaderSource = R"(
        #version 130

        in vec2 Position;
        in vec2 UV;
        in vec4 Color;

        out vec2 Frag_UV;
        out vec4 Frag_Color;

        uniform mat4 ProjMatrix;

        void main()
        {
            Frag_UV = UV;
            Frag_Color = Color;
            gl_Position = ProjMatrix * vec4(Position.xy, 0, 1);
        })";

    char const *fragmentShaderSource = R"(
        #version 130
     
        in vec2 Frag_UV;
        in vec4 Frag_Color;

        out vec4 Out_Color;

        uniform sampler2D Texture;

        void main()
        {
            Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
        })";

    // Create shaders
    state->VertexHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(state->VertexHandle, 1, &vertexShaderSource, 0);
    glCompileShader(state->VertexHandle);
    check_shader(state->VertexHandle, "vertex shader");

    state->FragmentHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(state->FragmentHandle, 1, &fragmentShaderSource, 0);
    glCompileShader(state->FragmentHandle);
    check_shader(state->FragmentHandle, "fragment shader");

    state->ShaderHandle = glCreateProgram();
    glAttachShader(state->ShaderHandle, state->VertexHandle);
    glAttachShader(state->ShaderHandle, state->FragmentHandle);
    glLinkProgram(state->ShaderHandle);
    check_program(state->ShaderHandle, "shader program");

    state->AttribLocationTexture     = glGetUniformLocation(state->ShaderHandle, "Texture");
    state->AttribLocationProjMatrix  = glGetUniformLocation(state->ShaderHandle, "ProjMatrix");

    state->AttribLocationPosition = glGetAttribLocation(state->ShaderHandle, "Position");
    state->AttribLocationUV       = glGetAttribLocation(state->ShaderHandle, "UV");
    state->AttribLocationColor    = glGetAttribLocation(state->ShaderHandle, "Color");

    // Create buffers
    glGenBuffers(1, &state->VboHandle);
    glGenBuffers(1, &state->IboHandle);

    ImGuiIO &io = ImGui::GetIO();
    u8 *pixels;
    int width, height;
    // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) 
    // because it is more likely to be compatible with user's existing shaders. 
    // If your ImTextureId represent a higher-level concept than just a GL texture id, 
    // consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Upload texture to graphics system
    glGenTextures(1, &state->FontTexture);
    glBindTexture(GL_TEXTURE_2D, state->FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    io.Fonts->TexID = (void *) (ptr) state->FontTexture;

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glBindBuffer(GL_ARRAY_BUFFER, lastVAO);
    glBindVertexArray(lastVBO);
}

void imgui_destroy_opengl_device_objects(ImGuiOpenGLState *state) {
    if (state->VboHandle) {
        glDeleteBuffers(1, &state->VboHandle);
        state->VboHandle = 0;
    }
    if (state->IboHandle) {
        glDeleteBuffers(1, &state->IboHandle);
        state->IboHandle = 0;
    }

    if (state->ShaderHandle && state->VertexHandle) {
        glDetachShader(state->ShaderHandle, state->VertexHandle);
    }
    if (state->VertexHandle) {
        glDeleteShader(state->VertexHandle);
        state->VertexHandle = 0;
    }

    if (state->ShaderHandle && state->FragmentHandle) {
        glDetachShader(state->ShaderHandle, state->FragmentHandle);
    }
    if (state->FragmentHandle) {
        glDeleteShader(state->FragmentHandle);
        state->FragmentHandle = 0;
    }

    if (state->ShaderHandle) {
        glDeleteProgram(state->ShaderHandle);
        state->ShaderHandle = 0;
    }

    if (state->FontTexture) {
        ImGuiIO &io = ImGui::GetIO();
        io.Fonts->TexID = 0;

        glDeleteTextures(1, &state->FontTexture);
        state->FontTexture = 0;
    }
}

void imgui_render_data_with_opengl(ImGuiOpenGLState *state, ImDrawData *drawData) {
    ImGuiIO &io = ImGui::GetIO();
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    s32 framebufferWidth  = (s32) (drawData->DisplaySize.x * io.DisplayFramebufferScale.x);
    s32 framebufferHeight = (s32) (drawData->DisplaySize.y * io.DisplayFramebufferScale.y);
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
        return;

    glActiveTexture(GL_TEXTURE0);

    // Backup GL state
    u32 lastActiveTexture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, (s32 *) &lastActiveTexture);

    s32 lastProgram, lastTexture, lastSampler, lastVAO, lastVBO;
    glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
    glGetIntegerv(GL_SAMPLER_BINDING, &lastSampler);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastVAO);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVBO);

    s32 lastPolygonMode[2], lastViewport[4], lastScissorBox[4];
    glGetIntegerv(GL_POLYGON_MODE, lastPolygonMode);
    glGetIntegerv(GL_VIEWPORT, lastViewport);
    glGetIntegerv(GL_SCISSOR_BOX, lastScissorBox);

    u32 lastBlendSourceRGB, lastBlendDestRGB, lastBlendSourceAlpha, lastBlendDestAlpha, lastBlendEquationRGB, lastBlendEquationAlpha;
    glGetIntegerv(GL_BLEND_SRC_RGB, (s32 *) &lastBlendSourceRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, (s32 *) &lastBlendDestRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (s32 *) &lastBlendSourceAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, (s32 *) &lastBlendDestAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (s32 *) &lastBlendEquationRGB);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (s32 *) &lastBlendEquationAlpha);

    u8 lastEnableBlend       = glIsEnabled(GL_BLEND);
    u8 lastEnableCullFace    = glIsEnabled(GL_CULL_FACE);
    u8 lastEnableDepthTest   = glIsEnabled(GL_DEPTH_TEST);
    u8 lastEnableScissorTest = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    {
        f32 L = drawData->DisplayPos.x;
        f32 R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        f32 T = drawData->DisplayPos.y;
        f32 B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        const f32 orthoProjection[4][4] =
        {
            { 2.0f/(R-L),  0.0f,         0.0f, 0.0f },
            { 0.0f,        2.0f/(T-B),   0.0f, 0.0f },
            { 0.0f,        0.0f,        -1.0f, 0.0f },
            { (R+L)/(L-R), (T+B)/(B-T),  0.0f, 1.0f },
        };
        glUseProgram(state->ShaderHandle);
        glUniform1i(state->AttribLocationTexture, 0);
        glUniformMatrix4fv(state->AttribLocationProjMatrix, 1, GL_FALSE, &orthoProjection[0][0]);
    }

    if (glBindSampler) {
        glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
    }

    // Recreate the VAO every time 
    // (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, 
    //  and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them)
    u32 vaoHandle = 0;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);
    glBindBuffer(GL_ARRAY_BUFFER, state->VboHandle);
    glEnableVertexAttribArray(state->AttribLocationPosition);
    glEnableVertexAttribArray(state->AttribLocationUV);
    glEnableVertexAttribArray(state->AttribLocationColor);
    glVertexAttribPointer(state->AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void *) offsetof(ImDrawVert, pos));
    glVertexAttribPointer(state->AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void *) offsetof(ImDrawVert, uv));
    glVertexAttribPointer(state->AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (void *) offsetof(ImDrawVert, col));

    // Draw
    ImVec2 position = drawData->DisplayPos;
    for (s32 i = 0; i < drawData->CmdListsCount; i++) {
        ImDrawList *commandList = drawData->CmdLists[i];
        ImDrawIdx *indexBufferOffset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, state->VboHandle);
        glBufferData(GL_ARRAY_BUFFER, (ptr) commandList->VtxBuffer.Size * sizeof(ImDrawVert), (void *) commandList->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->IboHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (ptr) commandList->IdxBuffer.Size * sizeof(ImDrawIdx), (void *) commandList->IdxBuffer.Data, GL_STREAM_DRAW);

        for (s32 j = 0; j < commandList->CmdBuffer.Size; j++) {
            ImDrawCmd *command = &commandList->CmdBuffer[j];
            if (command->UserCallback) {
                // User callback (registered via ImDrawList::AddCallback)
                command->UserCallback(commandList, command);
            } else {
                ImVec4 clipRect = ImVec4(command->ClipRect.x - position.x, command->ClipRect.y - position.y,
                    command->ClipRect.z - position.x, command->ClipRect.w - position.y);
                if (clipRect.x < framebufferWidth && clipRect.y < framebufferHeight && clipRect.z >= 0.0f && clipRect.w >= 0.0f) {
                    // Apply scissor/clipping rectangle
                    glScissor((s32) clipRect.x, (s32) (framebufferHeight - clipRect.w),
                        (s32) (clipRect.z - clipRect.x), (s32) (clipRect.w - clipRect.y));

                    // Bind texture, Draw
                    glBindTexture(GL_TEXTURE_2D, (u32) (ptr) command->TextureId);
                    glDrawElements(GL_TRIANGLES, (s32) command->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, indexBufferOffset);
                }
            }
            indexBufferOffset += command->ElemCount;
        }
    }
    glDeleteVertexArrays(1, &vaoHandle);

    // Restore modified GL state
    glUseProgram(lastProgram);
    if (glBindSampler) {
        glBindSampler(0, lastSampler);
    }
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glActiveTexture(lastActiveTexture);
    glBindVertexArray(lastVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lastVAO);
    glBlendEquationSeparate(lastBlendEquationRGB, lastBlendEquationAlpha);
    glBlendFuncSeparate(lastBlendSourceRGB, lastBlendDestRGB, lastBlendSourceAlpha, lastBlendDestAlpha);

    if (lastEnableBlend) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }

    if (lastEnableCullFace) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }

    if (lastEnableDepthTest) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    if (lastEnableScissorTest) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }

    glPolygonMode(GL_FRONT_AND_BACK, (u32) lastPolygonMode[0]);
    glViewport(lastViewport[0], lastViewport[1], (s32) lastViewport[2], (s32) lastViewport[3]);
    glScissor(lastScissorBox[0], lastScissorBox[1], (s32) lastScissorBox[2], (s32) lastScissorBox[3]);
}
