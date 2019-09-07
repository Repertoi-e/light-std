#include "pixel_buffer.h"

#include "../../vendor/stb_image.h"

LSTD_BEGIN_NAMESPACE

pixel_buffer::pixel_buffer(u8 *pixels, u32 width, u32 height, pixel_format format) {
    Pixels = pixels;
    Width = width;
    Height = height;
    Format = format;
    BPP = (s32) format;
}

pixel_buffer::pixel_buffer(file::path path, bool flipVertically, pixel_format format) {
    // auto handle = file::handle(path);
    //
    // string data;
    // handle.read_entire_file(&data);
    //
    // stbi_set_flip_vertically_on_load(flipVertically);
    //
    // // @Speed Killmenowkillmefastkillmenowkillmefast
    // s32 w, h, n;
    // u8 *loaded = stbi_load_from_memory((const u8 *) data.Data, (s32) data.ByteLength, &w, &h, &n, (s32) format);
    // size_t allocated = ((allocation_header *) loaded - 1)->Size;
    //
    // loaded = (u8 *) allocator::reallocate(loaded, allocated + POINTER_SIZE);
    // copy_memory(loaded + POINTER_SIZE, loaded, w * h * n);

    s32 w, h, n;
    u8 *loaded = stbi_load(path.UnifiedPath.to_c_string(), &w, &h, &n, (s32) format);

    Pixels = loaded;  // encode_owner((u8 *) loaded, this);
    Width = w;
    Height = h;
    Reserved = 0;  // allocated;
    Format = (pixel_format) n;
    BPP = (s32) Format;
}

void pixel_buffer::release() {
    if (is_owner()) {
        delete (Pixels - POINTER_SIZE);
    }
    Pixels = null;
    Format = pixel_format::Unknown;
    Width = Height = BPP = 0;
}

pixel_buffer *clone(pixel_buffer *dest, pixel_buffer src) {
    *dest = src;
    size_t size = src.Width * src.Height * src.BPP;
    dest->Pixels = new u8[size];
    copy_memory(dest->Pixels, src.Pixels, size);
    return dest;
}

pixel_buffer *move(pixel_buffer *dest, pixel_buffer *src) {
    dest->release();
    *dest = *src;

    if (!src->is_owner()) return dest;

    // Transfer ownership
    change_owner(src->Pixels, dest);
    change_owner(dest->Pixels, dest);
    return dest;
}

LSTD_END_NAMESPACE
