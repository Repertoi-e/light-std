module;

#include "../common.h"
#include "../common/type_info.h"

export module lstd.array_like;

//
// :CodeReusability: This file implements:
//  * get, subarray, find, find_not, find_any_of, find_not_any_of, has, compare, compare_lexicographically,
//  and operators <, <=, ==, !=, >=, >
//
// for structures that have members Data and Count - we call these array-likes.
//
// e.g. stack_array, array, string, guid - all have the members Data and Count, so functions
// are automatically generated to all these types using the definition below.
// They also resolve for different (but compatible) types (e.g. comparing stack_array<u8> and array<u8>)
// for which otherwise we need a combinatorial amount of code to support.
//
// Note: For arrays of different data types comparing doesn't reinterpret the values
// but directly returns false. e.g. array<char> is ALWAYS != array<s8> no matter the data.
//
// Your custom types (which aren't explicitly flagged) will also automatically get this treatment.
// You can explicitly disable this with a member "static constexpr bool IS_ARRAY = false;"
//
// Note: We currently expect the members to be named exactly Data and Count which may conflict with other naming styles.
// How should we handle this?
//

LSTD_BEGIN_NAMESPACE

template <typename T>
struct delegate;

template <typename T>
concept array_has_flag = requires(T t) {
    {t.IS_ARRAY};
};

template <typename T>
concept array_flag_false = requires(T t) {
    {t.IS_ARRAY == false};
};

template <typename T>
concept array_has_members = requires(T t) {
    {t.Data};
    {t.Count};
};

template <typename T>
concept is_array_like = array_has_members<T> &&(array_has_flag<T> && !array_flag_false<T> || !array_has_flag<T>);

