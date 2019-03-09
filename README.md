# light-std
A performance-oriented C++17 library for general programming that attempts to mimic Jai's context and allocators and provides common data structures that work with them.
This library is supposed to be a lighter replacement of c/c++'s standard library in general. Anybody who thinks modern C++ is bananas, you've come to the right place!

Example of custom allocations and implicit context:
```cpp
// Fast arena allocator that's available globally; supports only "free all"
temporary_storage_init(4_MiB);

byte *memory1 = new byte[200]; // using default allocator (MALLOC)

// (Allocator is a variable inside the Context struct)
PUSH_CONTEXT(Allocator, TEMPORARY_ALLOC) {
    byte *memory2 = new byte[200]; // now using the temporary allocator
    // ... //
}

// Delete always calls the allocator the memory was allocated with
delete[] memory1;
```

Example usage of data structures:
```cpp
string a = "ЗДРАСТИ";
for (auto ch : a) {
    ch = to_lower(ch);
}
// a == "здрасти", (string is fully utf-8 compatible)
```
```cpp
string a = "Hello, world!";
// Negative index' mean from end of string
string_view b = a(-6, -1); // world
// Also substrings don't cause allocations! 
```
```cpp
// Python-like range function
for (s32 i : range(20, 10, -2)) {
    // ...
}
```
```cpp
Dynamic_Array<s32> integers;
for (s32 i : range(5)) {
    integers.add(i);
}
assert(integers == to_array(0, 1, 2, 3, 4));
```
*More examples when the library is in a more solid state and API is finalized!*

The API is inspired by Rust and Python.

