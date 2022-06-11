module;

#include "../common.h"
#include "lstd/platform/windows.h"  // Declarations of Win32 functions

//
// Platform specific memory functions.
//

export module lstd.os.win32.memory;

import lstd.fmt;
import lstd.thread;

LSTD_BEGIN_NAMESPACE

export {
    // Allocates memory by calling the OS directly
    [[nodiscard("Leak")]] void *os_allocate_block(s64 size);

    // Expands/shrinks a memory block allocated by os_allocate_block().
    // This is NOT realloc. When this fails it returns null instead of allocating a new block and copying the contents of the old one.
    // That's why it's not called realloc.
    [[nodiscard("Leak")]] void *os_resize_block(void *ptr, s64 newSize);

    // Returns the size of a memory block allocated by os_allocate_block() in bytes
    s64 os_get_block_size(void *ptr);

    // Frees a memory block allocated by os_allocate_block()
    void os_free_block(void *ptr);

    // Creates/opens a shared memory block and writes data to it (use this for communication between processes)
    // @Security?
    void os_write_shared_block(string name, void *data, s64 size);

    // Read data from a shared memory block (use this for communication between processes)
    // @Security?
    void os_read_shared_block(string name, void *out, s64 size);

    // This is a utility which helps reduce fragmentation when you allocate multiple structs.
    //
    // Allocates one giant block with size determined from the size of types passed in (+ extraDynamicSize).
    // After that this calls constructors on non-scalar values.
    //
    // Returns a tuple with the pointers to the structs. The first pointer is the pointer to the beginning of the big block
    // which is the one which may eventually be passed to os_free_block().
    // The block with size _extraDynamicSize_ is the last in the tuple (with type void*).
    //
    // @Robustness Caveat: This doesn't call constructors on arrays, e.g. my_data_t[n].
    //       We can implement this but the code is going to get much more complicated.
    template <typename... Types>
    [[nodiscard("Leak")]] auto os_allocate_packed(s64 extraDynamicSize) {
        constexpr s64 TYPE_SIZE[]     = {sizeof(Types)...};
        constexpr s64 TOTAL_TYPE_SIZE = (sizeof(Types) + ...);

        // We decay, remove pointers and add a pointer in order to handle arrays
        // e.g.  byte[10] -> decays to -> byte *, but here if we add a pointer again, we would get byte **
        // The reason we return pointers is because we return the address in the block each element begins.
        using result_t = tuple<types::add_pointer_t<types::remove_pointer_t<types::decay_t<Types>>>..., void *>;
        result_t result;

        s64 size = TOTAL_TYPE_SIZE + extraDynamicSize;

        void *block = os_allocate_block(size);

        s64 offset = 0;
        static_for<0, sizeof...(Types)>([&result, &offset, block, TYPE_SIZE](auto i) {
            using element_t = tuple_get_t<i, result_t>;

            auto *p              = (element_t) ((byte *) block + offset);
            tuple_get<i>(result) = p;

            using element_t_no_pointer = types::remove_pointer_t<element_t>;

            // Call constructor on values that are not scalars
            // @Robustness This doesn't call constructors on arrays, e.g. my_data_t[n]
            if constexpr (!types::is_scalar<element_t_no_pointer>) {
                new (p) element_t_no_pointer;
            }

            offset += TYPE_SIZE[i];
        });

        tuple_get<sizeof...(Types)>(result) = (byte *) block + offset;

        return result;
    }
}

struct win32_memory_state {
    // Used to store global state (e.g. cached command-line arguments/env variables or directories), a tlsf allocator
    allocator PersistentAlloc;
    tlsf_allocator_data PersistentAllocData;

    // Stores blocks that have been added as pools for the tlsf allocator or a large allocation which was handled by _os_allocate_block()_.
    struct persistent_alloc_page {
        persistent_alloc_page *Next;
    };
    persistent_alloc_page *PersistentAllocBasePage;

    mutex PersistentAllocMutex;

    //
    // We don't use the default thread-local temporary allocator because we don't want to mess with the user's memory.
    //
    // Used for temporary storage (e.g. converting strings from utf8 to wchar for windows calls).
    // Memory returned is only guaranteed to be valid until the next call, because we call free_all
    // if we don't have enough space for the allocation. See note above _win32_temp_alloc()_.
    allocator TempAlloc;
    arena_allocator_data TempAllocData;

    mutex TempAllocMutex;  // @TODO: Remove
};

// :GlobalStateNoConstructors:
byte State[sizeof(win32_memory_state)];

// Short-hand macro for sanity
#define S ((win32_memory_state *) &State[0])

export {
    // @TODO: Print call stack
    void platform_report_warning(string message, source_location loc = source_location::current()) {
        print(">>> {!YELLOW}Platform warning{!} {}:{} (in function: {}): {}.\n", loc.File, loc.Line, loc.Function, message);
    }

    void platform_report_error(string message, source_location loc = source_location::current()) {
        print(">>> {!RED}Platform error{!} {}:{} (in function: {}): {}.\n", loc.File, loc.Line, loc.Function, message);
    }
}

