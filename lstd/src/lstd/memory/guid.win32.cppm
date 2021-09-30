module;

#include "lstd/platform/windows.h"

export module lstd.guid.win32;

import lstd.string;

LSTD_BEGIN_NAMESPACE

export {
    // Used for generating unique ids
    struct guid {
        stack_array<u8, 16> Data;

        // By default the guid is zero
        constexpr guid() {
            For(range(16)) Data[it] = 0;
        }

        constexpr auto operator<=>(guid other) const { return Data <=> other.Data; }

        constexpr operator bool() {
            guid empty;
            return Data != empty.Data;
        }
    };

    // Guaranteed to generate a unique id each time (time-based)
    guid create_guid() {
        GUID g;
        CoCreateGuid(&g);

        auto data = make_stack_array((u8) (g.Data1 >> 24 & 0xFF),
                                     (u8) (g.Data1 >> 16 & 0xFF),
                                     (u8) (g.Data1 >> 8 & 0xFF),
                                     (u8) (g.Data1 & 0xff),

                                     (u8) (g.Data2 >> 8 & 0xFF),
                                     (u8) (g.Data2 & 0xff),

                                     (u8) (g.Data3 >> 8 & 0xFF),
                                     (u8) (g.Data3 & 0xFF),

                                     g.Data4[0],
                                     g.Data4[1],
                                     g.Data4[2],
                                     g.Data4[3],
                                     g.Data4[4],
                                     g.Data4[5],
                                     g.Data4[6],
                                     g.Data4[7]);

        guid result;
        For(range(16)) result.Data[it] = data[it];
        return result;
    }

    u64 get_hash(guid value) {
        u64 hash             = 5381;
        For(value.Data) hash = (hash << 5) + hash + it;
        return hash;
    }
}

LSTD_END_NAMESPACE
