#pragma once

#include "../common.hpp"

// We include this in delegate, which is included in context
// and assert uses the context, which creates a circular dependency.
// Instead, if we assert in this file we call "os_assert_failed"
#if !defined LSTD_ASSERT
#if defined LSTD_DEBUG
#define assert(condition) \
    (!!(condition)) ? (void)0 : os_assert_failed(__FILE__, __LINE__, u8## "Note: Assert called in a file which can't use the context's assert function without creating a circular dependency.\n" ## #condition)
#else
#define assert(condition) ((void) 0)
#endif
#endif

LSTD_BEGIN_NAMESPACE

// Manages a block of memory
// Deletes it when all shared_memory's pointing to it get deleted
// (implements reference counting)
//
// You can provide a custom deleter (the default is operator delete)
template <typename T>
struct shared_memory {
    using element_t = T;
    using deleter_t = void (*)(element_t *);

    static void default_deleter(element_t *p) {
        p->~element_t();
        delete p;
    }

   private:
    element_t *_Pointer = null;

    struct shared_memory_count {
        s32 *Pn = null;
        deleter_t Deleter = default_deleter;

        s32 ref_count() const { return Pn ? *Pn : 0; }

        template <typename U>
        void acquire(U *p) {
            if (p) {
                if (!Pn) {
                    Pn = new s32;
                    *Pn = 1;
                } else {
                    ++(*Pn);
                }
            }
        }

        template <typename U>
        void release(U *p) {
            if (Pn) {
                --(*Pn);
                if (*Pn == 0) {
                    assert(Deleter);
                    Deleter(p);
                    delete Pn;
                }
                Pn = null;
            }
        }

        void swap(shared_memory_count &other) { std::swap(Pn, other.Pn); }
    };
    shared_memory_count Count;

   public:
    shared_memory() = default;

    explicit shared_memory(element_t *p) { acquire(p); }
    explicit shared_memory(element_t *p, deleter_t deleter) : Count(shared_memory_count{null, deleter}) { acquire(p); }

    template <class U>
    explicit shared_memory(const shared_memory<U> &ptr) : Count(ptr.Count) {
        // Must be coherent: no allocation allowed in this path
        assert(ptr.Pointer == null || ptr.ref_count() != 0);
        acquire(static_cast<element_t *>(ptr.Pointer));
    }

    shared_memory(const shared_memory &other) : Count(other.Count) {
        // Must be coherent: no allocation allowed in this path
        assert(other._Pointer == null || other.ref_count() != 0);
        acquire(other._Pointer);
    }

    shared_memory &operator=(shared_memory ptr) {
        swap(ptr);
        return *this;
    }

    ~shared_memory() { release(); }

    void release() {
        Count.release(_Pointer);
        _Pointer = null;
    }

    void reset(element_t *p) {
        assert(p == NULL || _Pointer != p);
        release();
        acquire(p);
    }

    void reset(element_t *p, deleter_t deleter) {
        assert(p == NULL || _Pointer != p);
        release();
        Count.Deleter = deleter;
        acquire(p);
    }

    void swap(shared_memory &other) {
        std::swap(_Pointer, other._Pointer);
        Count.swap(other.Count);
    }

    operator bool() const { return ref_count() > 0; }
    bool is_unique() const { return ref_count() == 1; }
    s32 ref_count() const { return Count.ref_count(); }

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

   private:
    void acquire(element_t *p) {
        Count.acquire(p);
        _Pointer = p;
    }
};

LSTD_END_NAMESPACE