export {
    // True if the type has _Data_ and _Count_ members (and the optional explicit flag is not false).
    template <typename T>
    concept any_array_like = is_array_like<types::remove_cv_t<T>>;

    // This returns the type of the _Data_ member of an array-like object
    template <any_array_like T>
    using array_data_t = types::remove_pointer_t<decltype(T::Data)>;

    //
    // This function translates an index that may be negative to an actual index.
    // For example 5 maps to 5
    // but -5 maps to length - 5
    //
    // It is used to support Python-like negative indexing.
    //
    // This function checks if the index is in range if LSTD_ARRAY_BOUNDS_CHECK is defined.
    //    In that case if _toleratePastLast_ is true, an index == length is accepted.
    //    This is useful when you are calculating the end index of an exclusive range
    //    and you don't want to trip an out of bounds assert.
    //
    constexpr always_inline s64 translate_index(s64 index, s64 length, bool toleratePastLast = false) {
        if (index < 0) {
            s64 r = length + index;
#if defined LSTD_ARRAY_BOUNDS_CHECK
            assert(r >= 0 && "Out of bounds");
            assert(r < (toleratePastLast ? length + 1 : length) && "Out of bounds");
#endif
            return r;
        } else {
#if LSTD_ARRAY_BOUNDS_CHECK
            assert(index < (toleratePastLast ? length + 1 : length) && "Out of bounds");
#endif
            return index;
        }
    }

    //
    // Search functions for arrays:
    //

    //
    // @Speed Optimize these functions for scalars (bit hacks and vectorization)
    //

    // Find the first occurence of an element which matches the predicate and is after a specified index.
    // Predicate must take a single argument (the current element) and return if it matches.
    template <any_array_like Arr>
    s64 find(Arr ref arr, delegate<bool(array_data_t<Arr> ref)> predicate, s64 start = 0, bool reversed = false) {
        if (!arr.Data || arr.Count == 0) return -1;
        start = translate_index(start, arr.Count);
        For(range(start, reversed ? -1 : arr.Count, reversed ? -1 : 1)) if (predicate(arr.Data[it])) return it;
        return -1;
    }

    // Find the first occurence of an element that is after a specified index, compares elements using ==
    template <any_array_like Arr>
    constexpr s64 find(Arr ref arr, array_data_t<Arr> ref search, s64 start = 0, bool reversed = false) {
        if (!arr.Data || arr.Count == 0) return -1;
        start = translate_index(start, arr.Count);
        For(range(start, reversed ? -1 : arr.Count, reversed ? -1 : 1)) if (arr.Data[it] == search) return it;
        return -1;
    }

    // Find the first occurence of a subarray that is after a specified index, compares elements using ==
    constexpr s64 find(any_array_like auto ref arr, any_array_like auto ref search, s64 start = 0, bool reversed = false) {
        if (!arr.Data || arr.Count == 0) return -1;
        if (!search.Data || search.Count == 0) return -1;
        start = translate_index(start, arr.Count);

        auto searchEnd = search.Data + search.Count;

        For(range(start, reversed ? -1 : arr.Count, reversed ? -1 : 1)) {
            auto progress = search.Data;
            for (auto s = arr.Data + it; progress != searchEnd; ++s, ++progress) {
                if (!(*s == *progress)) break;
            }
            if (progress == searchEnd) return it;
        }
        return -1;
    }

    // Find the first occurence of any element in the specified subarray that is after a specified index
    constexpr s64 find_any_of(any_array_like auto ref arr, any_array_like auto ref allowed, s64 start = 0, bool reversed = false) {
        if (!arr.Data || arr.Count == 0) return -1;
        if (!allowed.Data || allowed.Count == 0) return -1;
        start = translate_index(start, arr.Count);
        For(range(start, reversed ? -1 : arr.Count, reversed ? -1 : 1)) if (has(allowed, arr.Data[it])) return it;
        return -1;
    }

    // Find the first absence of an element that is after a specified index, compares elements using ==
    template <any_array_like Arr>
    constexpr s64 find_not(Arr ref arr, array_data_t<Arr> ref element, s64 start = 0, bool reversed = false) {
        if (!arr.Data || arr.Count == 0) return -1;
        start = translate_index(start, arr.Count);
        For(range(start, reversed ? -1 : arr.Count, reversed ? -1 : 1)) if (!(arr.Data[it] == element)) return it;
        return -1;
    }

    // Find the first absence of any element in the specified subarray that is after a specified index
    constexpr s64 find_not_any_of(any_array_like auto ref arr, any_array_like auto ref banned, s64 start = 0, bool reversed = false) {
        if (!arr.Data || arr.Count == 0) return -1;
        if (!banned.Data || banned.Count == 0) return -1;
        start = translate_index(start, arr.Count);
        For(range(start, reversed ? -1 : arr.Count, reversed ? -1 : 1)) if (!has(banned, arr.Data[it])) return it;
        return -1;
    }

    // Checks if _item_ is contained in the array
    template <any_array_like Arr>
    constexpr bool has(Arr ref arr, array_data_t<Arr> ref item) { return find(arr, item) != -1; }

    //
    // Compare functions for arrays:
    //

    // Compares this array to _arr_ and returns the index of the first element that is different.
    // If the arrays are equal, the returned value is -1.
    // @Speed @TODO A simple memcmp for scalars is sufficient and way faster
    constexpr s64 compare(any_array_like auto ref arr1, any_array_like auto ref arr2) {
        if (!arr1.Count && !arr2.Count) return -1;
        if (!arr1.Count || !arr2.Count) return 0;

        auto *s1   = arr1.Data;
        auto *s2   = arr2.Data;
        auto *end1 = arr1.Data + arr1.Count;
        auto *end2 = arr2.Data + arr2.Count;
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end1 && s2 == end2) return -1;
            if (s1 == end1) return s1 - arr1.Data;
            if (s2 == end2) return s2 - arr2.Data;
        }
        return s1 - arr1.Data;
    }

    // Compares this array to to _arr_ lexicographically.
    // The result is -1 if this array sorts before the other, 0 if they are equal, and +1 otherwise.
    // @Speed @TODO A simple memcmp for scalars is sufficient and way faster
    constexpr s32 compare_lexicographically(any_array_like auto ref arr1, any_array_like auto ref arr2) {
        if (!arr1.Count && !arr2.Count) return 0;
        if (!arr1.Count) return -1;
        if (!arr2.Count) return 1;

        auto *s1   = arr1.Data;
        auto *s2   = arr2.Data;
        auto *end1 = arr1.Data + arr1.Count;
        auto *end2 = arr2.Data + arr2.Count;
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end1 && s2 == end2) return 0;
            if (s1 == end1) return -1;
            if (s2 == end2) return 1;
        }
        return *s1 < *s2 ? -1 : 1;
    }

    constexpr bool arrays_match(any_array_like auto ref arr1, any_array_like auto ref arr2) { return compare(arr1, arr2) == -1; }
}

LSTD_END_NAMESPACE
