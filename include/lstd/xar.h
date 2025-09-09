#pragma once

#include "memory.h"
#include "bits.h"
#include "stack_array.h"

LSTD_BEGIN_NAMESPACE

//
// Andrew Reece's Exponential Array
// azmr.uk
//
// By default base shift is 3, meaning that the first chunk can hold 8 elements.
// Holding 30 chunks means that we can hold up to 8 * 2^29 = 4,294,967,296 elements (4.2 billion).
template <typename T, usize _N = 30, usize _BASE_SHIFT = 3, bool _STACK_FIRST = false>
struct exponential_array {
    static constexpr usize N = _N;
    static constexpr usize BASE_SHIFT = _BASE_SHIFT;
    static constexpr bool STACK_FIRST = _STACK_FIRST;
    
    T *Chunks[N] = {}; // If STACK_FIRST is true, Chunks[0] is always null, but we access the first chunk via FirstChunk.
    
    // Optional first chunk stored inline (on the stack) for 
    // zero-allocation small arrays, extra chunks going out on the heap.
    stack_array<T, _STACK_FIRST ? (1u << _BASE_SHIFT) : 0> FirstChunk = {};
    usize Count = 0;

    inline T *get_chunk_ptr(usize chunkIndex) {
        if constexpr (STACK_FIRST) {
            if (chunkIndex == 0) return FirstChunk.Data;
        }
        return Chunks[chunkIndex];
    }

    T& get(s64 index) {
        index = translate_negative_index(index, Count);
#if defined LSTD_ARRAY_BOUNDS_CHECK
        assert(index >= 0 && index < Count && "Index out of bounds");
#endif
        usize chunk_i = index;
        usize chunk_cap = 1 << BASE_SHIFT;
        usize chunks_i = 0;
        usize i_shift = index >> BASE_SHIFT;
        if (i_shift > 0) {
            chunks_i = msb(i_shift);
            chunk_cap = 1 << (chunks_i + BASE_SHIFT);
            chunk_i -= chunk_cap;
            chunks_i++;
        }
        return get_chunk_ptr(chunks_i)[chunk_i];
    }

    T ref operator[](s64 index) { return get(index); }
    T no_copy operator[](s64 index) const { return get(index); }
};

template <typename>
const bool is_xar = false;

template <typename T, usize N, usize BASE_SHIFT, bool STACK_FIRST>
const bool is_xar<exponential_array<T, N, BASE_SHIFT, STACK_FIRST>> = true;

template <typename T>
concept any_xar = is_xar<T>;

void reserve(any_xar auto ref arr, usize newSize, allocator alloc = {}) {
    if (newSize <= arr.Count) return;
    
    using ArrT = remove_cvref_t<decltype(arr)>;
    const usize base_size = 1u << arr.BASE_SHIFT;
    usize current_capacity = 0;
    usize next_index = 0; // first chunk index not yet accounted for

    // Account for inline first chunk 
    if constexpr (ArrT::STACK_FIRST) {
        current_capacity += base_size;
        next_index = 1;
    } else {
        if (arr.Chunks[0] != null) { current_capacity += base_size; next_index = 1; }
    }
    // Account for contiguous allocated chunks starting from index 1
    if (next_index <= 1 && arr.N > 1 && arr.Chunks[1] != null) { current_capacity += base_size; next_index = 2; }
    for (usize i = 2; i < arr.N; ++i) {
        if (arr.Chunks[i] == null) break;
        current_capacity += (1u << (arr.BASE_SHIFT + i - 1));
        next_index = i + 1;
    }
    
    if (newSize <= current_capacity) return;
    
    // Grow until capacity >= newSize, starting from the first missing chunk
    for (usize i = next_index; i < arr.N && current_capacity < newSize; ++i) {
        const usize chunk_size = (i == 0 || i == 1) ? base_size : (1u << (arr.BASE_SHIFT + i - 1));

        if (i == 0) {
            if constexpr (!ArrT::STACK_FIRST) {
                if (arr.Chunks[0] == null) {
                    using ElemT = remove_cvref_t<decltype(arr[0])>;
                    arr.Chunks[0] = malloc<ElemT>({.Count = (s64)chunk_size, .Alloc = alloc});
                }
            }
        } else {
            if (arr.Chunks[i] == null) {
                using ElemT = remove_cvref_t<decltype(arr[0])>;
                arr.Chunks[i] = malloc<ElemT>({.Count = (s64)chunk_size, .Alloc = alloc});
            }
        }
        current_capacity += chunk_size;
    }
}

template <typename T, usize N, usize B, bool S>
void add(exponential_array<T, N, B, S> ref arr, T no_copy element) {
    reserve(arr, arr.Count + 1);
    arr[arr.Count++] = element;
}

void free(any_xar auto ref arr) {
    For(range(arr.N)) {
        if (arr.Chunks[it] != null) {
            free(arr.Chunks[it]);
            arr.Chunks[it] = null;
        }
    }
    arr.Count = 0;
}

// Visit chunks in the exponential array, calling the visitor with (chunk_data, chunk_size, chunk_index)
// The visitor should return true to continue, false to stop iteration
void exponential_array_visit_chunks(any_xar auto ref arr, auto visitor) {
    usize processed = 0;
    for (usize chunk_i = 0; chunk_i < arr.N && processed < arr.Count; ++chunk_i) {
    const usize chunk_size = (chunk_i == 0 || chunk_i == 1)
                   ? (1u << arr.BASE_SHIFT)
                   : (1u << (arr.BASE_SHIFT + chunk_i - 1));
        auto *ptr = arr.get_chunk_ptr(chunk_i);
        if (ptr == null) break; // no more allocated chunks
        const usize elements_in_chunk = min(chunk_size, arr.Count - processed);
        if (!visitor(ptr, elements_in_chunk, chunk_i)) break;
        processed += elements_in_chunk;
    }
}

LSTD_END_NAMESPACE