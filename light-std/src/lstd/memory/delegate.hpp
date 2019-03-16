#pragma once

#include "../common.hpp"

#include "shared_memory.hpp"

LSTD_BEGIN_NAMESPACE

template <typename T>
struct Delegate;

template <typename R, typename... A>
struct Delegate<R(A...)> {
    using stub_t = R (*)(void *, A &&...);
    using return_t = R;

    void *ObjectPtr = null;
    stub_t StubPtr = null;

    Shared_Memory<void> Store;
    size_t StoreSize = 0;

    using destructor_caller_t = void (*)(void *);
    destructor_caller_t DestructorCaller = null;

    template <class T>
    static void functor_deleter_stub(void *p) {
        ((T *) p)->~T();
        operator delete(p);
    }

    template <class T>
    static void destructor_caller_stub(void *p) {
        ((T *) p)->~T();
    }

    Delegate() = default;
    Delegate(const Delegate &) = default;
    Delegate(Delegate &&) = default;

    Delegate(nullptr_t) : Delegate() {}

    template <typename C, typename = typename std::enable_if<std::is_class<C>{}>::type>
    explicit Delegate(const C *o) : ObjectPtr((C *) o) {}

    template <typename C, typename = typename std::enable_if<std::is_class<C>{}>::type>
    explicit Delegate(const C &o) : ObjectPtr((C *) &o) {}

    template <typename C>
    Delegate(C *objectPtr, R (C::*const method_ptr)(A...)) {
        *this = from(objectPtr, method_ptr);
    }

    template <typename C>
    Delegate(C *objectPtr, R (C::*const method_ptr)(A...) const) {
        *this = from(objectPtr, method_ptr);
    }

    template <typename C>
    Delegate(C &object, R (C::*const method_ptr)(A...)) {
        *this = from(object, method_ptr);
    }

    template <typename C>
    Delegate(const C &object, R (C::*const method_ptr)(A...) const) {
        *this = from(object, method_ptr);
    }

    template <typename T, typename = typename std::enable_if<!std::is_same<Delegate, typename std::decay_t<T>>{}>::type>
    Delegate(T &&f) {
        using functor_type = typename std::decay_t<T>;

        StoreSize = sizeof(functor_type);
        Store = Shared_Memory<void>(operator new(StoreSize), functor_deleter_stub<functor_type>);

        ObjectPtr = Store.get();
        new (ObjectPtr) functor_type(std::forward<T>(f));

        StubPtr = functor_stub<functor_type>;
        DestructorCaller = destructor_caller_stub<functor_type>;
    }

    Delegate &operator=(const Delegate &other) = default;
    Delegate &operator=(Delegate &&) = default;

    template <typename C>
    Delegate &operator=(R (C::*const rhs)(A...)) {
        return *this = from((C *) ObjectPtr, rhs);
    }

    template <typename C>
    Delegate &operator=(R (C::*const rhs)(A...) const) {
        return *this = from((const C *) ObjectPtr, rhs);
    }

    template <typename T, typename = typename std::enable_if_t<!std::is_same<Delegate, typename std::decay_t<T>>{}>>
    Delegate &operator=(T &&f) {
        using functor_type = typename std::decay_t<T>;

        size_t requiredSize = sizeof(functor_type);
        if (requiredSize > StoreSize || !Store.unique()) {
            StoreSize = requiredSize;
            Store.reset(operator new(StoreSize), functor_deleter_stub<functor_type>);
        } else {
            DestructorCaller(Store.get());
        }

        ObjectPtr = Store;
        new (ObjectPtr) functor_type(std::forward<T>(f));

        StubPtr = functor_stub<functor_type>;
        DestructorCaller = destructor_caller_stub<functor_type>;

        return *this;
    }

    template <R (*const function_ptr)(A...)>
    static Delegate from() {
        return {null, function_stub<function_ptr>};
    }

    template <typename C, R (C::*const method_ptr)(A...)>
    static Delegate from(C *objectPtr) {
        return {objectPtr, method_stub<C, method_ptr>};
    }

    template <typename C, R (C::*const method_ptr)(A...) const>
    static Delegate from(const C *objectPtr) {
        return {const_cast<C *>(objectPtr), const_method_stub<C, method_ptr>};
    }

