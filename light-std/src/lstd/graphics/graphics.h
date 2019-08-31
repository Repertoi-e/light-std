#pragma once

/// Defines the graphics API that can be used to draw stuff on windows.
/// Implementations of it can be switched dynamically.

#include "../file.h"
#include "../video.h"

LSTD_BEGIN_NAMESPACE

enum class gtype {
    Unknown = 0,

    BOOL,

    U8,
    U16,
    U32,

    S8,
    S16,
    S32,

    F32,

    BOOL_1x1,
    BOOL_1x2,
    BOOL_1x3,
    BOOL_1x4,
    BOOL_2x1,
    BOOL_2x2,
    BOOL_2x3,
    BOOL_2x4,
    BOOL_3x1,
    BOOL_3x2,
    BOOL_3x3,
    BOOL_3x4,
    BOOL_4x1,
    BOOL_4x2,
    BOOL_4x3,
    BOOL_4x4,

    U32_1x1,
    U32_1x2,
    U32_1x3,
    U32_1x4,
    U32_2x1,
    U32_2x2,
    U32_2x3,
    U32_2x4,
    U32_3x1,
    U32_3x2,
    U32_3x3,
    U32_3x4,
    U32_4x1,
    U32_4x2,
    U32_4x3,
    U32_4x4,

    S32_1x1,
    S32_1x2,
    S32_1x3,
    S32_1x4,
    S32_2x1,
    S32_2x2,
    S32_2x3,
    S32_2x4,
    S32_3x1,
    S32_3x2,
    S32_3x3,
    S32_3x4,
    S32_4x1,
    S32_4x2,
    S32_4x3,
    S32_4x4,

    F32_1x1,
    F32_1x2,
    F32_1x3,
    F32_1x4,
    F32_2x1,
    F32_2x2,
    F32_2x3,
    F32_2x4,
    F32_3x1,
    F32_3x2,
    F32_3x3,
    F32_3x4,
    F32_4x1,
    F32_4x2,
    F32_4x3,
    F32_4x4,

    BOOL_4 = BOOL_4x1,
    U32_4 = U32_4x1,
    S32_4 = S32_4x1,
    F32_4 = F32_4x1,

    BOOL_3 = BOOL_3x1,
    U32_3 = U32_3x1,
    S32_3 = S32_3x1,
    F32_3 = F32_3x1,

    BOOL_2 = BOOL_2x1,
    U32_2 = U32_2x1,
    S32_2 = S32_2x1,
    F32_2 = F32_2x1,

    BOOL_1 = BOOL_1x1,
    U32_1 = U32_1x1,
    S32_1 = S32_1x1,
    F32_1 = F32_1x1
};

// Returns the size of the scalar type, not the whole type, e.g. returns 32 on F32_4x4
gtype get_scalar_gtype(gtype type);
size_t get_size_of_base_gtype_in_bits(gtype type);
size_t get_count_of_gtype(gtype type);

struct texture : non_copyable, non_movable {
    enum wrap { NONE = 0, REPEAT, CLAMP, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER };
    enum filter { LINEAR, NEAREST };

    // @TODO: _texture_ should be an asset, move these fields when we have a catalog system
    string Name;
    file::path FilePath = "No path";  // We may have textures that are not created from disc

    wrap Wrap;
    filter Filter;

    virtual ~texture() = default;

    virtual void bind(u32 slot) = 0;

// For debugging purposes (to reset state), shouldn't use this in Dist
#if defined DEBUG || defined RELEASE
    virtual void unbind(u32 slot) = 0;
#endif

    virtual void release() = 0;
};

struct texture_2D : public texture {
    u32 Width, Height;

    // Set pixel data
    virtual void set_data(const u8 *pixels) = 0;

    // Fill the texture with 1 color
    virtual void set_data(u32 color) = 0;
};

// Holds both a vertex and a pixel shader (those are the two shader types we support for now!)
struct shader : non_copyable, non_movable {
    enum type { NONE = 0, VERTEX_SHADER, FRAGMENT_SHADER };

    struct uniform {
        string Name;
        gtype Type = gtype::Unknown;

        size_t ByteSize = 0;
        size_t Offset = 0;
        size_t Count = 0;  // _Count_ * _Size_ gives the total size
    };

    struct uniform_buffer {
        string Name;
        size_t ByteSize = 0;
        array<uniform> Uniforms;

        type ShaderType = NONE;
        u32 Position = 0;
    };

    // @TODO: _shader_ should be an asset, move these fields when we have a catalog system
    string Name;
    file::path FilePath;

    array<uniform_buffer> UniformBuffers;

    virtual ~shader() = default;

    virtual void bind() = 0;

// For debugging purposes (to reset state), shouldn't use this in Dist
#if defined DEBUG || defined RELEASE
    virtual void unbind() = 0;
#endif

