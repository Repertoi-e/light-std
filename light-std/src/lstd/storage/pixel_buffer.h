#pragma once

#include "../common.h"
#include "../file.h"
#include "owner_pointers.h"

LSTD_BEGIN_NAMESPACE

enum class pixel_format : s32 { Unknown = 0, Grey = 1, Grey_Alpha = 2, RGB = 3, RGBA = 4 };

struct pixel_buffer {
    pixel_format Format = pixel_format::Unknown;
    u32 Width = 0, Height = 0;
    s32 BPP = (s32) Format;  // BPP is == (s32) Format, but we set it anyway
    u8 *Pixels = null;
    size_t Reserved = 0;

    pixel_buffer() = default;

    // Just points to buffer (may get invalidated)
    pixel_buffer(u8 *pixels, u32 width, u32 height, pixel_format format);

    //
    // Loads from a file.
    //
    // If _format_ is not passed as _Unknown_, the file is loaded and converted to the requested one.
    // The _Format_ member is set at _Unknown_ if the load failed.
    pixel_buffer(file::path path, bool flipVertically = false, pixel_format format = pixel_format::Unknown);

    ~pixel_buffer() { release(); }

    void release();

    // Returns true if this object has any memory allocated by itself
    bool is_owner() const { return Reserved && decode_owner<pixel_buffer>(Pixels) == this; }
};

pixel_buffer *clone(pixel_buffer *dest, pixel_buffer src);
pixel_buffer *move(pixel_buffer *dest, pixel_buffer *src);

LSTD_END_NAMESPACE
