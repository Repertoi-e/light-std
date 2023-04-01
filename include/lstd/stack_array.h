#pragma once

#include "array.h"

LSTD_BEGIN_NAMESPACE

//
// A wrapper around T arr[..] which makes it easier to pass around and work
// with.
//
// To make an array from a list of elements use:
//
//  auto arr1 = make_stack_array(1, 4, 9);
//  auto arr2 = make_stack_array<s64>(1, 4, 9);
//
// or:
//
//  stack_array<s64> arr2 = { 1, 4, 9 };
//
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
// Different from array<T>, because the latter supports dynamic allocation
// and this object contains no other member than T Data[N], _Count_ is a
// static member for the given type and doesn't take space,
// which means that sizeof(stack_array<T, N>) == sizeof(T) * N.
//
// :CodeReusability: This is considered array_like (take a look at
// "array_like.h")
template <typename T, s64 N>
struct stack_array {
  T Data[N ? N : 1]{};
  static const s64 Count = N;

  stack_array() {}
  stack_array(initializer_list<T> list) {
    memcpy(Data, list.begin(), (list.end() - list.begin()) * sizeof(T));
  }

  T &operator[](s64 index) {
    return Data[translate_negative_index(index, Count)];
  }

  operator array<T>() { return array<T>(Data, Count); }
};

template <typename D = void, typename... Types>
stack_array<common_type_t<Types...>, sizeof...(Types)> make_stack_array(
    Types &&...t) {
  return {(Types &&) t...};
}

LSTD_END_NAMESPACE
