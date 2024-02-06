#pragma once

#include "array.h"
#include "atomic.h"
#include "big_integer.h"
#include "bits.h"
#include "common.h"
#include "context.h"
#include "delegate.h"
#include "fmt.h"
#include "hash_table.h"
#include "lstd/math.h"
#include "memory.h"
#include "os.h"
#include "parse.h"
#include "qsort.h"
#include "stack_array.h"
#include "string.h"
#include "string_builder.h"
#include "type_info.h"
#include "variant.h"
#include "writer.h"

/*
 * @Volatile with README.md
 * :TypePolicy:
 * - Keep it simple and data-oriented. Design data to simplify solutions and
 * minimize abstraction layers.
 * - Use `struct` instead of `class`, and keep everything public.
 * - Provide a default constructor that does minimal work.
 * - Avoid copy/move constructors and destructors.
 * - Never throw exceptions. Instead, return multiple values using structured
 * bindings (C++17). They make code complicated. When you can't handle an error
 * and need to exit from a function, return multiple values.
 *     auto [content, success] = path_read_entire_file("data/hello.txt");
 * In general, error conditions (which require returning a status) should be
 * rare. The code should just do the correct stuff. I find that using exceptions
 * leads to this mentality of "giving up and passing the responsibility to
 * handle error cases to the caller". Howoever, that quickly becomes complicated
 * and confidence is lost on what could happen and where. Code in general likes
 * to grow in complexity combinatorially as more functionality is added, if we
 * also give up the linear structure of code by using exceptions then that's a
 * disaster waiting to happen.
 *
 * Example:
 * Arrays are basic wrappers around contiguous memory with three fields (`Data`,
 * `Count`, and `Allocated`). By default, arrays are views. To make them
 * dynamic, call `reserve(arr)` or `make_array(...)`. To allocate and free
 * memory, call `reserve(arr)` and `free(arr)` or use `defer(free(arr))`.
 *
 * `string`s behave like arrays but have different types to avoid conflicts.
 * They take indices to code points (as they are UTF-8 by default) and are not
 * null-terminated. To make a deep copy, use `clone()`: `newPath = clone(path)`.
 * Functions accepting indices allow negative reversed indexing (Python-style)
 * for easy access to elements from the end.
 */