void create_new_temp_storage_block(s64 size) {
    if (S->TempAllocData.Block) {
        if (!os_resize_block(S->TempAllocData.Block, size)) {
            os_free_block(S->TempAllocData.Block);
            S->TempAllocData.Block = os_allocate_block(size);
        }
    } else {
        S->TempAllocData.Block = os_allocate_block(size);
    }

    S->TempAllocData.Size = size;
    S->TempAllocData.Used = 0;
}

// An extension to the arena allocator. Calls free_all when not enough space. Because we are not running e.g. a game
// there is no clear point at which to free_all the temporary allocator, that's why we assume that no allocation
// made with TempAlloc should persist beyond the next allocation.
//
// Note: This allocator doesn't work 100% in a multithreaded situation because free_all could be called at 
// an arbitrary point in time.
// 
// @TODO: Replace calls to the temporary alloc with calls to the persient alloc. They are both are fast. 
// Obviously this allocator is faster, but there is no clear point as to when we can safely call free_all..
// In a game free_all is called at the end of each frame which means there is no problem there
// since temporary allocations shouldn't persist until the next frame.
void *win32_temp_alloc(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
    auto *data = (arena_allocator_data *) context;

    lock(&S->TempAllocMutex);
    defer(unlock(&S->TempAllocMutex));

    auto *result = arena_allocator(mode, context, size, oldMemory, oldSize, options);
    if (!result && mode == allocator_mode::ALLOCATE) {
        if (size < S->TempAllocData.Size) {
            // If we couldn't allocate but the temporary storage block has enough space, we just call free_all
            free_all(S->TempAlloc);
            result = arena_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
            // Problem in multithreaded! See note above.
        } else {
            // If we try to allocate a block with size bigger than the temporary storage block,
            // we make a new, larger temporary storage block.

            platform_report_warning("Not enough memory in the temporary allocator block; reallocating the pool");

            os_free_block(S->TempAllocData.Block);
            create_new_temp_storage_block(size * 2);

            result = arena_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
        }
    }

    return result;
}

// Returns a pointer to the usable memory
void *create_persistent_alloc_page(s64 size) {
    void *result = os_allocate_block(size + sizeof(win32_memory_state::persistent_alloc_page));

    auto *p = (win32_memory_state::persistent_alloc_page *) result;

    p->Next = S->PersistentAllocBasePage;
    if (!S->PersistentAllocBasePage) {
        S->PersistentAllocBasePage = p;
    }

    return (void *) (p + 1);
}

void *win32_persistent_alloc(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
    auto *data = (tlsf_allocator_data *) context;

    lock(&S->PersistentAllocMutex);
    defer(unlock(&S->PersistentAllocMutex));

    if (mode == allocator_mode::ALLOCATE && ((u64) (size * 2) > PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE)) {
        platform_report_warning("Large allocation requested for the platform persistent allocator; querying the OS for memory directly");
        return create_persistent_alloc_page(size);
    }

    auto *result = tlsf_allocator(mode, context, size, oldMemory, oldSize, options);
    if (mode == allocator_mode::ALLOCATE && !result) {
        platform_report_warning("Not enough memory in the persistent allocator; adding another pool");

        void *block = create_persistent_alloc_page(PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE);
        tlsf_allocator_add_pool(&S->PersistentAllocData, block, PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE);

        result = tlsf_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
        assert(result);
    }
    return result;
}

