#pragma once

#include "memory.hpp"

// Manages a block of memory (and provides a way to expand it)
// At the beginning it uses a stack array of a given size
// and when it runs out of space, it allocates dynamically.
//
// StackElements - the amount of elements of type T stored on the stack
//                 before dynamically allocating memory
//
// You can provide a custom deleter (the default is operator delete)
template <typename T, size_t StackElements>
struct stack_dynamic_memory {
    using element_t = T;
    using deleter_t = void (*)(element_t *);

    constexpr static size_t STACK_ELEMENTS = StackElements;
    constexpr static size_t STACK_SIZE = STACK_ELEMENTS * sizeof(element_t);

    static void default_deleter(element_t *p) {
        p->~element_t();
        delete p;
    }

   private:
    element_t _StackData[STACK_ELEMENTS]{};
    element_t *_Pointer = _StackData;

    deleter_t _Deleter = default_deleter;

   public:
    size_t Reserved = 0;
    allocator_closure Allocator;

    stack_dynamic_memory() = default;
    explicit stack_dynamic_memory(const element_t *p, size_t elements) {
        if (!p) return;

        if (elements > STACK_ELEMENTS) {
            Reserved = elements;
            _Pointer = new (&Allocator, ensure_allocator) element_t[elements];
        }
        if (elements) copy_elements(_Pointer, p, elements);
    }

    explicit stack_dynamic_memory(const element_t *p, size_t elements, deleter_t deleter)
        : _Deleter(deleter), stack_dynamic_memory(p, elements) {}

    stack_dynamic_memory(const stack_dynamic_memory &other) {
        if (other.is_dynamic()) {
            Reserved = other.Reserved;
            _Pointer = new (&Allocator, ensure_allocator) element_t[Reserved];
        }
        if (other._Pointer) {
            copy_elements(_Pointer, other._Pointer, other.is_dynamic() ? Reserved : STACK_ELEMENTS);
        }
    }
    stack_dynamic_memory(stack_dynamic_memory &&other) { other.swap(*this); }

    ~stack_dynamic_memory() { release(); }

    void release() {
        if (is_dynamic()) {
            _Deleter(_Pointer);
            _Pointer = _StackData;
        }
    }

    stack_dynamic_memory &operator=(const stack_dynamic_memory &other) {
        release();

        stack_dynamic_memory(other).swap(*this);
        return *this;
    }

    stack_dynamic_memory &operator=(stack_dynamic_memory &&other) {
        release();

        stack_dynamic_memory(std::move(other)).swap(*this);
        return *this;
    }

    void swap(stack_dynamic_memory &other) {
        if (is_dynamic() && other.is_dynamic()) {
            std::swap(_Pointer, other._Pointer);
        } else {
            For(range(STACK_SIZE)) {
                auto temp = std::move(_StackData[it]);
                _StackData[it] = std::move(other._StackData[it]);
                other._StackData[it] = std::move(temp);
            }

            if (!is_dynamic()) {
                auto temp = other._Pointer;
                other._Pointer = other._StackData;
                _Pointer = temp;
            }
            if (!other.is_dynamic()) {
                auto temp = _Pointer;
                _Pointer = _StackData;
                other._Pointer = temp;
            }
        }
        std::swap(_Deleter, other._Deleter);

        std::swap(Reserved, other.Reserved);
        std::swap(Allocator, other.Allocator);
    }

    // Grow so there are at least space for the specified number of elements
    void grow(size_t elements) { reserve((is_dynamic() ? Reserved : STACK_ELEMENTS) + elements); }

    // Reserve a total given amount of elements
    // (grow() is relative, this is absolute)
    void reserve(size_t elements) {
        size_t currentElements = is_dynamic() ? Reserved : STACK_ELEMENTS;

        if (!is_dynamic()) {
            // Return if there is enough space
            if (elements <= STACK_ELEMENTS) return;

            // If we use the stack but we need more size,
            // it's time to convert to dynamically allocated memory.
            _Pointer = new (&Allocator, ensure_allocator) element_t[elements];
            copy_elements(_Pointer, _StackData, currentElements);
            Reserved = elements;
        } else {
            // Return if there is enough space
            if (elements <= currentElements) return;

            _Pointer = resize(_Pointer, elements);
            Reserved = elements;
        }
    }

    bool is_dynamic() const { return (_Pointer != _StackData) && Reserved; }

    operator bool() const { return _Pointer; }

    template <typename U = element_t>
    std::enable_if_t<!std::is_same_v<U, void>, U &> operator*() const {
        assert(_Pointer);
        return *_Pointer;
    }

    element_t *operator->() const { return _Pointer; }
    element_t *get() const { return _Pointer; }

    element_t &operator[](size_t index) { return _Pointer[index]; }
    element_t operator[](size_t index) const { return _Pointer[index]; }
};
