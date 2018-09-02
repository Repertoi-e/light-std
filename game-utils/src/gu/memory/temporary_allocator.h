#pragma once

#include "../string/string.h"
#include "memory.h"

GU_BEGIN_NAMESPACE

// A temporary allocator. Init with a set size, (wrapper: temporary_storage_init), and it
// can be used globally to allocate memory that is not meant to last long. With this allocator you don't
// free individual allocations, but instead FREE_ALL the allocator (wrapper: temporary_storage_reset). You can
// set this allocator as the context's one and any code you call uses the very very fast and cheap
// allocator (provided it does not create a custom allocator or not use our context at all).
//
// A typical place to reset the allocator is at the start of every frame, if you are doing a game for example.
//
// Note that calling the allocator with Allocator_Mode::FREE (or the wrapper Delete from memory.h) does nothing.
//
struct Temporary_Storage {
    void *Data = 0;
    size_t Size = 0, Occupied = 0, HighestUsed = 0;
};

inline Temporary_Storage *__temporary_allocator_data;

template <typename... Args>
void print(string const &format, Args &&... argsPack);

inline void *__temporary_allocator(Allocator_Mode mode, void *allocatorData, size_t size, void *oldMemory,
                                   size_t oldSize, s32 options) {
    Temporary_Storage *storage = (Temporary_Storage *) allocatorData;

    switch (mode) {
        case Allocator_Mode::ALLOCATE:
            [[fallthrough]];
        case Allocator_Mode::RESIZE: {
            if (storage->Occupied + size > storage->Size) {
                bool switched = false;

                if (__context.Allocator.Function == __temporary_allocator ||
                    __context.Allocator.Data == __temporary_allocator_data) {
                    switched = true;
                    __context.Allocator = MALLOC;
                }
                __temporary_allocator_data = 0;

                print("!!! Warning !!!\n");
                print(">> Temporary allocator ran out of space, using malloc for allocation...\n");
                print(">> Invalidating pointer to __temporary_allocator_data...\n");
                if (switched) print(">> Context detected with temporary allocator, switching it to malloc...\n");

                return __default_allocator(mode, allocatorData, size, oldMemory, oldSize, options);
            }

            void *block = (byte *) storage->Data + storage->Occupied;

            if (mode == Allocator_Mode::RESIZE) {
                CopyMemory(block, oldMemory, oldSize);
            }

            storage->Occupied += size;
            storage->HighestUsed = Max(storage->HighestUsed, storage->Occupied);
            return block;
        }
        case Allocator_Mode::FREE:
            return 0;
        case Allocator_Mode::FREE_ALL:
            storage->Occupied = 0;
            return 0;
        default:
            assert(false);  // We shouldn't get here
    }
    return 0;
}

inline void temporary_storage_init(size_t allocatorSize) {
    __temporary_allocator_data = New<Temporary_Storage>(MALLOC);

    __temporary_allocator_data->Data = New<byte>(allocatorSize, MALLOC);
    __temporary_allocator_data->Size = allocatorSize;
}

inline void temporary_storage_reset() { __temporary_allocator_data->Occupied = 0; }

// Use for regions that use a lot of temporary memory but you don't need the memory outside of them.
// This can be used as a "partial" free all of the temporary allocator which can be useful if there is
// not enough temporary storage and you don't want to init it with a larger size.
inline size_t temporary_storage_get_mark() { return __temporary_allocator_data->Occupied; };
inline void temporary_storage_set_mark(size_t mark) { __temporary_allocator_data->Occupied = mark; };

#define TEMP_VAR_GEN_(LINE) __game_utils_temporary_storage_mark_##LINE
#define TEMP_VAR_GEN(LINE) TEMP_VAR_GEN_(LINE)
#define TEMPORARY_STORAGE_MARK_SCOPE                              \
    size_t TEMP_VAR_GEN(__LINE__) = temporary_storage_get_mark(); \
    defer { temporary_storage_set_mark(TEMP_VAR_GEN(__LINE__)); }

#define TEMPORARY_ALLOC \
    Allocator_Closure { __temporary_allocator, __temporary_allocator_data }

GU_END_NAMESPACE
