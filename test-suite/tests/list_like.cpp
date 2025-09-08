#include "../test.h"

// Define simple node types for testing
struct SNode {
  s32 Value;
  SNode *Next;
};

struct DNode {
  s32 Value;
  DNode *Next;
  DNode *Prev;
};

// Minimal formatters for nodes so list views can print values
template<>
struct formatter<SNode> {
  void format(const SNode &n, fmt_context *f) { format_value(n.Value, f); }
};

template<>
struct formatter<DNode> {
  void format(const DNode &n, fmt_context *f) { format_value(n.Value, f); }
};

TEST(slist_basic_ops) {
  SNode n1{1, nullptr};
  SNode n2{2, nullptr};
  SNode n3{3, nullptr};

  SNode *head = nullptr;
  push_front(head, &n2); // [2]
  push_front(head, &n1); // [1,2]
  insert_after(&n2, &n3); // [1,2,3]

  CHECK_WRITE("[1, 2, 3]", "{}", head);
  CHECK_WRITE("<singly_linked_list_like> { count: 3, data: [1, 2, 3] }", "{:#}", (head));

  // pop front
  auto p = pop_front(head); (void)p; // [2,3]
  CHECK_WRITE("[2, 3]", "{}", (head));

  // remove middle
  remove(head, &n2); // [3]
  CHECK_WRITE("[3]", "{}", (head));
}

TEST(dlist_basic_ops) {
  DNode n1{1, nullptr, nullptr};
  DNode n2{2, nullptr, nullptr};
  DNode n3{3, nullptr, nullptr};

  DNode *head = nullptr; DNode *tail = nullptr;
  push_back(head, tail, &n1); // [1]
  push_back(head, tail, &n2); // [1,2]
  insert_after(tail, &n1, &n3); // [1,3,2]

  CHECK_WRITE("[1, 3, 2]", "{}", (head));
  CHECK_WRITE("<doubly_linked_list_like> { count: 3, data: [1, 3, 2] }", "{:#}", (head));

  remove(head, tail, &n3); // [1,2]
  CHECK_WRITE("[1, 2]", "{}", (head));

  push_front(head, tail, &n3); // [3,1,2]
  CHECK_WRITE("[3, 1, 2]", "{}", (head));
}

struct FNode { f64 V; FNode *Next; };
template<> struct formatter<FNode> { void format(const FNode &n, fmt_context *f){ format_value(n.V, f);} };

TEST(list_forwarding_specs_on_nodes) {
    CHECK_WRITE("2.14", "{:2}", 2.14);

  FNode a{3.14159265, nullptr};
  FNode b{2.71828, nullptr};
  a.Next = &b;
  CHECK_WRITE("[3.14159265, 2.71828]", "{}", (&a));
  CHECK_WRITE("[3.14, 2.72]", "{:.2}", (&a)); 
  CHECK_WRITE("[3.142, 2.718]", "{:.3f}", (&a)); 
  CHECK_WRITE("[3.14, 2.72]", "{:.{}}", (&a), 2); 
  CHECK_WRITE("[3.142, 2.718]", "{:.{}f}", (&a), 3);

  // Debug with precision using list formatting
  string dbg = sprint("{:#.3f}", (&a)); defer(free(dbg.Data));
  assert(match_beginning(dbg, "<singly_linked_list_like> { count: 2, data: ["));
  assert(search(dbg, string("3.142")) != -1);
}
