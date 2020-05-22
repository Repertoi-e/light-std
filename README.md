
# light-std
A C++17 library created for personal use that aims to replace the standard C/C++ library. It's performance-oriented and designed for general programming.
It attempts to mimic some of Jai's (Jonathan Blow's new language) features (implicit context and allocator system)

This library is supposed to be a replacement of C/C++'s standard library in but designed entirely differently. 

Anybody who thinks modern C++ is stupid, you've come to the right place! std:: is way too complicated and messy and a lot of C++'s features add unnecessary complications to the library code.
My view on some of those features and simpler solutions are visible in the type policy.

## Type policy

### Aim of this policy
- Dramatically reduce complexity and code size (both library AND user side!) UNLESS that comes at a run-time cost

### Rules
- Always provide a default constructor (implicit or by "T() = default")
- Every data member should have the same access control (everything should be public or private or protected)
- Strive to make classes/structures/objects (whatever you wanna call them) data oriented.
  Programs work with data. Design your data so it makes the solution straightforward and minimize abstraction layers.
- No user defined copy/move constructors (reduces bloat and forces you to program in a way that's centralized around data, not objects).
- No throwing of exceptions, anywhere

### "No user defined copy/move constructors":
_This may sound crazy if you have a type that owns memory (how would you deep copy the contents and not just the pointer when you copy the object?)_

All allocations in this library contain a header with some information about the allocation. One of the pieces of information is a pointer to the object that owns the memory. That pointer is set manually using functions from `memory/owner_pointers.h`.

So this library implements `string` the following way:
- `string` is a struct that contains a pointer to a byte buffer and 2 fields containing pre-calculated utf8 code unit and code point lengths, as well as a field `Reserved` that contains the number of bytes allocated by that string (default is 0).
- A string may own its allocated memory or it may not, which is determined by encoding the _this_ pointer in the allocation header when the string reserves a buffer. That way when you shallow copy the string, the _this_ pointer is obviously different (because it is a different object) and when the copied string gets destructed it doesn't free the memory (it doesn't own it). Only when the original string gets destructed does the memory get freed and any shallow copies of it are invalidated (they point to freed memory).
- When a string gets contructed from a literal it doesn't allocate memory. `Reserved` is 0 and the object works like a view. 
- If the string was constructed from a literal, shallow copy of another string or a byte buffer, when you call modifying methods (like appending, inserting code points, etc.) the string allocates a buffer, copies the old one and now owns memory.

_What is described above is essentially this. strings by default are "string views" - they don't own memory, they point to existing memory. There are methods which modify the contents of the string, in that case a new string is created that allocates a buffer and frees it when it gets destructed. Copies of that string are, again, views that point to that string's memory (they are super light and do no allocations/deallocations when they get constructed/destroyed). If the first string gets destroyed, all views of it are pointing to freed memory. In order to deep copy the string explicitly the following function is provided:_
-  `clone(T *dest, T src)` is a global function that ensures a deep copy of the argument passed. Objects that own memory (like `string`) overload `clone()` and make sure the copy reserves a buffer and copies. It is the recommended way to deep clone any kind of object in this library (and not by using a copy constructor). Using a function is more explicit and you know when can a copy happen.
- `move(T *dest, T *src)` is a global function that transfers ownership.
> The buffer in `src` (iff `src` owns it) is now owned by `dest` (`src` becomes simply a view into `dest`). So `move` is cheaper than `clone` and is used for example when inserting objects into an array.

> **Note**: In C++ the default assignment operator doesn't call the destructor, so assigning to a string that owns a buffer will cause a leak.

Doing things this way has certain benenits. For example, you can stop thinking about `const &` in function parameters. Code like: `parse(string contents)` doesn't deep copy the string and it's really cheap to pass it as a parameter (string is represented by couple of pointer-sized members). When you start doing this a lot you can appreciate the simplicity in not having to write `const string &` everywhere... 
The same thing applies for `array<T>`, `table<K, V>`, etc. 

