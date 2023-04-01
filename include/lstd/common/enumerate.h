#pragma once

#include "namespace.h"
#include "numeric/numeric.h"
#include "semantic.h"

LSTD_BEGIN_NAMESPACE

//
// This one is inspired from Python's enumerate().
// For each loop which also gives the index of the element.
//
// Example usage:
//
//    For_enumerate(a) {
//        b[it_index] = it + 1;  // Here _it_ is the object in the iterable and
//        _it_index_ is the index.
//    }
//
// .. which is the same as:
//
//    For(range(a.Count)) {
//        b[it] = a[it] + 1;    // Here _it_ is the iterator value of the range
//        between 0 and count.
//    }
//
// You can change the names of the internal
// variables by using _For_enumerate_as_.
//
#define For_enumerate_as(it_index, it, in) \
  for (auto [it_index, it] : LSTD_NAMESPACE::enumerate_impl(in))
#define For_enumerate(in) For_enumerate_as(it_index, it, in)

template <typename T, typename TIter = decltype(T().begin()),
          typename = decltype(T().end())>
auto enumerate_impl(T no_copy in) {
  struct iterator {
    s64 I;
    TIter Iter;

    bool operator!=(iterator no_copy other) const { return Iter != other.Iter; }
    void operator++() { ++I, ++Iter; }

    struct dereference_result {
      s64 Index;
      decltype(*(TIter())) Value;
    };

    auto operator*() const { return dereference_result{I, *Iter}; }
  };

  struct iterable_wrapper {
    T Iterable;

    auto begin() { return iterator{0, Iterable.begin()}; }
    auto end() { return iterator{0, Iterable.end()}; }
  };

  return iterable_wrapper{in};
}

LSTD_END_NAMESPACE