    template <typename C, R (C::*const method_ptr)(A...)>
    static Delegate from(C &object) {
        return {&object, method_stub<C, method_ptr>};
    }

    template <typename C, R (C::*const method_ptr)(A...) const>
    static Delegate from(const C &object) {
        return {(C *) &object, const_method_stub<C, method_ptr>};
    }

    template <typename T>
    static Delegate from(T &&f) {
        return std::forward<T>(f);
    }

    static Delegate from(R (*function_ptr)(A...)) { return function_ptr; }

    template <typename C>
    using member_pair = std::pair<C *, R (C::*const)(A...)>;

    template <typename C>
    using const_member_pair = std::pair<const C *, R (C::*const)(A...) const>;

    template <typename C>
    static Delegate from(C *objectPtr, R (C::*const method_ptr)(A...)) {
        return member_pair<C>(objectPtr, method_ptr);
    }

    template <typename C>
    static Delegate from(const C *objectPtr, R (C::*const method_ptr)(A...) const) {
        return const_member_pair<C>(objectPtr, method_ptr);
    }

    template <typename C>
    static Delegate from(C &object, R (C::*const method_ptr)(A...)) {
        return member_pair<C>(&object, method_ptr);
    }

    template <typename C>
    static Delegate from(const C &object, R (C::*const method_ptr)(A...) const) {
        return const_member_pair<C>(&object, method_ptr);
    }

    void release() {
        StubPtr = null;
        Store.reset();
    }

    void swap(Delegate &other) { std::swap(*this, other); }

    bool operator==(const Delegate &rhs) const { return (ObjectPtr == rhs.ObjectPtr) && (StubPtr == rhs.StubPtr); }
    bool operator!=(const Delegate &rhs) const { return !operator==(rhs); }
    bool operator<(const Delegate &rhs) const {
        return (ObjectPtr < rhs.ObjectPtr) || ((ObjectPtr == rhs.ObjectPtr) && (StubPtr < rhs.StubPtr));
    }

    bool operator==(nullptr_t) const { return !StubPtr; }
    bool operator!=(nullptr_t) const { return StubPtr; }
    explicit operator bool() const { return StubPtr; }

    R operator()(A... args) const { return StubPtr(ObjectPtr, std::forward<A>(args)...); }

   private:
    Delegate(void *o, stub_t m) : ObjectPtr(o), StubPtr(m) {}

    template <R (*function_ptr)(A...)>
    static R function_stub(void *, A &&... args) {
        return function_ptr(std::forward<A>(args)...);
    }

    template <typename C, R (C::*method_ptr)(A...)>
    static R method_stub(void *objectPtr, A &&... args) {
        return (((C *) objectPtr)->*method_ptr)(std::forward<A>(args)...);
    }

    template <typename C, R (C::*method_ptr)(A...) const>
    static R const_method_stub(void *objectPtr, A &&... args) {
        return (((const C *) objectPtr)->*method_ptr)(std::forward<A>(args)...);
    }

    template <typename>
    struct is_member_pair : std::false_type {};

    template <typename C>
    struct is_member_pair<std::pair<C *, R (C::*const)(A...)>> : std::true_type {};

    template <typename>
    struct is_const_member_pair : std::false_type {};

    template <typename C>
    struct is_const_member_pair<std::pair<const C *, R (C::*const)(A...) const>> : std::true_type {};

    template <typename T>
    static typename std::enable_if_t<!(is_member_pair<T>::value || is_const_member_pair<T>::value), R> functor_stub(
        void *objectPtr, A &&... args) {
        return (*static_cast<T *>(objectPtr))(std::forward<A>(args)...);
    }

    template <typename T>
    static typename std::enable_if_t<is_member_pair<T>::value || is_const_member_pair<T>::value, R> functor_stub(
        void *objectPtr, A &&... args) {
        return (((T *) objectPtr)->first->*((T *) objectPtr)->second)(std::forward<A>(args)...);
    }

    template <typename T>
    struct Hash;

    friend struct Hash<Delegate>;
};

LSTD_END_NAMESPACE
