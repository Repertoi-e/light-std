module;

#include "lstd/memory/string.h"
#include "lstd/common/windows.h"  // Declarations of Win32 functions

//
// Platform specific memory functions.
//

export module os.win64.memory;

import fmt;

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
    void os_write_shared_block(const string &name, void *data, s64 size);

    // Read data from a shared memory block (use this for communication between processes)
    void os_read_shared_block(const string &name, void *out, s64 size);

    // This is a utility which aids in reducing fragmentation when you can allocate program state in one place in the code.
    //
    // Allocates one giant block with size determined from the size of types passed in (+ extraDynamicSize).
    // Returns pointers in the block spaced out accordingly.
    // After that this calls constructors on non-scalar values.
    //
    // Note: @Robustness This doesn't call constructors on arrays, e.g. my_data_t[n].
    //       We can implement this but the code is going to get much more complicated.
    //
    // Returns a tuple with pointers to each type in the resulting block (the first pointer is the pointer to the beginning of the big block).
    // The block with size _extraDynamicSize_ is the last in the tuple (with type void*).
    template <typename... Types>
    [[nodiscard("Leak")]] auto os_allocate_packed(s64 extraDynamicSize) {
        constexpr s64 TYPE_SIZE[] = {sizeof(Types)...};
        constexpr s64 TOTAL_TYPE_SIZE = (sizeof(Types) + ...);

        // We decay, remove pointers and add a pointer in order to handle arrays
        // e.g.  byte[10] -> decays to -> byte *, but here if we add a pointer again, we would get byte **
        // The reason we return pointers is because we return the address in the block each element begins.
        using result_t = tuple<types::add_pointer_t<types::remove_pointer_t<types::decay_t<Types>>>..., void *>;
        result_t result;

        s64 size = TOTAL_TYPE_SIZE + extraDynamicSize;

        void *block = os_allocate_block(size);

        s64 offset = 0;
        static_for<0, sizeof...(Types)>([&](auto i) {
            using element_t = tuple_get_t<i, result_t>;

            auto *p = (element_t)((byte *) block + offset);
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

struct win64_memory_state {
    allocator PersistentAlloc;  // Used to store global state, a tlsf allocator
    thread::mutex PersistentAllocMutex;

    // We don't use the temporary allocator bundled with the Context because we don't want to mess with the user's memory.
    allocator TempAlloc;  // Used for temporary storage (e.g. converting strings from utf8 to utf16 for windows calls).
                          // Memory returned is only guaranteed to be valid until the next TempAlloc call, because we call free_all
                          // if we don't have enough space for the allocation.
    void *TempStorageBlock;
    s64 TempStorageSize;
    thread::mutex TempAllocMutex;
};

// :GlobalStateNoConstructors:
byte State[sizeof(win64_memory_state)];

// Short-hand macro for sanity
#define S ((win64_memory_state *) &State[0])

void create_temp_storage_block(s64);
void create_persistent_alloc_block(s64);

export namespace internal {
// @TODO: Add option to print call stack?
void platform_report_warning(string message, source_location loc = source_location::current()) {
    print(">>> {!YELLOW}Platform warning{!} {}:{} (in function: {}): {}.\n", loc.File, loc.Line, loc.Function, message);
}

// @TODO: Add option to print call stack?
void platform_report_error(string message, source_location loc = source_location::current()) {
    print(">>> {!RED}Platform error{!} {}:{} (in function: {}): {}.\n", loc.File, loc.Line, loc.Function, message);
}
}  // namespace internal

// An extension to the arena allocator. Calls free_all when not enough space. Because we are not running a game
// there is no clear point at which to free_all the temporary allocator, that's why we assume that no allocation
// made with TempAlloc should persist beyond the next allocation.
void *win64_temp_alloc(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
    auto *data = (arena_allocator_data *) context;

    thread::scoped_lock _(&S->TempAllocMutex);

    auto *result = arena_allocator(mode, context, size, oldMemory, oldSize, options);
    if (mode == allocator_mode::ALLOCATE) {
        if (size > S->TempStorageSize) {
            // If we try to allocate a block with size bigger than the temporary storage block, we make a new, larger temporary storage block

            internal::platform_report_warning("Not enough memory in the temporary allocator; expanding the pool");

            allocator_remove_pool(S->TempAlloc, S->TempStorageBlock);
            os_free_block((byte *) S->TempStorageBlock - sizeof(arena_allocator_data));

            create_temp_storage_block(size * 2);
            result = arena_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
        } else if (!result) {
            // If we couldn't allocate but the temporary storage block has enough space, we just call free_all
            free_all(S->TempAlloc);
            result = arena_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
        }
    }

    return result;
}

void create_temp_storage_block(s64 size) {
    // We allocate the arena allocator data and the starting pool in one big block in order to reduce fragmentation.
    auto [data, pool] = os_allocate_packed<arena_allocator_data>(size);
    S->TempAlloc = {win64_temp_alloc, data};
    allocator_add_pool(S->TempAlloc, pool, size);

    S->TempStorageBlock = pool;
    S->TempStorageSize = size;
}

void *win64_persistent_alloc(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
    auto *data = (tlsf_allocator_data *) context;

    thread::scoped_lock _(&S->PersistentAllocMutex);

    auto *result = tlsf_allocator(mode, context, size, oldMemory, oldSize, options);
    if (mode == allocator_mode::ALLOCATE) {
        if (!result) {
            internal::platform_report_warning("Not enough memory in the persistent allocator; adding a pool");

            create_persistent_alloc_block(size * 3);
        }
        result = tlsf_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
    }
    return result;
}

void create_persistent_alloc_block(s64 size) {
    // We allocate the arena allocator data and the starting pool in one big block in order to reduce fragmentation.
    auto [data, pool] = os_allocate_packed<tlsf_allocator_data>(size);
    S->PersistentAlloc = {win64_persistent_alloc, data};
    allocator_add_pool(S->PersistentAlloc, pool, size);
}

export namespace internal {
// These functions are used by other windows platform files.
allocator platform_get_persistent_allocator() { return S->PersistentAlloc; }
allocator platform_get_temporary_allocator() { return S->TempAlloc; }

void platform_init_allocators() {
    S->TempAllocMutex.init();
    S->PersistentAllocMutex.init();

    create_temp_storage_block(PLATFORM_TEMPORARY_STORAGE_STARTING_SIZE);
    create_persistent_alloc_block(PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE);
}

// Windows uses utf16.. Sigh...
//
// This function uses the platform temporary allocator if no explicit allocator was specified.
utf16 *platform_utf8_to_utf16(const string &str, allocator alloc = {}) {
    if (!str.Length) return null;

    if (!alloc) alloc = S->TempAlloc;

    utf16 *result;
    PUSH_ALLOC(alloc) {
        // src.Length * 2 because one unicode character might take 2 wide chars.
        // This is just an approximation, not all space will be used!
        result = allocate_array<utf16>(str.Length * 2 + 1);
    }

    utf8_to_utf16(str.Data, str.Length, result);
    return result;
}

// This function uses the platform temporary allocator if no explicit allocator was specified.
string platform_utf16_to_utf8(const utf16 *str, allocator alloc = {}) {
    string result;

    if (!alloc) alloc = S->TempAlloc;

    PUSH_ALLOC(alloc) {
        // String length * 4 because one unicode character might take 4 bytes in utf8.
        // This is just an approximation, not all space will be used!
        string_reserve(result, c_string_length(str) * 4);
    }

    utf16_to_utf8(str, (utf8 *) result.Data, &result.Count);
    result.Length = utf8_length(result.Data, result.Count);

    return result;
}
}  // namespace internal

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

void *try_heap_realloc(void *ptr, s64 newSize, bool *reportError) {
    void *result = null;
    __try {
        result = HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY | HEAP_GENERATE_EXCEPTIONS, ptr, newSize);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // We specify HEAP_REALLOC_IN_PLACE_ONLY, so STATUS_NO_MEMORY is a valid error.
        // We don't need to report it.
        *reportError = GetExceptionCode() != STATUS_NO_MEMORY;
    }
    return result;
}

export {
    void *os_allocate_block(s64 size) {
        assert(size < MAX_ALLOCATION_REQUEST);
        return HeapAlloc(GetProcessHeap(), 0, size);
    }

    void *os_resize_block(void *ptr, s64 newSize) {
        assert(ptr);
        assert(newSize < MAX_ALLOCATION_REQUEST);

        s64 oldSize = os_get_block_size(ptr);
        if (newSize == 0) newSize = 1;

        bool reportError = false;
        void *result = try_heap_realloc(ptr, newSize, &reportError);

        if (result) return result;

        // If a failure to contract was caused by platform limitations, just return the original block
        if (newSize < oldSize && !is_contraction_possible(oldSize)) return ptr;

        if (reportError) {
            windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "HeapReAlloc");
        }
        return null;
    }

    s64 os_get_block_size(void *ptr) {
        s64 result = HeapSize(GetProcessHeap(), 0, ptr);
        if (result == -1) {
            windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "HeapSize");
            return 0;
        }
        return result;
    }

#define CREATE_MAPPING_CHECKED(handleName, call)                                                               \
    HANDLE handleName = call;                                                                                  \
    if (!handleName) {                                                                                         \
        string extendedCallSite = sprint("{}\n        (the name was: {!YELLOW}\"{}\"{!GRAY})\n", #call, name); \
        char *cStr = string_to_c_string(extendedCallSite);                                                     \
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), cStr);                                \
        free(cStr);                                                                                            \
        free(extendedCallSite);                                                                                \
        return;                                                                                                \
    }

    void os_write_shared_block(const string &name, void *data, s64 size) {
        utf16 *name16 = internal::platform_utf8_to_utf16(name, S->PersistentAlloc);
        defer(free(name16));

        CREATE_MAPPING_CHECKED(h, CreateFileMappingW(INVALID_HANDLE_VALUE, null, PAGE_READWRITE, 0, (DWORD) size, name16));
        defer(CloseHandle(h));

        void *result = MapViewOfFile(h, FILE_MAP_WRITE, 0, 0, size);
        if (!result) {
            windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "MapViewOfFile");
            return;
        }
        copy_memory(result, data, size);
        UnmapViewOfFile(result);
    }

    void os_read_shared_block(const string &name, void *out, s64 size) {
        utf16 *name16 = internal::platform_utf8_to_utf16(name, S->PersistentAlloc);
        defer(free(name16));

        CREATE_MAPPING_CHECKED(h, OpenFileMappingW(FILE_MAP_READ, false, name16));
        defer(CloseHandle(h));

        void *result = MapViewOfFile(h, FILE_MAP_READ, 0, 0, size);
        if (!result) {
            windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "MapViewOfFile");
            return;
        }

        copy_memory(out, result, size);
        UnmapViewOfFile(result);
    }

    void os_free_block(void *ptr) {
        WIN_CHECKBOOL(HeapFree(GetProcessHeap(), 0, ptr));
    }
}

LSTD_END_NAMESPACE