// These functions are used by other windows platform files.
export {
    allocator platform_get_persistent_allocator() { return S->PersistentAlloc; }
    allocator platform_get_temporary_allocator() { return S->TempAlloc; }

    void platform_init_allocators() {
        S->TempAllocMutex       = create_mutex();
        S->PersistentAllocMutex = create_mutex();

        S->TempAlloc = {win32_temp_alloc, &S->TempAllocData};

        S->TempAllocData.Block = null;

        create_new_temp_storage_block(PLATFORM_TEMPORARY_STORAGE_STARTING_SIZE);

        S->PersistentAllocBasePage = null;
        S->PersistentAlloc         = {win32_persistent_alloc, &S->PersistentAllocData};

        void *block = create_persistent_alloc_page(PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE);
        tlsf_allocator_add_pool(&S->PersistentAllocData, block, PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE);
    }

    void platform_uninit_allocators() {
        lock(&S->TempAllocMutex);
        lock(&S->PersistentAllocMutex);

        // Free all pages (pools and big allocations)
        auto *p = S->PersistentAllocBasePage;
        while (p) {
            auto *o = p;
            p       = p->Next;
            os_free_block(o);
        }
        S->PersistentAllocBasePage = null;

        // Free temporary storage arena
        os_free_block(S->TempAllocData.Block);
        S->TempAllocData.Size = 0;
        S->TempAllocData.Used = 0;

        // Release mutexes

        unlock(&S->TempAllocMutex);
        unlock(&S->PersistentAllocMutex);

        free_mutex(&S->TempAllocMutex);
        free_mutex(&S->PersistentAllocMutex);
    }

    // Windows uses wchar.. Sigh...
    //
    // This function uses the platform temporary allocator if no explicit allocator was specified.
    wchar *platform_utf8_to_utf16(string str, allocator alloc = {}) {
        if (!str.Count) return null;

        if (!alloc) alloc = S->TempAlloc;

        wchar *result;
        PUSH_ALLOC(alloc) {
            // src.Length * 2 because one unicode character might take 2 wide chars.
            // This is just an approximation, not all space will be used!
            result = malloc<wchar>({.Count = string_length(str) * 2 + 1});
        }

        utf8_to_utf16(str.Data, string_length(str), result);
        return result;
    }

    // This function uses the platform temporary allocator if no explicit allocator was specified.
    string platform_utf16_to_utf8(const wchar *str, allocator alloc = {}) {
        string result;

        if (!alloc) alloc = S->TempAlloc;

        PUSH_ALLOC(alloc) {
            // String length * 4 because one unicode character might take 4 bytes in utf8.
            // This is just an approximation, not all space will be used!
            make_dynamic(&result, c_string_length(str) * 4);
        }

        utf16_to_utf8(str, (char *) result.Data, &result.Count);

        return result;
    }
}

// Tests whether the allocation contraction is possible
bool is_contraction_possible(s64 oldSize) {
    // Check if object allocated on low fragmentation heap.
    // The LFH can only allocate blocks up to 16KB in size.
    if (oldSize <= 0x4000) {
        LONG heapType = -1;
        if (!HeapQueryInformation(GetProcessHeap(), HeapCompatibilityInformation, &heapType, sizeof(heapType), null)) {
            return false;
        }
        return heapType != 2;
    }

    // Contraction possible for objects not on the LFH
    return true;
}

void *os_allocate_block(s64 size) {
    assert(size < MAX_ALLOCATION_REQUEST);
    return HeapAlloc(GetProcessHeap(), 0, size);
}

void *os_resize_block(void *ptr, s64 newSize) {
    assert(ptr);
    assert(newSize < MAX_ALLOCATION_REQUEST);

    s64 oldSize = os_get_block_size(ptr);
    if (newSize == 0) newSize = 1;

    void *result = HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY, ptr, newSize);
    if (result) return result;

    // If a failure to contract was caused by platform limitations, just return the original block
    if (newSize < oldSize && !is_contraction_possible(oldSize)) return ptr;

    // HeapReAlloc doesn't set an error

    return null;
}

s64 os_get_block_size(void *ptr) {
    s64 result = HeapSize(GetProcessHeap(), 0, ptr);
    if (result == -1) {
        // HeapSize doesn't set an error
        return 0;
    }
    return result;
}

#define CREATE_MAPPING_CHECKED(handleName, call)                                                               \
    HANDLE handleName = call;                                                                                  \
    if (!handleName) {                                                                                         \
        string extendedCallSite = sprint("{}\n        (the name was: {!YELLOW}\"{}\"{!GRAY})\n", #call, name); \
        char *cStr              = string_to_c_string(extendedCallSite);                                        \
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), cStr);                                \
        free(cStr);                                                                                            \
        free(extendedCallSite.Data);                                                                           \
        return;                                                                                                \
    }

void os_write_shared_block(string name, void *data, s64 size) {
    wchar *name16 = platform_utf8_to_utf16(name, S->PersistentAlloc);
    defer(free(name16));

    CREATE_MAPPING_CHECKED(h, CreateFileMappingW(INVALID_HANDLE_VALUE, null, PAGE_READWRITE, 0, (DWORD) size, name16));
    defer(CloseHandle(h));

    WIN32_CHECK_BOOL(result, MapViewOfFile(h, FILE_MAP_WRITE, 0, 0, size));
    if (!result) return;

    copy_memory_fast(result, data, size);
    UnmapViewOfFile(result);
}

void os_read_shared_block(string name, void *out, s64 size) {
    wchar *name16 = platform_utf8_to_utf16(name, S->PersistentAlloc);
    defer(free(name16));

    CREATE_MAPPING_CHECKED(h, OpenFileMappingW(FILE_MAP_READ, false, name16));
    defer(CloseHandle(h));

    WIN32_CHECK_BOOL(result, MapViewOfFile(h, FILE_MAP_READ, 0, 0, size));
    if (!result) return;

    copy_memory_fast(out, result, size);
    UnmapViewOfFile(result);
}

void os_free_block(void *ptr) {
    WIN32_CHECK_BOOL(r, HeapFree(GetProcessHeap(), 0, ptr));
}

LSTD_END_NAMESPACE
