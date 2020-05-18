#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/graphics/api.h"
#include "lstd/os.h"

#undef MAC
#undef _MAC
#include <Windows.h>

#include <d3d10_1.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

LSTD_BEGIN_NAMESPACE

static ID3DBlob *compile_shader(string source, const char *profile, const char *main) {
    ID3DBlob *shaderBlob = null, *errorBlob = null;
    DXCHECK(D3DCompile(source.Data, source.ByteLength, null, null, null, main, profile, D3DCOMPILE_DEBUG, 0,
                       &shaderBlob, &errorBlob));
    if (errorBlob) {
        fmt::print("... shader compile errors (profile = {!GRAY}{}{!}):\n{!YELLOW}{}\n{!}\n", profile,
                   (const char *) errorBlob->GetBufferPointer());
        assert(false);
        errorBlob->Release();
    }
    return shaderBlob;
}

static gtype string_to_gtype(string type) {
    size_t digit = type.find_any_of("0123456789");
    if (digit != npos) {
        size_t x = type.find('x');
        string scalarType = type.substring(0, digit);
        size_t offset = (type[digit] - '0' - 1) * 4 + (x == npos ? 0 : type[x + 1] - '0' - 1);
        if (scalarType == "bool") return (gtype)((u32) gtype::BOOL_1x1 + offset);
        if (scalarType == "int" || scalarType == "int32") return (gtype)((u32) gtype::S32_1x1 + offset);
        if (scalarType == "uint" || scalarType == "uint32" || scalarType == "dword")
            return (gtype)((u32) gtype::U32_1x1 + offset);
        if (scalarType == "float") return (gtype)((u32) gtype::F32_1x1 + offset);
    } else {
        if (type == "bool") return gtype::BOOL;
        if (type == "int" || type == "int32") return gtype::S32;
        if (type == "uint" || type == "uint32" || type == "dword") return gtype::U32;
        if (type == "float") return gtype::F32;
    }
    return gtype::Unknown;
}

void d3d_shader_init(shader *s) {
    auto source = s->Source;

    s->D3D.VSBlob = compile_shader(source, "vs_4_0", "VSMain");
    s->D3D.PSBlob = compile_shader(source, "ps_4_0", "PSMain");

    auto *vs = (ID3DBlob *) s->D3D.VSBlob;
    auto *ps = (ID3DBlob *) s->D3D.PSBlob;
    s->Graphics->D3D.Device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), null, &s->D3D.VS);
    s->Graphics->D3D.Device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), null, &s->D3D.PS);
    
    // Remove comments
    // size_t startPos;
    // while ((startPos = source.find("/*")) != npos) {
    //     size_t endPos = source.find("*/", startPos);
    //     assert(endPos != npos);
    //     source.remove(startPos, endPos);
    // }
	// 
    // while ((startPos = source.find("//")) != npos) {
    //     size_t endPos = source.find('\n', startPos);
    //     assert(endPos != npos);
    //     source.remove(startPos, endPos);
    // }
    // @TODO Parse shaders structs
    /*

    // Parse constant buffers and extract metadata
    size_t cbuffer = 0;
    while ((cbuffer = source.find("cbuffer", cbuffer)) != npos) {
        size_t brace = source.find('}');
        string block = source.substring(cbuffer, brace + 1);

        while (true) {
            if (source.substring(cbuffer, brace).count('{') == block.count('}')) break;
            brace = source.find('}');
            block = source.substring(cbuffer, brace + 1);
        }
        ++cbuffer;

        // Tokenize
        array<string> tokens;
        {
            size_t start = 0;
            size_t end = block.find_any_of(" \t\r\n");

            while (end <= npos) {
                string token = block.substring(start, (end == npos ? block.Length : end));
                if (token) tokens.append(token);
                if (end == npos) break;
                start = end + 1;
                end = block.find_any_of(" \t\r\n", start);
            }
        }

        size_t tokenIndex = 1;

        shader::uniform_buffer uniformBuffer;
        clone(&uniformBuffer.Name, tokens[tokenIndex++]);

        if (tokens[tokenIndex++] != ":") {
            fmt::print("... error when parsing shader, no register found in constant buffer declaration!\n");
            fmt::print("    Here is the block:\n{!YELLOW}{}{!}\n", block);
            assert(false);
        }

        string reg = tokens[tokenIndex++];
        auto it = reg.begin();
        while (!is_digit(*it)) ++it;
        while (is_digit(*it)) {
            uniformBuffer.Position *= 10;
            uniformBuffer.Position += *it - '0';
            ++it;
        }

        tokenIndex++;  // "{"
        while (tokens[tokenIndex] != "}") {
            string type = tokens[tokenIndex++];
            if (type == "linear" || type == "centroid" || type == "nointerpolation" || type == "noperspective" ||
                type == "sample") {
                type = tokens[tokenIndex++];
            }

            // @TODO !!!
            if (type == "struct") {
                assert(false);
            }

            string name = tokens[tokenIndex++];
            if (name[-1] == ';') {
                name = name.substring(0, -1);
            } else {
                assert(tokens[tokenIndex++] == ";");
            }

            if (uniformBuffer.ByteSize % 16 != 0) {
                uniformBuffer.ByteSize = ((uniformBuffer.ByteSize >> 4) + 1) << 4;
            }

            shader::uniform decl;
            clone(&decl.Name, name);
            decl.Type = string_to_gtype(type);
            decl.Offset = uniformBuffer.ByteSize;
            decl.ByteSize = get_size_of_base_gtype_in_bits(decl.Type) / 8;  // Guaranteed not to be 1-bit
            decl.Count = get_count_of_gtype(decl.Type);
            uniformBuffer.ByteSize += decl.ByteSize * decl.Count;
            move(uniformBuffer.Uniforms.append(), &decl);
        }
        move(&s->UniformBuffers.append(uniformBuffer)->Uniforms, &uniformBuffer.Uniforms);
    }
    */
}

void d3d_shader_bind(shader *s) {
    s->Graphics->CurrentlyBoundShader = s;

    s->Graphics->D3D.DeviceContext->VSSetShader(s->D3D.VS, null, 0);
    s->Graphics->D3D.DeviceContext->PSSetShader(s->D3D.PS, null, 0);
}

void d3d_shader_unbind(shader *s) {
    s->Graphics->CurrentlyBoundShader = null;

    s->Graphics->D3D.DeviceContext->VSSetShader(null, null, 0);
    s->Graphics->D3D.DeviceContext->PSSetShader(null, null, 0);
}

void d3d_shader_release(shader *s) {
    SAFE_RELEASE(s->D3D.VS);
    SAFE_RELEASE(s->D3D.PS);

    auto *vs = (ID3DBlob *) s->D3D.VSBlob;
    auto *ps = (ID3DBlob *) s->D3D.PSBlob;
    SAFE_RELEASE(vs);
    SAFE_RELEASE(ps);
    s->D3D.VSBlob = null;
    s->D3D.PSBlob = null;
}

shader::impl g_D3DShaderImpl = {d3d_shader_init, d3d_shader_bind, d3d_shader_unbind, d3d_shader_release};

LSTD_END_NAMESPACE

#endif
