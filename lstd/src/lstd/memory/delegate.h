#pragma once

#include "../internal/common.h"

LSTD_BEGIN_NAMESPACE

//
// This file is a modified implementation of a delegate by Vadim Karkhin.
// https://github.com/tarigo/delegate
//
// Here is the license that came with it:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 Vadim Karkhin
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

template <typename T>
struct delegate;

// This is an object which can store a global function, a method (binded to some instance), or a functor / lambda.
// It doesn't allocate any dynamic memory.
template <typename R, typename... A>
struct delegate<R(A...)> {
    using return_t = R;

    template <typename Class, typename Signature>
    struct target {
        Class *InstancePtr;
        Signature FunctionPtr;
    };

    struct default_class;                                          // Unknown default class (undefined)
    using default_function = void (default_class::*)(void);        // Unknown default function (undefined)
    using default_type = target<default_class, default_function>;  // Default target type

    static const s64 TargetSize = sizeof(default_type);  // Size of default target data

    alignas(default_type) char Storage[TargetSize]{};

    using stub_t = R (*)(void *, A &&...);
    alignas(stub_t) stub_t Invoker = null;

    // Invoke static method / free function
    template <nullptr_t, typename Signature>
    static R invoke(void *data, A &&... args) {
        return (*reinterpret_cast<const target<nullptr_t, Signature> *>(data)->FunctionPtr)((A &&)(args)...);
    }

    // Invoke method
    template <typename Class, typename Signature>
    static R invoke(void *data, A &&... args) {
        return (reinterpret_cast<const target<Class, Signature> *>(data)->InstancePtr->*reinterpret_cast<const target<Class, Signature> *>(data)->FunctionPtr)((A &&)(args)...);
    }

    // Invoke function object (functor)
    template <typename Class, nullptr_t>
    static R invoke(void *data, A &&... args) {
        return (*reinterpret_cast<const target<Class, nullptr_t> *>(data)->InstancePtr)((A &&)(args)...);
    }

    delegate() = default;

    // Construct from null
    delegate(nullptr_t) {}

    // Construct delegate with static method / free function
    delegate(R (*function)(A...)) {
        using Signature = decltype(function);

        auto storage = (target<nullptr_t, Signature> *) &Storage[0];
        storage->InstancePtr = null;
        storage->FunctionPtr = function;
        Invoker = &delegate::invoke<null, Signature>;
    }

    // Construct delegate with method
    template <typename Class, typename Signature>
    delegate(Class *object, Signature method) {
        auto storage = (target<Class, Signature> *) &Storage[0];
        storage->InstancePtr = object;
        storage->FunctionPtr = method;
        Invoker = &delegate::invoke<Class, Signature>;
    }

    // Construct delegate with function object (functor) / lambda
    template <typename Class>
    delegate(Class *functor) {
        auto storage = (target<Class, nullptr_t> *) &Storage[0];
        storage->InstancePtr = functor;
        storage->FunctionPtr = null;
        Invoker = &delegate::invoke<Class, null>;
    }

    // Assign null pointer
    delegate &operator=(nullptr_t) {
        zero_memory(Storage, TargetSize);
        Invoker = null;
        return *this;
    }

    // Compare storages
    s32 compare_lexicographically(void *storage) const {
        For(range(TargetSize)) {
            if (Storage[it] < storage[it]) {
                return -1;
            } else if (Storage[it] > storage[it]) {
                return 1;
            }
        }
        return 0;
    }

    bool operator==(const delegate &rhs) const { return compare_lexicographically(&rhs.Storage[0]) == 0; }
    bool operator!=(const delegate &rhs) const { return compare_lexicographically(&rhs.Storage[0]) != 0; }

    bool operator<(const delegate &rhs) const {
        return compare_lexicographically(&rhs.Storage[0]) == -1;
    }

    bool operator<=(const delegate &rhs) const {
        auto result = compare_lexicographically(&rhs.Storage[0]);
        return result == -1 || result == 0;
    }

    bool operator>(const delegate &rhs) const {
        return compare_lexicographically(&rhs.Storage[0]) == 1;
    }

    bool operator>=(const delegate &rhs) const {
        auto result = compare_lexicographically(&rhs.Storage[0]);
        return result == 1 || result == 0;
    }

    bool operator==(nullptr_t) const { return !Invoker; }
    bool operator!=(nullptr_t) const { return Invoker; }

    operator bool() const { return Invoker; }

    // Call delegate
    R operator()(A... args) const {
        return (*Invoker)((void *) &Storage[0], (A &&)(args)...);
    }

   private:
};

LSTD_END_NAMESPACE
