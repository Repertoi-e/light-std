
# light-std
A performance-oriented C++17 library for general programming that attempts to mimic Jai's context and allocators and provides common data structures that work with them.

This library is supposed to be a lighter replacement of c/c++'s standard library in general but designed almost entirely differently. 

Anybody who thinks modern C++ is bananas, you've come to the right place! My view on a lot of C++ features is clearly visible in our type policy.
## Type policy

### Aim of this policy
- Dramatically reduce (or keep the same) complexity and code size (both library AND user side!) UNLESS that comes at a cost of run-time overhead

### Rules
- Always provide a default constructor (implicit or by "T() = default")
- Every data member should have the same access control (everything should be public or private or protected)
- No user defined copy/move constructors
- No virtual or overriden functions
- No throwing of exceptions, anywhere

Most of the requirements above are to clasify the type as POD. Templated containers and functions in this library assert that the type is POD. If your type doesn't classify as POD (has bit fields, non-default but parameterless constructor or is a subclass with extended members) you can declare it explictly with `DECLARE_IS_POD(type, true)`
>Declaring your type as POD explictly means you are sure the type CAN be TREATED as POD without bugs.

### "Every data member should have the same access control" (in order to qualify the type as POD)
This also provides freedom saves frustration of your type having a limited API.
> If you really want private members I suggest you prefix them with _.

### "No user defined copy/move constructors":
_This may sound crazy if you have a type that owns memory (how would you deep copy the contents and not just the pointer when you copy the object?)_

This library implements `string` the following way:
- `string` is a struct that contains a pointer to a byte buffer and 2 fields containing precalculated utf8 code unit and code point lengths, as well as a field `Reserved` that contains the number of bytes allocated by that string (default is 0).
- A string may own its allocated memory or it may not, which is determined by encoding the `this` pointer before the byte buffer when the string reserves memory. That way when you shallow copy the string, the `this` pointer is obviously different (because it is a different object) and when the copied string gets destructed it doesn't free the memory (it doesn't own it). Only when the original string gets destructed does the memory get freed and any shallow copies of it are invalidated (they point to freed memory).
- When a string gets contructed from a literal it doesn't allocate memory. `Reserved` is 0 and the object works like a view. 
- When you call modifying methods (like appending, inserting code points, etc.):
  - If the string points to an allocated buffer but from a different object, the method asserts.
  - If the string was constructed from a literal or a byte buffer that wasn't allocated by a different string, the string allocates a buffer, copies the old one and becomes an owner.

_What is described above happens when the object gets copied around (assigned to variables, passed to functions, etc.) In order to copy the string explicitly the following functions are provided:_
-  `clone(T *dest, T src)` is a global function that ensures a deep copy of the argument passed.
> There is a string overload for clone() that deep copies the contents to `dest` (`dest` now has allocated memory and the byte buffer contains the string from `src`).
- `move(T *dest, T *src)` is a global function that transfers ownership.
> The buffer in `src` (iff `src` owns it) is now owned by `dest` (`src` becomes simply a view into `dest`).

> So `move` is cheaper than `clone` when you don't need the old string to remain an owner.

> **Note**: `clone` and `move` work on all types and are the required way to implement functionality normally present in copy/move c-tors.

> **Note**: In c++ the default assignment operator doesn't call the destructor, so assigning to a string that owns a buffer will cause a leak.

Types that manage memory in this library follow similar design to string and helper functions (as well as an example) are provided in `storage/owner_pointers.h`.

### "No virtual or overriden functions":
They bring a slight run-time overhead and aren't really a good design (in most cases they can be avoided). I recommend striving away from a design that requires inheritance and overloading (OOP in such sense in general) but I came up with a possible work-around that is best shown with an example:

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

          void write(string *str) { WriteFunction(this, str); }
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
Exceptions make your code complicated. They are a good way to handle errors in small examples, but don't really help in large programs/projects. You can't be 100% sure what can throw where and when thus you don't really know what your program is doing (you aren't sure it even works 100% of the time).

You should design code in such a way that errors can't occur (or if they do - handle them, not just bail, and when even that is not possible - stop execution).

**_Every type in this library complies with this policy_**

## Examples

Container API is inspired by Rust and Python

### Example usage of data structures:
```cpp
// string uses utf8 encoding
string a = "ЗДРАСТИ";
for (auto ch : a) {
    // Here _a_ is non-const so _ch_ is actually a reference to the code point in the string
    ch = to_lower(ch);
}
assert(a == "здрасти"); 
```
```cpp
// A negative index means "from the end of string""
string a = "Hello, world!";
string b = a(-6, -1);
assert(b == "world");
// Also substrings don't cause allocations, because string doesn't allocate
// unless cloned explicitly (with clone()) or attempted to be modified (by methods like append(), etc.).
```
```cpp
array<s32> integers = {0, 1};
// Python-like range function
for (s32 i : range(2, 5)) {
    integers.add(i);
}
assert(integers == to_array(0, 1, 2, 3, 4));
```

### Example of custom allocations and implicit context:
```cpp
// The temporary allocator is provided by the library, it's essentially 
// a fast arena allocator that's available globally; supports only "free all"
Context.init_temporary_allocator(4_MiB);

byte *memory1 = new byte[150]; // using default allocator (Malloc)

// _Context.Allocator_ is a variable inside the Context struct which can be accessed by anyone
// or changed within a scope with "PUSH_CONTEXT" like so:
PUSH_CONTEXT(Allocator, Context.TemporaryAlloc) {
    // Now using the temporary allocator, because the library overloads operators new/delete.
    // operator new by default uses _Context.Allocator_
    byte *memory2 = new byte[150];

    // Allocates with an explicit allocator even though _Context.Allocator_ is the temporary allocator.
    byte *memory3 = new (Malloc) byte[150];
    delete[] memory3;

    // ...
}

// Delete always calls the allocator the memory was allocated with.
// This information is stored in an allocation header written in memory before every pointer returned by an allocator.
delete[] memory1;
```

_More examples when the library is in a more solid state and API is finalized!_


