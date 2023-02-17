module;

#include "../common.h"

//
// This is an object which can store a global function, 
// a method (binded to some object instance), or a functor/lambda.
// It doesn't allocate any dynamic memory (contrary to std::function). 
// It does this by keeping a pointer to the functor/lambda,
// which means that the variable needs to outlive the delegate,
// which is not as big of a problem as it may sound.
//
// @TODO: Show example
//

export module lstd.delegate;

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

LSTD_BEGIN_NAMESPACE

export {
    template <typename T>
    struct delegate;

    template <typename R, typename... A>
    struct delegate<R(A...)> {
        using stub_t = R(*)(void*, A no_copy...);
        using return_t = R;

        template <typename Type, typename Signature>
        struct target {
            Type* InstancePtr;
            Signature FunctionPtr;
        };

        struct default_type_;                                              // Unknown default type (undefined)
        using default_function = void (default_type_::*)(void);            // Unknown default function (undefined)
        using default_type = target<default_type_, default_function>;  // Default target type

        static const s64 Count = sizeof(default_type);

        alignas(default_type) byte Data[Count]{};
        alignas(stub_t) stub_t Invoker = null;

        // Invoke static method / free function
        template <null_t, typename Signature>
        static R invoke(void* data, A no_copy... args) {
            return (*((target<null_t, Signature> *) data)->FunctionPtr)(args...);
        }

        // Invoke method
        template <typename Type, typename Signature>
        static R invoke(void* data, A no_copy... args) {
            return (((target<Type, Signature> *) data)->InstancePtr->*((target<Type, Signature> *) data)->FunctionPtr)(args...);
        }

        // Invoke function object (functor)
        template <typename Type, null_t>
        static R invoke(void* data, A no_copy... args) {
            return (*((target<Type, null_t> *) data)->InstancePtr)(args...);
        }

        delegate() {}

        // Construct from null
        delegate(null_t) {}

        // Construct delegate with static method / free function
        delegate(R(*function)(A...)) {
            using Signature = decltype(function);

            auto storage = (target<null_t, Signature> *) & Data[0];
            storage->InstancePtr = null;
            storage->FunctionPtr = function;
            Invoker = &delegate::template invoke<null, Signature>;
        }

        // Construct delegate with method
        template <typename Type, typename Signature>
        delegate(Type* object, Signature method) {
            auto storage = (target<Type, Signature> *) & Data[0];
            storage->InstancePtr = object;
            storage->FunctionPtr = method;
            Invoker = &delegate::template invoke<Type, Signature>;
        }

        // Construct delegate with function object (functor) / lambda
        template <typename Type>
        delegate(Type* functor) {
            auto storage = (target<Type, null_t> *) & Data[0];
            storage->InstancePtr = functor;
            storage->FunctionPtr = null;
            Invoker = &delegate::template invoke<Type, null>;
        }

        // Assign null pointer
        delegate& operator=(null_t) {
            memset0(Data, Count);
            Invoker = null;
            return *this;
        }

        bool operator==(null_t) const { return !Invoker; }
        bool operator!=(null_t) const { return Invoker; }

        explicit operator bool() const { return Invoker; }

        // Call operator
        R operator()(A... args) const {
            return (*Invoker)((void*)&Data[0], args...);
        }
    };
}

LSTD_END_NAMESPACE
