#pragma once

#include "../common.hpp"

LSTD_BEGIN_NAMESPACE

template <typename T>
struct Shared_Memory {
    using deleter_t = void (*)(void *);
    using element_t = T;

    static void default_deleter(void *p) {
        ((T *) p)->~T();
        delete p;
    }

    Shared_Memory() {}

    explicit Shared_Memory(T *p) { acquire(p); }
    explicit Shared_Memory(T *p, deleter_t deleter) : Count(Shared_Memory_Count{null, deleter}) { acquire(p); }

    template <class U>
    Shared_Memory(const Shared_Memory<U> &ptr) : Count(ptr.Count) {
        // Must be coherent: no allocation allowed in this path
        assert(ptr.Pointer == null || ptr.ref_count() != 0);
        acquire(static_cast<typename Shared_Memory<T>::element_t *>(ptr.Pointer));
    }

    Shared_Memory(const Shared_Memory &other) : Count(other.Count) {
        // Must be coherent: no allocation allowed in this path
        assert(other.Pointer == null || other.ref_count() != 0);
        acquire(other.Pointer);
    }

    Shared_Memory &operator=(Shared_Memory ptr) {
        swap(ptr);
        return *this;
    }

    ~Shared_Memory() { reset(); }

    void reset() {
        Count.release(Pointer);
        Pointer = null;
    }

    void reset(T *p) {
        assert(p == NULL || Pointer != p);
        reset();
        acquire(p);
    }

    void reset(T *p, deleter_t deleter) {
        assert(p == NULL || Pointer != p);
        reset();
        Count.Deleter = deleter;
        acquire(p);
    }

    void swap(Shared_Memory &other) {
        std::swap(Pointer, other.Pointer);
        Count.swap(other.Count);
    }

    operator bool() const { return ref_count() > 0; }
    bool unique() const { return ref_count() == 1; }
    s32 ref_count() const { return Count.ref_count(); }

    template <typename U = T>
    std::enable_if_t<!std::is_same_v<U, void>, U &> operator*() const {
        assert(Pointer);
        return *Pointer;
    }

    T *operator->() const {
        assert(Pointer);
        return Pointer;
    }

    T *get() const { return Pointer; }

   private:
    struct Shared_Memory_Count {
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

        void swap(Shared_Memory_Count &other) { std::swap(Pn, other.Pn); }
    };

    void acquire(T *p) {
        Count.acquire(p);
        Pointer = p;
    }

    element_t *Pointer = null;
    Shared_Memory_Count Count;
};

LSTD_END_NAMESPACE
