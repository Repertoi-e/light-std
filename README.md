# cpp-utils
A performance-oriented C++17 library for general programming that attempts to mimic Jai's context and allocators and provides common data structures that work with them.

The philosophy behind the API is: concise code, less typing, as/or more expresive, more Python-ish:
```cpp
string a = "HELLO";
for (auto ch : a) {
    ch = to_lower(ch);
}
```
```cpp
string a = "Hello, world!";
// Substrings don't cause allocations! 
string_view b = a(-6, -1); // world
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
assert(integers == to_array<s32>(0, 1, 2, 3, 4));
```
*More examples when the library is in a more solid state and API is finalized!*

Currently the library is written with only x64 architecture in mind.