It also has some downsides. One thing to really watch out for is that you can't return these kinds of objects as results from functions. For example, 
```cpp
array<token> parse_input(string input) { 
    array<token> result;
    result.append(...);
    // ... implement something .. //
    return result;
}
```
might look right, but when returning, `result` gets shallow-copied and then destroyed at the end of the scope of the function.
The caller receives a view into freed memory.

A corrent way to write such a function might be:
```cpp
void parse_input(array<token> *out, string input) { 
    out->append(...);
    // ... implement something .. //
}
```
You might argue that doesn't look very nice, but in my opinion it's better, because it encourages a certain coding style. Generally in C++ it's very easy to fall into a trap where we write functions that copy and return new objects right and left all the time and paying that cost without having to. Our excuse is that "the compiler will optimize it anyway" (although C++'s copy elision for return values doesn't work all the time, e.g. when the function has conditions...), but the problem is that with that kind of mindset a large program becomes infeasible to run in debug. That really creates friction when developing. Managing data more carefully from the call site produces much faster code, even in debug.

### "No throwing of exceptions, anywhere"
Exceptions make your code complicated. 
You should design your code in such a way that errors can't occur (or if they do - handle them, not just bail, and when even that is not possible - stop execution).
I can't be bothered to write about exceptions. Really, they shouldn't exist in C++, forget about exception safety, `noexcept` and all that bullshit.

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

### Example of custom allocators and implicit context:

```
    // When allocating you should use the context's allocator
    // This makes it so when users call your functions they
    // can specify an allocator beforehand by pushing a new context variable,
    // without having to pass you anything as a parameter for example.
    //
    // The idea for this comes from the implicit context in Jai.
    allocator Alloc = Malloc;

    // The default alignment size 
    u16 AllocAlignment = POINTER_SIZE;  // By default

    // This allocator gets initialized the first time it gets used in a thread.
    // Each thread gets a unique temporary allocator to prevent data races and to remain fast.
    temporary_allocator_data TemporaryAllocData;
    allocator TemporaryAlloc = {temporary_allocator, &TemporaryAllocData};

    // Frees the memory held by the temporary allocator (if any). // Gets called by the context's destructor.
    void release_temporary_allocator();

    io::writer *Log = internal::g_ConsoleLog;
    os_unexpected_exception_handler_t *UnexpectedExceptionHandler = default_unexpected_exception_handler;
    
    thread::id ThreadID; // The current thread's ID
```

```cpp
memory = new char[150]; // Using the default allocator 
memory = new (Context.Alloc) char[150]; // the same as the line above

memory = new (Malloc) char[150];
memory = new (Context.TemporaryAlloc) char[150];

// my_allocator_function implements all the functionality of the allocator (allocating, alignment, freeing, etc.)
// Take a look at _os_allocator_, _default_allocator_, _temporary_allocator_, to see examples on how to implement one.
allocator myAlloctor = { my_allocator_function, null /*Can also pass user data!*/};
memory = new (myAlloctor) char[150];
```

```cpp
char *memory = new char[150]; // using default allocator (Malloc)

// The temporary allocator is provided by the library, it's essentially a fast arena allocator that's available globally and supports only "free all".
// _Context.Alloc_ is a read-only variable inside the Context struct which can be accessed everywhere or changed within a scope with "PUSH_CONTEXT" like so:
PUSH_CONTEXT(Alloc, Context.TemporaryAlloc) {
    // Now using the temporary allocator, because the library overloads operators new/delete.
    // operator new by default uses _Context.Alloc_
    char *memory2 = new char[150];

    // Allocates with an explicit allocator even though _Context.Alloc_ is the temporary allocator.
    char *memory3 = new (Malloc) char[150];
    delete[] memory3;

    // ...
}

// Delete always calls the allocator the memory was allocated with
delete[] memory1;
```
