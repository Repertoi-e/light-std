module;

#include "../common.h"

export module lstd.stack_array;

export import lstd.array;
export import lstd.qsort;

LSTD_BEGIN_NAMESPACE

//
// A wrapper around T arr[..] which makes it easier to pass around and work with.
//
// To make an array from a list of elements use:
//
//  auto arr1 = make_stack_array(1, 4, 9);
//  auto arr2 = make_stack_array<s64>(1, 4, 9);
//
// To iterate:
// For(arr1) {
//     ...
// }
//
// For(range(arr1.Count)) {
//     T element = arr1[it];
//     ...
// }
//
// Different from array<T>, because the latter supports dynamic allocation.
// This object contains no other member than T Data[N], _Count_ is a static member for the given type and doesn't take space,
// which means that sizeof(stack_array<T, N>) == sizeof(T) * N.
//
// :CodeReusability: This is considered array_like (take a look at array_like.h)
export {
    template <typename T, s64 N>
    struct stack_array {
        T Data[N ? N : 1];
        static constexpr s64 Count = N;

        constexpr T &operator[](s64 index) { return Data[translate_index(index, Count)]; }

        constexpr operator array<T>() { return array<T>(Data, Count); }
    };

    // types::is_same_template wouldn't work because stack_array contains a s64 (and not a type) as a second template parameter.
    // At this point I hate C++
    template <typename>
    constexpr bool is_stack_array = false;

    template <typename T, s64 N>
    constexpr bool is_stack_array<stack_array<T, N>> = true;

    template <typename T>
    concept any_stack_array = is_stack_array<types::remove_cv_t<T>>;

    // @Cleanup Actually remove == operator overload for strings and arrays.
    // May be error-prone due to pointers...
    constexpr bool operator==(any_array auto a, any_stack_array auto b) { return compare(a, b) == -1; }
    constexpr bool operator==(any_stack_array auto a, any_stack_array auto b) { return compare(a, b) == -1; }

    // To make range based for loops work.
    auto begin(any_stack_array auto &arr) { return arr.Data; }
    auto end(any_stack_array auto &arr) { return arr.Data + arr.Count; }

    template <typename D = void, typename... Types>
    constexpr stack_array<types::common_type_t<Types...>, sizeof...(Types)> make_stack_array(Types && ...t) { return {(Types &&) t...}; }
}

LSTD_END_NAMESPACE
