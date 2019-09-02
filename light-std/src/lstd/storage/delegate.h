#pragma once

#include "../common.h"

#include "owner_pointers.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
struct delegate;

template <typename R, typename... A>
struct delegate<R(A...)> {
    using stub_t = R (*)(void *, A &&...);
    using return_t = R;

    void *ObjectPtr = null;
    stub_t StubPtr = null;

    char *Store = null;
    size_t StoreSize = 0;

    using destructor_caller_t = void (*)(void *);
    destructor_caller_t DestructorCaller = null;

    template <typename T>
    static void destructor_caller_stub(void *p) {
        ((T *) p)->~T();
    }

    delegate() = default;
    delegate(nullptr_t) {}

    template <typename C, typename = typename enable_if<is_class<C>{}>::type>
    explicit delegate(const C *o) : ObjectPtr((C *) o) {}

    template <typename C, typename = typename enable_if<is_class<C>{}>::type>
    explicit delegate(const C &o) : ObjectPtr((C *) &o) {}

    template <typename C>
    delegate(C *objectPtr, R (C::*const method_ptr)(A...)) {
        *this = from(objectPtr, method_ptr);
    }

    template <typename C>
    delegate(C *objectPtr, R (C::*const method_ptr)(A...) const) {
        *this = from(objectPtr, method_ptr);
    }

    template <typename C>
    delegate(C &object, R (C::*const method_ptr)(A...)) {
        *this = from(object, method_ptr);
    }

    template <typename C>
    delegate(const C &object, R (C::*const method_ptr)(A...) const) {
        *this = from(object, method_ptr);
    }

    template <typename T, typename = typename enable_if<!is_same<delegate, decay_t<T>>{}>::type>
    delegate(T &&f) {
        using functor_type = decay_t<T>;

        StoreSize = sizeof(functor_type);
        ObjectPtr = Store = encode_owner(new char[StoreSize + POINTER_SIZE], this);
        new (ObjectPtr) functor_type((T &&) f);

        StubPtr = functor_stub<functor_type>;
        DestructorCaller = destructor_caller_stub<functor_type>;
    }

    template <typename C>
    delegate &operator=(R (C::*const rhs)(A...)) {
        return *this = from((C *) ObjectPtr, rhs);
    }

    template <typename C>
    delegate &operator=(R (C::*const rhs)(A...) const) {
        return *this = from((const C *) ObjectPtr, rhs);
    }

    template <typename F>
    enable_if_t<!is_same_v<delegate, decay_t<F>>, delegate &> operator=(F &&f) {
        using functor_type = decay_t<F>;

        size_t requiredSize = sizeof(functor_type);
        if (requiredSize > StoreSize) {
            release();

            StoreSize = requiredSize;
            Store = encode_owner(new char[StoreSize + POINTER_SIZE], this);
        }

        ObjectPtr = Store;
        new (ObjectPtr) functor_type((F &&) f);

        StubPtr = functor_stub<functor_type>;
        DestructorCaller = destructor_caller_stub<functor_type>;

        return *this;
    }

    template <R (*const function_ptr)(A...)>
    static delegate from() {
        return {null, function_stub<function_ptr>};
    }

    template <typename C, R (C::*const method_ptr)(A...)>
    static delegate from(C *objectPtr) {
        return {objectPtr, method_stub<C, method_ptr>};
    }

    template <typename C, R (C::*const method_ptr)(A...) const>
    static delegate from(const C *objectPtr) {
        return {const_cast<C *>(objectPtr), const_method_stub<C, method_ptr>};
    }

    template <typename C, R (C::*const method_ptr)(A...)>
    static delegate from(C &object) {
        return {&object, method_stub<C, method_ptr>};
    }

    template <typename C, R (C::*const method_ptr)(A...) const>
    static delegate from(const C &object) {
        return {(C *) &object, const_method_stub<C, method_ptr>};
    }

    template <typename F>
    static delegate from(F &&f) {
        return (F &&) f;
    }

    static delegate from(R (*function_ptr)(A...)) { return function_ptr; }

