#pragma once

#include "common.h"

LSTD_BEGIN_NAMESPACE

// Concepts for node-like types with Next/Prev pointers
template <typename T>
concept singly_linked_node_like = requires(T t) {
  { t.Next };
} && is_pointer<decltype(T::Next)> && is_same<remove_pointer_t<decltype(T::Next)>, T>;

template <typename T>
concept doubly_linked_node_like = singly_linked_node_like<T> && requires(T t) {
  { t.Prev };
} && is_pointer<decltype(T::Prev)> && is_same<remove_pointer_t<decltype(T::Prev)>, T>;

// Basic singly-linked list algorithms
template <singly_linked_node_like T>
always_inline void push_front(T *&head, T *node) {
  if (!node) return;
  node->Next = head;
  head = node;
}

template <singly_linked_node_like T>
always_inline void insert_after(T *pos, T *node) {
  if (!pos || !node) return;
  node->Next = pos->Next;
  pos->Next = node;
}

template <singly_linked_node_like T>
always_inline void push_back(T *&head, T *node) {
  if (!node) return;
  node->Next = null;
  if (!head) { head = node; return; }
  T *p = head; while (p->Next) p = p->Next; p->Next = node;
}

// O(1) push_back when caller maintains a tail pointer
template <singly_linked_node_like T>
always_inline void push_back(T *&head, T *&tail, T *node) {
  if (!node) return;
  node->Next = null;
  if (!head) { head = tail = node; return; }
  tail->Next = node; tail = node;
}

template <singly_linked_node_like T>
always_inline T *pop_front(T *&head) {
  if (!head) return null;
  T *n = head; head = head->Next; n->Next = null; return n;
}

template <singly_linked_node_like T>
always_inline void remove(T *&head, T *node) {
  if (!head || !node) return;
  if (head == node) { head = head->Next; node->Next = null; return; }
  T *prev = head;
  while (prev && prev->Next != node) prev = prev->Next;
  if (prev && prev->Next == node) { prev->Next = node->Next; node->Next = null; }
}

template <singly_linked_node_like T>
always_inline s64 length(T *head) {
  s64 n = 0; while (head) { ++n; head = head->Next; } return n;
}

// Basic doubly-linked list algorithms
template <doubly_linked_node_like T>
always_inline void push_front(T *&head, T *&tail, T *node) {
  if (!node) return;
  node->Prev = null; node->Next = head;
  if (head) head->Prev = node; else tail = node;
  head = node;
}

template <doubly_linked_node_like T>
always_inline void push_back(T *&head, T *&tail, T *node) {
  if (!node) return;
  node->Next = null; node->Prev = tail;
  if (tail) tail->Next = node; else head = node;
  tail = node;
}

template <doubly_linked_node_like T>
always_inline void insert_after(T *&tail, T *pos, T *node) {
  if (!pos || !node) return;
  node->Prev = pos; node->Next = pos->Next;
  if (pos->Next) pos->Next->Prev = node; else tail = node;
  pos->Next = node;
}

template <doubly_linked_node_like T>
always_inline void insert_before(T *&head, T *pos, T *node) {
  if (!pos || !node) return;
  node->Next = pos; node->Prev = pos->Prev;
  if (pos->Prev) pos->Prev->Next = node; else head = node;
  pos->Prev = node;
}

template <doubly_linked_node_like T>
always_inline void remove(T *&head, T *&tail, T *node) {
  if (!node) return;
  if (node->Prev) node->Prev->Next = node->Next; else head = node->Next;
  if (node->Next) node->Next->Prev = node->Prev; else tail = node->Prev;
  node->Next = node->Prev = null;
}

template <doubly_linked_node_like T>
always_inline s64 length(T *head) {
  s64 n = 0; while (head) { ++n; head = head->Next; } return n;
}

LSTD_END_NAMESPACE
