#pragma once

#include "array_like.h"

LSTD_BEGIN_NAMESPACE

//
// This is a basic wrapper around contiguous memory, it contains a typed pointer
// and a size.
//
// Functions on this object allow negative reversed indexing which begins at
// the end of the array, so -1 is the last element -2 the one before that, etc.
// (Python-style)
//
// Note: We have a very fluid philosophy of containers and ownership. We don't
// implement copy constructors or destructors, which means that the programmer
// is totally in control of how the memory gets managed. In order to get a deep
// copy of an array use clone(). See :TypePolicy in "common.h"
//
// This object being just three 64 bit integers can be cheaply and safely passed
// to functions by value without performance concerns and indirection.
// (Remember that the array doesn't "own" it's buffer, it's up to the
// programmer!)
//
// Functionality for dynamic arrays and array views is implemented in
// array_like.h
//
template <typename T>
struct array {
  T *Data = null;
  s64 Count = 0;
  s64 Allocated = 0;

  array() {}

  // This constructs a view (use make_array to copy)
  array(T *data, s64 count) : Data(data), Count(count) {}

  // .. while here we dynamically allocated because of different behavior in
  // Debug, Release. (initializer lists get optimized). Essentially this
  // provides support for short-hand for doing this:
  //
  //	array<int> a = { 1, 2, 3 };
  //
  // Instead of calling   make_array(...)   with pointer and size to stack
  // elements.
  array(initializer_list<T> items) {
    reserve(*this);
    add(*this, items);
  }

  auto operator[](s64 index) {
    return Data[translate_negative_index(index, Count)];
  }
  auto operator[](s64 index) const {
    return Data[translate_negative_index(index, Count)];
  }
};

template <typename T>
mark_as_leak array<T> make_array(T *data, s64 count) {
  array<T> result;
  reserve(result, count);
  add(result, data, count);
  return result;
}

template <typename T>
mark_as_leak array<T> make_array(initializer_list<T> items) {
  return make_array(items.First, items.Last - items.First);
}

// Returns a deep copy of _src_
template <typename T>
mark_as_leak array<T> clone(array<T> no_copy src) {
  return make_array(src.Data, src.Count);
}

template <typename T>
void free(array<T> ref arr) {
  free(arr.Data);
  arr.Count = arr.Allocated = 0;
}

LSTD_END_NAMESPACE