    template <typename C>
    using member_pair = pair<C *, R (C::*const)(A...)>;

    template <typename C>
    using const_member_pair = pair<const C *, R (C::*const)(A...) const>;

    template <typename C>
    static delegate from(C *objectPtr, R (C::*const method_ptr)(A...)) {
        return member_pair<C>(objectPtr, method_ptr);
    }

    template <typename C>
    static delegate from(const C *objectPtr, R (C::*const method_ptr)(A...) const) {
        return const_member_pair<C>(objectPtr, method_ptr);
    }

    template <typename C>
    static delegate from(C &object, R (C::*const method_ptr)(A...)) {
        return member_pair<C>(&object, method_ptr);
    }

    template <typename C>
    static delegate from(const C &object, R (C::*const method_ptr)(A...) const) {
        return const_member_pair<C>(&object, method_ptr);
    }

    void release() {
        StubPtr = null;
        if (is_owner()) {
            DestructorCaller(ObjectPtr);
            delete[](Store - POINTER_SIZE);
        }
    }

    bool is_owner() const { return StoreSize && decode_owner<delegate>(Store) == this; }

    bool operator==(delegate rhs) const { return (ObjectPtr == rhs.ObjectPtr) && (StubPtr == rhs.StubPtr); }
    bool operator!=(delegate rhs) const { return !operator==(rhs); }
    bool operator<(delegate rhs) const {
        return (ObjectPtr < rhs.ObjectPtr) || ((ObjectPtr == rhs.ObjectPtr) && (StubPtr < rhs.StubPtr));
    }

    bool operator==(nullptr_t) const { return !StubPtr; }
    bool operator!=(nullptr_t) const { return StubPtr; }
    explicit operator bool() const { return StubPtr; }

    R operator()(A... args) const { return StubPtr(ObjectPtr, ((A &&) args)...); }

   private:
    template <R (*function_ptr)(A...)>
    static R function_stub(void *, A &&... args) {
        return function_ptr(((A &&) args)...);
    }

    template <typename C, R (C::*method_ptr)(A...)>
    static R method_stub(void *objectPtr, A &&... args) {
        return (((C *) objectPtr)->*method_ptr)(((A &&) args)...);
    }

    template <typename C, R (C::*method_ptr)(A...) const>
    static R const_method_stub(void *objectPtr, A &&... args) {
        return (((const C *) objectPtr)->*method_ptr)(((A &&) args)...);
    }

    template <typename>
    struct is_member_pair : false_t {};

    template <typename C>
    struct is_member_pair<pair<C *, R (C::*const)(A...)>> : true_t {};

    template <typename>
    struct is_const_member_pair : false_t {};

    template <typename C>
    struct is_const_member_pair<pair<const C *, R (C::*const)(A...) const>> : true_t {};

    template <typename T>
    static enable_if_t<!(is_member_pair<T>::value || is_const_member_pair<T>::value), R> functor_stub(void *objectPtr,
                                                                                                      A &&... args) {
        return (*static_cast<T *>(objectPtr))(((A &&) args)...);
    }

    template <typename T>
    static enable_if_t<is_member_pair<T>::value || is_const_member_pair<T>::value, R> functor_stub(void *objectPtr,
                                                                                                   A &&... args) {
        return (((T *) objectPtr)->First->*((T *) objectPtr)->Second)(((A &&) args)...);
    }
};

template <typename T>
delegate<T> *clone(delegate<T> *dest, delegate<T> src) {
    dest->release();
    *dest = src;
    if (src.StoreSize) {
        dest->Store = encode_owner(new char[src.StoreSize + POINTER_SIZE], dest);
        copy_memory(dest->Store, src.Store, src.StoreSize);
    }
    return dest;
}

template <typename T>
delegate<T> *move(delegate<T> *dest, delegate<T> *src) {
    dest->release();
    *dest = *src;

    if (!src->is_owner()) return;

    // Transfer ownership
    if (src->Store) change_owner(src->Store, dest);
    if (dest->Store) change_owner(dest->Store, dest);
    return dest;
}

LSTD_END_NAMESPACE
