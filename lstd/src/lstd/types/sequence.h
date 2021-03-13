#pragma once

#include "types.h"

//
// This file provides types to work with templates and a sequence of things (most commonly integral types).
//

LSTD_BEGIN_NAMESPACE

template <typename T_, T_... Ints>
struct sequence {
    using T = T_;
    static constexpr s64 SIZE = sizeof...(Ints);
};

template <typename T, s64 N, typename IndexSeq>
struct make_sequence_impl;

template <typename T, s64 N, s64... Is>
struct make_sequence_impl<T, N, sequence<T, Is...>> {
    using type = typename make_sequence_impl<T, N - 1, sequence<T, N - 1, Is...>>::type;
};

template <typename T, s64... Is>
struct make_sequence_impl<T, 0, sequence<T, Is...>> {
    using type = sequence<T, Is...>;
};

template <s64... Is>
using integer_sequence = sequence<s64, Is...>;

template <typename T, s64 N>
using make_sequence = typename make_sequence_impl<T, N, sequence<T>>::type;

template <s64 N>
using make_integer_sequence = typename make_sequence_impl<s64, N, sequence<s64>>::type;

template <typename IS1, typename IS2>
struct merge_sequence;

template <typename T, T... Indices1, T... Indices2>
struct merge_sequence<sequence<T, Indices1...>, sequence<T, Indices2...>> {
    using type = sequence<T, Indices1..., Indices2...>;
};

template <typename IS1, typename IS2>
struct merge_integer_sequence;

template <s64... Indices1, s64... Indices2>
struct merge_integer_sequence<integer_sequence<Indices1...>, integer_sequence<Indices2...>> {
    using type = integer_sequence<Indices1..., Indices2...>;
};

template <typename IS>
struct reverse_sequence;

template <typename T, T Head, T... Indices>
struct reverse_sequence<sequence<T, Head, Indices...>> {
    using type = typename merge_sequence<typename reverse_sequence<sequence<T, Indices...>>::type, sequence<T, Head>>::type;
};

template <typename IS>
struct reverse_integer_sequence;

template <s64 Head, s64... Indices>
struct reverse_integer_sequence<integer_sequence<Head, Indices...>> {
    using type = typename merge_integer_sequence<typename reverse_integer_sequence<integer_sequence<Indices...>>::type, integer_sequence<Head>>::type;
};

LSTD_END_NAMESPACE
