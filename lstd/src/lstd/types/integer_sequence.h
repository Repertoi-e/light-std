#pragma once

#include "basic_types.h"

LSTD_BEGIN_NAMESPACE

template <typename T_, T_... Ints>
struct integer_sequence {
    using T = T_;
    static constexpr s64 SIZE = sizeof...(Ints);
};

template <typename T, s64 N, typename IndexSeq>
struct make_integer_sequence_impl;

template <typename T, s64 N, s64... Is>
struct make_integer_sequence_impl<T, N, integer_sequence<T, Is...>> {
    using type = typename make_integer_sequence_impl<T, N - 1, integer_sequence<T, N - 1, Is...>>::type;
};

template <typename T, s64... Is>
struct make_integer_sequence_impl<T, 0, integer_sequence<T, Is...>> {
    using type = integer_sequence<T, Is...>;
};

template <s64... Is>
using index_sequence = integer_sequence<s64, Is...>;

template <typename T, s64 N>
using make_integer_sequence = typename make_integer_sequence_impl<T, N, integer_sequence<T>>::type;

template <s64 N>
using make_index_sequence = typename make_integer_sequence_impl<s64, N, integer_sequence<s64>>::type;

template <typename IS1, typename IS2>
struct merge_integer_sequence;

template <typename T, T... Indices1, T... Indices2>
struct merge_integer_sequence<integer_sequence<T, Indices1...>, integer_sequence<T, Indices2...>> {
    using type = integer_sequence<T, Indices1..., Indices2...>;
};

template <typename IS1, typename IS2>
struct merge_index_sequence;

template <s64... Indices1, s64... Indices2>
struct merge_index_sequence<index_sequence<Indices1...>, index_sequence<Indices2...>> {
    using type = index_sequence<Indices1..., Indices2...>;
};

template <typename IS>
struct reverse_integer_sequence;

template <typename T, T Head, T... Indices>
struct reverse_integer_sequence<integer_sequence<T, Head, Indices...>> {
    using type =
        typename merge_integer_sequence<typename reverse_integer_sequence<integer_sequence<T, Indices...>>::type,
                                        integer_sequence<T, Head>>::type;
};

template <typename IS>
struct reverse_index_sequence;

template <s64 Head, s64... Indices>
struct reverse_index_sequence<index_sequence<Head, Indices...>> {
    using type = typename merge_index_sequence<typename reverse_index_sequence<index_sequence<Indices...>>::type,
                                               index_sequence<Head>>::type;
};

LSTD_END_NAMESPACE
