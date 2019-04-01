#pragma once

#include "../context.hpp"

LSTD_BEGIN_NAMESPACE

// Manages a block of memory
// Deletes it when destructing owned_memory
//
// You can provide a custom deleter (the default is operator delete)
template <typename T>
struct owned_memory {
    using element_t = T;
    using deleter_t = void (*)(element_t *);

    static void default_deleter(element_t *p) {
        p->~element_t();
        delete p;
    }

   private:
    element_t *_Pointer = null;
    deleter_t _Deleter = default_deleter;

   public:
    allocator_closure Allocator;

    owned_memory() = default;

    explicit owned_memory(element_t *p) : _Pointer(p) { Allocator = GET_ALLOCATOR(p); }
    explicit owned_memory(element_t *p, deleter_t deleter) : _Pointer(p), _Deleter(deleter) {
        Allocator = GET_ALLOCATOR(p);
    }

    // Does a deep copy using the same allocator _Pointer was allocated with
    owned_memory(const owned_memory &other) {
        release();

        size_t size = GET_SIZE(other._Pointer);

        _Pointer = (element_t *) new (&Allocator, ensure_allocator) byte[size];
        copy_elements(_Pointer, other._Pointer, size / sizeof(element_t));

        _Deleter = other._Deleter;
    }

    owned_memory(owned_memory &&other) { other.swap(*this); }

    owned_memory &operator=(const owned_memory &other) {
        release();

        owned_memory(other).swap(*this);
        return *this;
    }

    owned_memory &operator=(owned_memory &&other) {
        release();

        owned_memory(std::move(other)).swap(*this);
        return *this;
    }

    ~owned_memory() { release(); }

    void release() {
        if (_Pointer) {
            _Deleter(_Pointer);
            _Pointer = null;
        }
        _Deleter = default_deleter;
    }

    void reset(element_t *p) {
        release();
        _Pointer = p;
    }

    void reset(element_t *p, deleter_t deleter) {
        release(p);
        _Deleter = deleter;
    }

    void swap(owned_memory &other) {
        std::swap(_Pointer, other._Pointer);
        std::swap(_Deleter, other._Deleter);
    }

    operator bool() const { return _Pointer; }

    template <typename U = element_t>
    std::enable_if_t<!std::is_same_v<U, void>, U &> operator*() const {
        assert(_Pointer);
        return *_Pointer;
    }

    element_t *operator->() const {
        assert(_Pointer);
        return _Pointer;
    }

    element_t *get() const { return _Pointer; }

    element_t &operator[](size_t index) { return _Pointer[index]; }
    element_t operator[](size_t index) const { return _Pointer[index]; }
};

LSTD_END_NAMESPACE