    virtual void release() = 0;
};

struct buffer_layout {
    struct element {
        string Name;
        gtype Type = gtype::Unknown;
        size_t SizeInBits = 0;
        bool Normalized = false;
        size_t Count = 0;
        u32 AlignedByteOffset = 0;  // 1-bit values add 7 bits of packing for the next element
    };

    array<element> Elements;
    size_t TotalSize = 0;  // Calculated in bytes (1-bit values add 7 bits of packing),
                           // generally used internally to calculate the offset for the next element

    void add(string name, gtype type, size_t count = 1, bool normalized = false);
};

enum class primitive_topology { PointList = 0, LineList, LineStrip, TriangleList, TriangleStrip };

struct buffer : non_copyable, non_movable {
    enum type {
        NONE = 0,
        VERTEX_BUFFER,
        INDEX_BUFFER,
        SHADER_UNIFORM_BUFFER  // Used to pack shader data, "constant buffers" (dx), "uniform buffer objects" (gl), etc.
    };

    // This only makes sense when using DX, OpenGL doesn't support these options when binding a buffer
    enum usage {
        DEFAULT,    // The buffer requires read and write access by the GPU
        IMMUTABLE,  // Cannot be modified after creation, so it must be created with initial data
        DYNAMIC,    // Can be written to by the CPU and read by the GPU
        STAGING     // Supports data transfer (copy) from the GPU to the CPU
    };

    enum map_access {
        READ,                    // Buffer can only be read by the CPU
        WRITE,                   // Buffer can only be written to by the CPU
        READ_WRITE,              // Buffer can be read and written to by the CPU
        WRITE_DISCARD_PREVIOUS,  // Previous contents of buffer may be discarded, and new buffer is opened for writing
        WRITE_UNSYNCHRONIZED  // An advanced option that allows you to add more data to the buffer even while the GPU is
                              // using parts. However, you must not work with the parts the GPU is using.
    };

    union bind_data {
        char Raw[24]{};

        // Used with vertex buffer
        struct {
            primitive_topology Topology;
            u32 Stride;  // Gets automatically determined by the layout if left at 0
            u32 Offset;  // 0 by default
        };

        // Used with index buffer
        // (already defined) size_t Offset;  // 0 by default

        // Used with shader data buffer
        struct {
            shader::type ShaderType;  // PS or VS
            u32 Position;             // register (dx), location (gl)
        };
    };

    type Type = NONE;
    usage Usage = DEFAULT;
    size_t Size = 0;
    size_t Stride = 0;  // Determined by the buffer layout

    virtual ~buffer() = default;

    virtual void set_input_layout(buffer_layout layout) = 0;

    virtual void *map(map_access access) = 0;
    virtual void unmap() = 0;

    virtual void bind(bind_data bindData) = 0;

// For debugging purposes (to reset state), shouldn't use this in Dist
#if defined DEBUG || defined RELEASE
    virtual void unbind() = 0;
#endif

    virtual void release() = 0;
};

enum class cull : u32 { None = 0, Front, Back };

struct graphics : non_copyable, non_movable {
    virtual ~graphics() = default;

    virtual void init() = 0;

    virtual void add_target_window(window *win) = 0;
    virtual void remove_target_window(window *win) = 0;
    virtual void set_current_target_window(window *win) = 0;

    virtual void clear_color(vec4 color) = 0;

    virtual void set_blend(bool enabled) = 0;
    virtual void set_depth_testing(bool enabled) = 0;
    virtual void set_cull_mode(cull mode) = 0;

    virtual void create_buffer(buffer *buffer, buffer::type type, buffer::usage usage, size_t size) = 0;
    virtual void create_buffer(buffer *buffer, buffer::type type, buffer::usage usage, const char *initialData,
                               size_t initialDataSize) = 0;
    void create_buffer(buffer *buffer, buffer::type type, buffer::usage usage, array_view<char> initialData) {
        create_buffer(buffer, type, usage, initialData.begin(), initialData.size());
    }

    virtual void create_shader(shader *shader, string name, file::path path) = 0;

    virtual void create_texture_2D(texture_2D *texture, string name, u32 width, u32 height,
                                   texture::filter filter = texture::LINEAR, texture::wrap wrap = texture::CLAMP) = 0;

    virtual void create_texture_2D_from_file(texture_2D *texture, string name, file::path path, bool flipX = false,
                                             bool flipY = false, texture::filter filter = texture::LINEAR,
                                             texture::wrap wrap = texture::CLAMP) = 0;

    virtual void draw(size_t vertices, size_t startVertexLocation = 0) = 0;
    virtual void draw_indexed(size_t indices, size_t startIndex = 0, size_t baseVertexLocation = 0) = 0;

    virtual void swap() = 0;

    virtual void release() = 0;
};

LSTD_END_NAMESPACE
