#pragma once

#include "memory.h"

GU_BEGIN_NAMESPACE

template <typename T>
struct Stack {
    struct Node {
        T Data;
        Node *Next;
    };

    Node *Head = 0;

    // The allocator used for allocating new nodes.
    // If we pass a null allocator to a New/Delete wrapper it uses the context's one automatically.
    Allocator_Closure Allocator;

    Stack() { Allocator = CONTEXT_ALLOC; }
};

template <typename T>
void push(Stack<T> &stack, T const &item) {
    if (!stack.Allocator.Function) {
        stack.Allocator = CONTEXT_ALLOC;
    }
    typename Stack<T>::Node *node = New<typename Stack<T>::Node>(stack.Allocator);

    node->Data = item;
    node->Next = stack.Head;
    stack.Head = node;
}

template <typename T>
T pop(Stack<T> &stack) {
    assert(stack.Head);

    T top = stack.Head->Data;
    typename Stack<T>::Node *next = stack.Head->Next;

    Delete<typename Stack<T>::Node>(stack.Head, stack.Allocator);
    stack.Head = next;
    return top;
}

GU_END_NAMESPACE
