#include "../test.h"

TEST(stack_array) {
  // Keep the stack_array owner alive while creating a view array
  auto st = make_stack_array(0, 1, 2, 3, 4);
  array<s32> a = st;

  For(range(a.Count)) { assert_eq(a[it], it); }

  assert_true(has(a, 3));
  assert_true(has(a, 4));
  assert_true(has(a, 0));

  assert_false(has(a, 10));
  assert_false(has(a, 20));

  assert_eq(search(a, 3, .Start = -1, .Reversed = true), 3);
  assert_eq(search(a, 4, .Start = -1, .Reversed = true), 4);
  assert_eq(search(a, 0, .Start = -1, .Reversed = true), 0);
  assert_eq(search(a, 3), 3);
  assert_eq(search(a, 4), 4);
  assert_eq(search(a, 0), 0);
}

TEST(array) {
  array<s64> a;
  defer(free(a.Data));

  For(range(10)) { a += {it}; }
  For(range(10)) { assert_eq(a[it], it); }

  insert_at_index(a, 3, -3);
  assert_eq(a, make_stack_array<s64>(0, 1, 2, -3, 3, 4, 5, 6, 7, 8, 9));

  remove_ordered_at_index(a, 4);
  assert_eq(a, make_stack_array<s64>(0, 1, 2, -3, 4, 5, 6, 7, 8, 9));

  s64 count = a.Count;
  For(range(count)) { remove_ordered_at_index(a, -1); }
  assert_eq(a.Count, 0);

  For(range(10)) { insert_at_index(a, 0, it); }
  assert_eq(a, make_stack_array<s64>(9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

  remove_ordered_at_index(a, -1);
  assert_eq(a, make_stack_array<s64>(9, 8, 7, 6, 5, 4, 3, 2, 1));

  remove_ordered_at_index(a, 0);
  assert_eq(a, make_stack_array<s64>(8, 7, 6, 5, 4, 3, 2, 1));

  s64 f = search(a, 9);
  assert_eq(f, -1);
  f = search(a, 8);
  assert_eq(f, 0);
  f = search(a, 1);
  assert_eq(f, 7);
  f = search(a, 3);
  assert_eq(f, 5);
  f = search(a, 5);
  assert_eq(f, 3);
}

TEST(hash_table) {
  hash_table<string, s32> t;
  defer(free(t));

  set(t, "1", 1);
  set(t, "4", 4);
  set(t, "9", 10101);

  assert((void *)search(t, "1").Value);
  assert_eq(*search(t, "1").Value, 1);
  assert((void *)search(t, "4").Value);
  assert_eq(*search(t, "4").Value, 4);
  assert((void *)search(t, "9").Value);
  assert_eq(*search(t, "9").Value, 10101);

  set(t, "9", 20202);
  assert((void *)search(t, "9").Value);
  assert_eq(*search(t, "9").Value, 20202);
  set(t, "9", 9);

  s64 loopIterations = 0;
  for (auto [key, value] : t) {
    string str = sprint("{}", *value);
    assert_eq_str(*key, str);
    free(str);

    ++loopIterations;
  }
  assert_eq(loopIterations, t.Count);

  hash_table<string, s32> empty;
  for (auto [key, value] : empty) {
    (void)key, (void)value;  // Unused variable
    assert(false);
  }
}

TEST(hash_table_clone) {
  hash_table<string, s32> t;
  defer(free(t));

  set(t, "1", 1);
  set(t, "4", 4);
  set(t, "9", 9);

  hash_table<string, s32> copy = clone(t);
  defer(free(copy));

  set(copy, "11", 20);

  s64 loopIterations = 0;
  for (auto [key, value] : t) {
    string str = sprint("{}", *value);
    assert_eq_str(*key, str);
    free(str.Data);

    ++loopIterations;
  }
  assert_eq(loopIterations, t.Count);

  assert_eq(t.Count, 3);
  assert_eq(copy.Count, 4);
}

struct v2 {
  f32 x, y;
};

struct v3 {
  f32 x, y, z;
};

u64 get_hash(v2) {
  return 10;  // Ha-hA
}

TEST(hash_table_alignment) {
  // This test uses SIMD types which require a 16 byte alignment or otherwise
  // crash. It tests if the block allocation in the table handles alignment of
  // key and value arrays.

  hash_table<v2, v3> simdTable;
  resize(simdTable, 0, 16);

  add(simdTable, {1, 2}, {1, 2, 3});
  add(simdTable, {1, 3}, {4, 7, 9});
}

TEST(array_empty_and_views)
{
  array<s32> e;
  assert_eq(e.Count, 0);

  // slice on empty
  auto slice_result = slice(e, 0, 0);
  assert_eq(slice_result.Count, 0);

  // search on empty
  assert_eq(search(e, 42), -1);
}

/*TEST(array_bounds_and_indices)
{
  array<s32> a = make_stack_array(10, 20, 30);
  defer(free(a));

  // begin==end
  auto slice_result = slice(a, 1, 1);
  assert_eq(slice_result.Count, 0);
  
  // negative indexing boundaries
  assert_eq(a[-1], 30);
  assert_eq(a[-a.Count], 10);

  // set using -Count (need to make it owned first)
  reserve(a);
  a[-a.Count] = 999;
  assert_eq(a[0], 999);
}

TEST(array_search_corner_cases)
{
  {
    array<s32> a = make_stack_array(1, 2, 3, 2, 4);
    defer(free(a));
    
    // start beyond length
    assert_eq(search(a, 2, .Start = 10), -1);
    // reversed search from end
    assert_eq(search(a, 2, .Start = -1, .Reversed = true), 3);
    // reversed search from 0 should only find at 0 if match
    assert_eq(search(a, 1, .Start = 0, .Reversed = true), 0);
  }
  {
    // overlapping replace/remove
    array<s32> a = make_stack_array(1, 1, 1, 1);
    defer(free(a));
    
    replace_all(a, make_stack_array<s32, 2>(1, 1), make_stack_array<s32, 1>(2));
    assert_eq(a, make_stack_array(2, 2));

    replace_all(a, make_stack_array(2), make_stack_array(3, 3, 3));
    assert_eq(a, make_stack_array(3, 3, 3, 3, 3, 3));

    remove_all(a, make_stack_array(3, 3));
    assert_eq(a, make_stack_array(3, 3));
  }
}

TEST(array_insert)
{
  array<s32> a = make_stack_array(5);
  defer(free(a));

  insert_at_index(a, 1, 4);
  insert_at_index(a, 0, 1);
  assert_eq(a, make_stack_array(1, 5, 4));

  insert_at_index(a, 3, make_stack_array(2, 3));
  assert_eq(a, make_stack_array(1, 5, 4, 2, 3));

  insert_at_index(a, 0, make_stack_array(0, 0));
  assert_eq(a, make_stack_array(0, 0, 1, 5, 4, 2, 3));

  insert_at_index(a, 2, make_stack_array(-1));
  assert_eq(a, make_stack_array(0, 0, -1, 1, 5, 4, 2, 3));
}

TEST(array_remove)
{
  array<s32> a = make_stack_array(10, 20, 30, 40, 50, 60, 70);
  defer(free(a));

  remove_range(a, -3, a.Count);
  assert_eq(a, make_stack_array(10, 20, 30, 40));
  
  remove_ordered_at_index(a, 1);
  assert_eq(a, make_stack_array(10, 30, 40));
  
  remove_ordered_at_index(a, 1);
  assert_eq(a, make_stack_array(10, 40));
  
  remove_ordered_at_index(a, 0);
  assert_eq(a, make_stack_array(40));
  
  remove_ordered_at_index(a, -1);
  assert_eq(a.Count, 0);

  a = make_stack_array(1, 2, 3, 4, 5);

  remove_range(a, 0, 3);
  assert_eq(a, make_stack_array(4, 5));
}

TEST(array_replace_range)
{
  array<s32> a = make_stack_array(1, 2, 3, 4, 5);
  defer(free(a));

  // Replace middle elements
  replace_range(a, 1, 3, make_stack_array(99, 100));
  assert_eq(a, make_stack_array(1, 99, 100, 4, 5));

  // Replace with empty (removal)
  replace_range(a, 1, 3, make_stack_array<s32>());
  assert_eq(a, make_stack_array(1, 4, 5));

  // Replace at beginning
  replace_range(a, 0, 1, make_stack_array(10, 11, 12));
  assert_eq(a, make_stack_array(10, 11, 12, 4, 5));

  // Replace at end
  replace_range(a, -2, a.Count, make_stack_array(99));
  assert_eq(a, make_stack_array(10, 11, 12, 99));
}

TEST(array_replace_all)
{
  array<s32> a = make_stack_array(1, 2, 1, 2, 3, 1, 2);
  defer(free(a));

  // Replace all occurrences
  replace_all(a, make_stack_array(1, 2), make_stack_array(9));
  assert_eq(a, make_stack_array(9, 9, 3, 9));

  // Replace single element
  a = make_stack_array(5, 5, 5, 3, 5);
  replace_all(a, make_stack_array(5), make_stack_array(7, 8));
  assert_eq(a, make_stack_array(7, 8, 7, 8, 7, 8, 3, 7, 8));

  // Replace with empty (remove all)
  a = make_stack_array(1, 2, 3, 1, 2, 4);
  remove_all(a, make_stack_array<s32, 2>(1, 2));
  assert_eq(a, (make_stack_array<s32, 2>(3, 4)));

  // Replace non-existent pattern
  a = make_stack_array(1, 2, 3, 4);
  replace_all(a, make_stack_array<s32, 2>(9, 9), make_stack_array<s32, 1>(5));
  assert_eq(a, make_stack_array(1, 2, 3, 4));  // unchanged
}

TEST(array_remove_all)
{
  array<s32> a = make_stack_array(1, 2, 1, 3, 1, 4);
  defer(free(a));

  remove_all(a, make_stack_array(1));
  assert_eq(a, make_stack_array(2, 3, 4));

  a = make_stack_array(5, 6, 5, 6, 7, 5, 6);
  remove_all(a, make_stack_array(5, 6));
  assert_eq(a, make_stack_array(7));

  // Remove non-existent element
  a = make_stack_array(1, 2, 3);
  remove_all(a, make_stack_array(999));
  assert_eq(a, make_stack_array(1, 2, 3));  // unchanged

  // Remove from empty array
  array<s32> empty_arr;
  remove_all(empty_arr, make_stack_array(1));
  assert_eq(empty_arr.Count, 0);
}

TEST(array_slice_edge_cases)
{
  array<s32> a = make_stack_array(10, 20, 30, 40, 50);

  // Normal slice
  array<s32> result = slice(a, 1, 4);
  assert_eq(result, make_stack_array(20, 30, 40));

  // Slice with negative indices
  result = slice(a, -3, -1);
  assert_eq(result, make_stack_array(30, 40));

  // Slice beyond bounds (should clamp)
  result = slice(a, 3, 100);
  assert_eq(result, make_stack_array(40, 50));

  // Empty slice
  result = slice(a, 2, 2);
  assert_eq(result.Count, 0);

  // Reversed bounds (should return empty)
  result = slice(a, 3, 1);
  assert_eq(result.Count, 0);

  // Slice entire array
  result = slice(a, 0, a.Count);
  assert_eq(result, a);
}

TEST(array_complex_patterns)
{
  // Test overlapping patterns
  array<s32> a = make_stack_array(1, 1, 1, 1, 1);
  defer(free(a));

  replace_all(a, make_stack_array(1, 1), make_stack_array(2, 2, 2));
  // Should handle overlapping replacements properly

  // Test nested patterns
  a = make_stack_array(1, 2, 1, 2, 1, 2, 3);
  replace_all(a, make_stack_array(1, 2), make_stack_array(9));
  assert_eq(a, make_stack_array(9, 9, 9, 3));

  // Test single element vs multi-element patterns
  a = make_stack_array(5, 5, 5, 5);
  replace_all(a, make_stack_array(5), make_stack_array(1, 2));
  assert_eq(a, make_stack_array(1, 2, 1, 2, 1, 2, 1, 2));
}*/
