#include "graphics.h"

LSTD_BEGIN_NAMESPACE

namespace g {
gtype get_scalar_gtype(gtype type) {
    u32 v = (u32) type;
    if (v >= (u32) gtype::BOOL_1x1 && v <= (u32) gtype::BOOL_4x4) return gtype::BOOL;
    if (v >= (u32) gtype::U32_1x1 && v <= (u32) gtype::U32_4x4) return gtype::U32;
    if (v >= (u32) gtype::S32_1x1 && v <= (u32) gtype::S32_4x4) return gtype::S32;
    if (v >= (u32) gtype::F32_1x1 && v <= (u32) gtype::F32_4x4) return gtype::F32;
    return type;
}

size_t get_size_of_base_gtype_in_bits(gtype type) {
    if (type == gtype::BOOL) return 1;
    if (type == gtype::U8) return 8;
    if (type == gtype::S8) return 8;
    if (type == gtype::U16) return 16;
    if (type == gtype::S16) return 16;
    return 32;
}

size_t get_count_of_gtype(gtype type) {
    switch (type) {
        case gtype::BOOL_1x2:
        case gtype::U32_1x2:
        case gtype::S32_1x2:
        case gtype::F32_1x2:
        case gtype::BOOL_2x1:
        case gtype::U32_2x1:
        case gtype::S32_2x1:
        case gtype::F32_2x1:
            return 2;
        case gtype::BOOL_1x3:
        case gtype::U32_1x3:
        case gtype::S32_1x3:
        case gtype::F32_1x3:
        case gtype::BOOL_3x1:
        case gtype::U32_3x1:
        case gtype::S32_3x1:
        case gtype::F32_3x1:
            return 3;
        case gtype::BOOL_1x4:
        case gtype::U32_1x4:
        case gtype::S32_1x4:
        case gtype::F32_1x4:
        case gtype::BOOL_4x1:
        case gtype::U32_4x1:
        case gtype::S32_4x1:
        case gtype::F32_4x1:
            return 4;
        case gtype::BOOL_2x2:
        case gtype::U32_2x2:
        case gtype::S32_2x2:
        case gtype::F32_2x2:
            return 2 * 2;
        case gtype::BOOL_2x3:
        case gtype::U32_2x3:
        case gtype::S32_2x3:
        case gtype::F32_2x3:
            return 2 * 3;
        case gtype::BOOL_2x4:
        case gtype::U32_2x4:
        case gtype::S32_2x4:
        case gtype::F32_2x4:
            return 2 * 4;
        case gtype::BOOL_3x2:
        case gtype::U32_3x2:
        case gtype::S32_3x2:
        case gtype::F32_3x2:
            return 3 * 2;
        case gtype::BOOL_3x3:
        case gtype::U32_3x3:
        case gtype::S32_3x3:
        case gtype::F32_3x3:
            return 3 * 3;
        case gtype::BOOL_3x4:
        case gtype::U32_3x4:
        case gtype::S32_3x4:
        case gtype::F32_3x4:
            return 3 * 4;
        case gtype::BOOL_4x2:
        case gtype::U32_4x2:
        case gtype::S32_4x2:
        case gtype::F32_4x2:
            return 4 * 2;
        case gtype::BOOL_4x3:
        case gtype::U32_4x3:
        case gtype::S32_4x3:
        case gtype::F32_4x3:
            return 4 * 3;
        case gtype::BOOL_4x4:
        case gtype::U32_4x4:
        case gtype::S32_4x4:
        case gtype::F32_4x4:
            return 4 * 4;
        default:
            return 1;
    }
}

void buffer_layout::add(string name, gtype type, size_t count, bool normalized) {
    size_t sizeInBits = get_size_of_base_gtype_in_bits(type);

    count *= get_count_of_gtype(type);

    assert(TotalSize <= numeric_info<u32>::max());
    Elements.append({name, get_scalar_gtype(type), sizeInBits, normalized, (u32) count, (u32) TotalSize});

    if (sizeInBits == 1) sizeInBits = 8;
    TotalSize += (sizeInBits / 8) * count;
}
}  // namespace g

LSTD_END_NAMESPACE
