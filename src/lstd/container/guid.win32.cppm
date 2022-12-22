module;

#include "lstd/platform/windows.h"

export module lstd.guid.win32;

import lstd.string;

LSTD_BEGIN_NAMESPACE

export {
    // Used for generating unique ids
    struct guid {
        u8 Data[16];
        static const s64 Count = 16;

        // By default the guid is zero
        guid() {
            For(range(16)) Data[it] = 0;
        }

        explicit operator bool() {
            For(range(16)) if (Data[it]) return true;
            return false;
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
}

LSTD_END_NAMESPACE
