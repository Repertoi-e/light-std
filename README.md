# light-std
A performance-oriented C++17 library for general programming that attempts to mimic Jai's context and allocators and provides common data structures that work with them.
This library is supposed to be a lighter replacement of c/c++'s standard library in general but designed almost entirely differently. 

Anybody who thinks modern C++ is bananas, you've come to the right place! 
This library's view on a lot of C++ features is clearly found in our type policy (type as in a declaration of a struct/class).

## User type policy

### Aim of this policy
- Dramatically reduce (or keep the same) complexity and code size (both library AND user side!) UNLESS that comes at a cost of run-time overhead (i.e. slower code)

### Rules
- Always provide a default constructor (implicit or by "T() = default") in order to qualify the type as POD (plain old data)
- Every data member should have the same access control (everything should be public or private or protected) in order to qualify the type as POD
- No user defined copy/move constructors
- No virtual or overriden functions
- No throwing of exceptions, anywhere
- No bit fields in order to qualify the type as POD

Every other type which doesn't comply with one of the above requirements
should not be expected to be handled properly by containers/functions in this library.

### "Every data member should have the same access control":
This also provides freedom to the caller and IF they know exactly what they are doing, 
it saves frustration of your container having a limited API.
This comes at a cost of backwards-compatibility though, so that is something to the thought of.
Another benefit is striving away from getter/setters which is one of the most annoying patterns 
in API design in my opinion (if getting/setting doesn't require any extra code).

### "No user defined copy/move constructors":
A string, for example, may contain allocated memory (when it's not small enough to store on the stack)
On assignment, a string "view" is created (shallow copy of the string). This means that the new string
doesn't own it's memory so the destructor shouldn't deallocate it. To get around this, string stores
it's data in a "shared_memory" (std::shared_ptr in the C++ std).
In order to do a deep copy of the string, a clone() overload is provided.
clone(T) is a global function that is supposed to ensure a deep copy of the argument passed

A type that may contain owned memory is suggested to follow string's design or if it can't - be designed differently.

### "No virtual or overriden functions":
Unfortunately work-arounds may increase user-side code complexity.
A possible work-around is best shown as an example:
*Using virtual functions:*
```cpp
      struct writer {
          virtual void write(string str) { /*may also be pure virtual*/
          }
      };

      struct console_writer : writer {
          void write(string str) override { /*...*/
          }
      };
```

*Work-around:*
```cpp
      struct writer {
          using write_func_t = void(writer *context, string *str);

          static void default_write(writer *context, string *str) {}
          write_func_t *WriteFunction = default_write; /*may also be null by default (simulate pure virtual)*/

          void writer(string *str) { WriteFunction(this, str); }
      };

      struct console_writer : writer {
          static void console_write(writer *context, string *str) {
              auto *consoleWriter = (console_writer *) console_write;
              /*...*/
          }

          console_writer() { WriteFunction = console_write; }
      };
```

### "No throwing of exceptions, anywhere"
Exceptions make your code complicated. They are a good way to handle errors in small examples,
but don't really help in large programs/projects. You can't be 100% sure what can throw where and when
thus you don't really know what your program is doing (you aren't sure it even works 100% of the time).
You should design code in such a way that errors can't occur (or if they do - handle them, not just bail,
and when even that is not possible - stop execution)
For example a read_file() function should return null if it couldn't open the file,
and then the caller can handle that error (try a different file or prompt the user idk)

_Every type in this library complies with the user type policy_

## Examples

Container API is inspired by Rust and Python

### Example usage of data structures:
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

### Example of custom allocations and implicit context:
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

_More examples when the library is in a more solid state and API is finalized!_


