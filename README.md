
# light-std
A C++20 data-oriented library created for personal use that aims to replace the standard C/C++ library.

It is completely stand-alone - it doesn't include any headers from the default standard library. It doesn't link with the runtime library. It's built and tested on Windows with the MSVC compiler.

## Why

Memory layout is very important for modern CPUs. The programmer should be very aware of that when writing code.

> Ulrich Drepper, What Every Programmer Should Know About Memory, 2007 - https://people.freebsd.org/~lstewart/articles/cpumemory.pdf

Usually modern high-level languages put much of the memory management behind walls of abstraction.
("Modern C++" as well!) Hardware has gotten blazingly fast, but software has deteriorated. CPUs can make billions
of calculations per second, but reading even ONE byte from RAM can take hundreds of clock cycles
(if the memory is not in the cache). You MUST think about the cache if you want to write fast software.

Once one starts thinking about the cache he starts programming in a data oriented way.
To try to model the real world in the program (OOP) is not the point anymore, but instead 
structure the program in the way that the computer will work with the data - data that is processed 
together should be close together in memory.

That's why we should remove some of the abstraction. We should think about the computer's memory.
We should write fast software. 

Of course, writing abstractions is the rational thing to do (most of the time).
After all we can do so much more stuff instead of micro-optimizing everything. But being a bit too careless
results in the modern mess of software that wastes most of CPU time doing nothing, because people decided to
abstract too much stuff. We can even slow down global warming by not wasting CPU clock cycles.

## This library currently provides:

- A memory model inspired by Jonathan Blow's Jai - simple overridable allocators with an implicit context system.
- Robust debug memory switch that has checks for double frees, cross-thread frees, block overlaps from allocations and etc.
- Data structures - utf-8 non-null-terminated string, dynamic array, hash table, etc.
- `os` module - common operations that require querying the OS.
- `path` module - procedures that work with file paths. 
- `fmt` module - a formatting library inspired by Python's formatting syntax (that's even faster than printf).
- `parse` module - for parsing strings, integers, booleans, GUIDs, etc.
- Threads, mutexes, atomic operations.
- Console input/output.

## Principles 

- **Clean code**
> Readibility and reasonability are most important. Comments are a powerful tool to explain WHY (not WHAT is done) and the philosophy behind it.

- **Less code is better**
> Every line of code is a liability and a possible source of bugs and security holes. So we avoid big dependencies.

- **Closer to C than to modern C++**
> Ditch copy/move constructors, destructors, exceptions. This reduces the amount of abstractions and bloat code and increases our confidence that the code is doing what we expect.

- **Code reusability**
> Using C++20 features: terse templates, concepts, and modules, we can do conditional procedure compilation and combine functions in ways to reduce code, which otherwise would be a combinatorial amount. Examples: `parse.cppm` and `array_like.cppm`. Conditional procedure compilation even leads to performance improvement (even in Debug), and in C++20 it doesn't require ugly macro magic which makes code difficult to reason about (most of the time it's as simple as `if constexpr`).

## Documentation

### Type policy

#### Ideal
- Keep it simple.
- Keep it data oriented.
  Programs work with data. Design your data so it makes the solution straightforward and minimize abstraction layers.
- Use struct instead of class, keep everything public.
- Always provide a default constructor, which does as little as possible.
- copy/move constructors and destructors are banned. No excuse.
- No throwing of exceptions, .. ever, .. anywhere. No excuse.
  They make code complicated. When you can't handle an error and need to exit from a function, return multiple values. C++ doesn't really help with this, but you can use C++17 structured bindings, e.g.:
>          auto [content, success] = path_read_entire_file("data/hello.txt");
- In general, error conditions (which require returning a status) should be rarely justifiable. The code should just do the correct stuff. Otherwise it quickly becomes complicated and we lose confidence on what could happen and where.

#### Example
`array` is this library is a basic wrapper around contiguous memory. It contains 2 fields (`Data` and `Count`).
It has no sense of ownership. That is determined explictly in code and by the programmer.

By default arrays are views, to make them dynamic, call `make_dynamic(&arr)`.
After that you can modify them (insert or remove elements).

You can safely pass around copies and return arrays from functions by value because
there is no hidden destructor which will free the memory. Moreover they are just
a pointer and a size, so it's cheap and avoids indirection from passing them by pointer/reference. 

If you call `make_dynamic(&arr)` nothing really special happens except
that `arr.Data` is now pointing to allocated memory. To free it when no longer needed call `free(arr.Data)` as normal.
You can also call `defer(free(arr.Data))` to free it on scope exit (like a destructor).

> ```cpp
>      array<string> pathComponents;
>      make_dynamic(&pathComponents, 8);
>      defer(free(pathComponents.Data));
> ```

`string`s are just `array<char>`. All of this applies to them as well.
They are treated as utf-8 and not null-terminated, which means that taking substrings doesn't allocate memory.

To make a deep copy of an array (or string) use clone(): `newPath = clone(&path)`.

> ```cpp
>      // Constructed from a zero-terminated string buffer. Doesn't allocate memory.
>      // Like arrays, strings are views by default.
>      string path = "./data/";
>      make_dynamic(&path);         // Allocates a buffer and copies the string it was pointing to
>      defer(free(path.Data));
>      
>      string_append(&path, "output.txt");
>      
>      string pathWithoutDot = substring(path, 2, -1);
> ```

> Functions on objects which take indices allow negative reversed indexing which begins at
> the end of the array/string, so -1 is the last element (or code point) -2 the one before that, etc. (Python-style)

### Arrays by example

> ex.1 
> ```cpp
>      byte data[100];
>      auto subArray = array<byte>(data + 20, 30); 
> 
>      auto shallowCopy = subArray;
> ```

> ex.2 Allocating memory explicitly.
> ```cpp
>      array<char> sequence;
>      make_dynamic(&sequence 8);
>      add(sequence, '0');
>      add(sequence, 'b');
>      add(sequence, '1');
>      // ... futher appends may require reallocating which is handled automatically (oldAlloc * 2)
>  
>      array<char> otherSequence = clone(&sequence);  // Deep-copy
>      defer(free(otherSequence.Data));               // Runs at the end of the scope
>  
>      auto shallowCopyOfSequence = sequence;
>      free(shallowCopyOfSequence); // 'sequence' is also freed (they point to the same memory)
>  
>      // Here 'sequence' is no longer valid, but 'otherSequence' (which was deep copied) still is.
>  
>      // Attempting to free a subarray is undefined.
>      // It is guaranteed to trip an assert in our memory layer, before the allocator.
>      auto subData = array<byte>(otherSequence.Data + 1, 2);
>      free(subData.Data); 
> ```

> ex.3 Example how array doesn't know what the current context allocator is (more on that later). 
> Here `PERSISTENT` is an allocator that the platform layer uses for internal allocations. The example is taken directly from the `os` module.
> ```cpp
> [[nodiscard("Leak")]] array<string> os_get_command_line_arguments() {
>     s32 argc;
>     utf16 **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
>   
>     if (argv == null) return {}; // We may return an empty array safely, because `free` on a null does nothing.
>  
>     array<string> result;
>     PUSH_ALLOC(PERSISTENT) {
>         make_dynamic(&result, argc - 1);
>
>         // Loop over all arguments but skip the .exe name
>         For(range(1, argc)) {
>             add(&result, utf16_to_utf8(argv[it]));
>         }
>     }
>     // This does a shallow copy, there is no copy constructor, but there is also no destructor 
>     // to invalidate `result`. After programming for a while it feels great to do this sort of stuff
>     // without expecting unnecessary allocations to happen.
>     return result; 
> }
> ```

> ex.4 We also provide some syntactic sugar: terse `for` macros and a Python-like `range` function.
> ```cpp
>      array<s32> integers;
>
>      for (s32 it : range(50)) { // Means "for every integer in [0, 50)" 
>          add(&integers, it);
>      }
>
>      for (s32 it : range(40, 20, -2)) { // Every second integer in [40, 20) in reverse
>          add(&integers, it);
>      }
>
>      For(range(10, 20, 3)) { // Every third integer in [10, 20).. you get the idea
>          add(&integers, it);
>      }
>
>
>      // Specify custom variable names in the macro
>      For_as(i, range(10)) { 
>          For_as(j, range(20)) { /* i,j in 10x20 grid */ } 
>      }
>      
>
>      array<string> args = os_get_command_line_arguments();
>      defer(free(args.Data));
>
>      For(args) print("{}\n", it); 
>     
>
>      For_enumerate(args) {
>          // Here we have 2 implicit variables - 'it' and 'it_index', which contain the current object and it's respective index.
>
>          if (it == "--option") {
>              if (it_index + 1 >= args.Count) continue; // Probably tell the user...
>              value = args[it_index + 1];
>              continue;
>          }
>      }
>
>      // Here you can also specify custom variable names
>      // For_enumerate_as(my_it_index_name, my_it_name, args) { .. }
> ```

> ex.5 Arrays support Python-like negative indexing.
> ```cpp
>      auto nums = make_stack_array(1, 2, 3, 4, 5);
>
>      // Negative index is the same as "Length - index", 
>      // so -1 is translated to the index of the last element.
>      print("{}", nums[-1]); // 5
> ```

> ex.6 We also have a wrapper for a fixed-size array (equivalent to std::array).
> ```cpp
>      s32 data[5];               // These two variables have the same size, but the second one
>      stack_array<s32, 5> data;  // can be passed as a parameter to functions without decaying to a pointer.
>
>      s64 data[] = {1, 2, 3, 4, 5};
>      auto data = make_stack_array<s64>(1, 2, 3, 4, 5);
> ```

> ex.7 Arrays constexpr (C++20 also supports dynamic allocation at compile-time)
> ```cpp
>      constexpr auto data = make_stack_array<s64>(1, 2, 3, 4, 5);
>      constexpr array<s64> view = data;
> ```


### Strings by example

Strings in this library support unicode (UTF-8) and aren't null-terminated.

`str.Count` tells you the number of bytes stored in the string. To get
the number of code points call `string_length(str)`.
The number of code points is smaller or equal to the number of bytes,
because a certain code point can be encoded in up to 4 bytes.

Array examples from the previous section are relevant here.

Note: Functions like `insert_at_index` have a different suffix for strings `string_insert_at_index`, because we treat indices for strings as pointing to code points. If you want to work with the raw bytes of the string, you can use the normal non-prefixed array routines.

> ex.1 This string is constructed from a zero-terminated string buffer (which the compiler stores in read-only memory when the program launches). 
> This doesn't allocate memory, `free(path.Data)` will crash.
> ```cpp
>         string path = "./data/"; 
> ```

> ex.2 Substrings don't cause allocations.
> ```cpp
>     string path = "./data/output.txt";
>     
>     s64 dot = string_find(path, '.', string_length(path), true);  // Reverse find 
>     string pathExtension = substring(path, dot, -1); // ".txt"    
> ```

> ex.3 Modifying individual code points.
> ```cpp
>     string greeting = "ЗДРАСТИ";
>     For(greeting) {
>         it = to_lower(it);
>         // Here the string is non-const so 'it' is actually a reference 
>         // to the code point in the string, so you can modify it directly.
>         // This also correctly handles replacing a code point with a larger 
>         // or smaller one. 
>         // See the implementation of string_iterator.
>     }
>     greeting; // "здрасти"
> ```

> ex.4 This example is taken directly from the `path` module.
> ```cpp
> [[nodiscard("Leak")]] array<string> path_split_into_components(string path) {
>     array<string> result;
>     make_dynamic(&result, 8);
>
>     s64 start = 0, prev = 0;
>     while ((start = string_find_any_of(path, "\\/", start + 1)) != -1) {
>         add(&result, substring(path, prev, start);
>         prev = start + 1;
>     }
>     
>     // There is an edge case in which the path ends with a slash, in that case there is no "another" component.
>     // The if is here so we don't crash with index out of bounds.
>     //
>     // Note that both /home/user/dir and /home/user/dir/ mean the same thing.
>     // You can use other functions to check if the former is really a directory or a file (querying the OS).
>     if (prev < path.Length) {
>         // Add the last component - from prev to path.Length
>         add(&result, substring(path, prev, string_length(path)));
>     }
>     return result;
> }
> ```

> ex.5 Also taken from the `path` module.
> ```cpp
> [[nodiscard("Leak")]] string path_join(array<string> paths) {
>     auto [result_drive, result_path] = path_split_drive(paths[0]);
> 
>     string result = clone(&result_path);
> 
>     For(range(1, paths.Count)) {
>         auto p = paths[it];
>         auto [p_drive, p_path] = path_split_drive(p);
>         if (p_path && path_is_sep(p_path[0])) {
>             // Second path is absolute
>             if (p_drive || !result_drive) {
>                 result_drive = p_drive;  // These are just substrings so it's fine
>             }
>             free(result.Data);
>             result = clone(&p_path);
>             continue;
>         } else if (p_drive && p_drive != result_drive) {
>             // _string_compare_ignore_case_ returns the index at which the strings differ.
>             // it returns -1 if the strings match.
>             if (string_compare_ignore_case(p_drive, result_drive) != -1) {
>                 // Different drives => ignore the first path entirely
>                 result_drive = p_drive;
>                 free(result.Data);
>                 result = clone(&p_path);
>                 continue;
>             }
>             // Same drives, different case
>             result_drive = p_drive;
>         }
> 
>         // Second path is relative to the first, append them 
>         if (result && !path_is_sep(result[-1])) {
>             string_append(&result, '\\');
>         }
>         string_append(&result, p_path);
>     }
> 
>     // Add separator between UNC and non-absolute path if needed
>     if (result && !path_is_sep(result[0]) && result_drive && result_drive[-1] != ':') {
>         string_insert_at_index(&result, 0, '\\');
>     } else {
>         string_insert_at_index(&result, 0, result_drive);
>     }
>     return result;
> }
> ```

### Allocating memory by example

We don't use new and delete.
1) The syntax is ugly in my opinion.
2) You have to be careful not to mix `new` with `delete[]`/`new[]` with `delete`.
3) It's an operator and can be overriden by structs/classes.
4) Modern C++ people say not to use new/delete as well, so ..

Now seriously, there are two ways to allocate memory in C++: `malloc` and `new`.
For the sake of not introducing a THIRD way, we override `malloc`.

> Since we don't link the CRT `malloc` is undefined, so we need to
> provide a replacement anyway (or modify code which is annoying and
> not always possible, e.g. a prebuilt library, @TODO it may actually be possible with symbol patching.).

`new` and `delete` actually have different semantics (`new` - initializing the values,
`delete` - calling a destructor if defined). Our type policy is against destructors
but we will call them anyway, just in case (some third party libraries might rely on that).

So we provide templated versions of malloc/free:

`malloc<T>`:
- Calls constructors on non-scalar values.
- Returns T* so you don't have to cast from void*
- Can't call with T == void (use non-templated malloc in that case!)

`realloc<T>`
- Assumes your type can be copied to another place in memory byte by byte and just work.
- Doesn't call copy/move constructors.

`free<T>`
- Also calls the destructor. Normally we are against destructors - explicit code is better.
To implement "destructor functionality" (uninitializing the type, freeing members, etc.)
you should provide a function, for example: 

>```cpp
>    free_string(my_string *s) {
>        free(s.Data);   // Free buffer as normal
>        atomic_add(&GlobalStringCount, -1);
>    }
>```

You may say that this is just a renamed destructor. But this doesn't run hiddenly
(when exiting a scope or a function). Sometimes you actually want to do that. Instead of littering every return path with `free(s)`, you can do `defer(free(s));` which is a useful macro that calls 
any statements at the end of a scope (emulates a destructor).

#### Context and allocators 

The `Context` is a global thread local variable that contains certain options that change the behaviour of the program.

It stores the allocator that is used by default for allocating new blocks, the logger which is used to print messages (pointer to a writer, e.g. `cout`), and other stuff.

Here is a part of the structure (full structure defined in `context.cppm`): 
```cpp
struct context {
    u32 ThreadID; 
    
    allocator Alloc;  
    u16 AllocAlignment = POINTER_SIZE;
    u64 AllocOptions = 0;

    bool LogAllAllocations = false; 
    panic_handler_t PanicHandler = default_panic_handler;
    writer *Log;
};
```
Feel free to add more variables to your local copy of the library.

The context gets initialized when the program runs and also when creating a thread with our API. 
In the second case know the parent thread, so we copy the options from it. If the thread is created
in another way we leave it zero initialized and up to the caller to decide what should go inside of it.
(AFAIK there is now way to tell what the parent thread was in other APIs...)

> ex.1 
> ```cpp
> void *memory;
> memory = malloc<char>({.Count = 150});   // Using Context.Alloc (make sure you have set it beforehand)
> 
> memory = malloc<char>({.Count = 150, .Alloc = TemporaryAllocator}); // Uses another allocator explicitly
> 
> // You can also implement custom allocators:
> // my_allocator_function is a function which implements all the functionality of the allocator (allocating, resizing, freeing, etc.)
>
> allocator myAlloctor = { my_allocator_function, null };
> memory = malloc<char>({.Count = 150, .Alloc = myAlloctor});;
> ```

> ex.2 Using C++20 syntax.
> ```cpp
>     void *node = malloc<ast_binop>();
>     void *node = malloc<ast_binop>({.Alloc = AstNodeAllocator});
>     void *node = malloc<ast_binop>({.Options = LEAK});
> 
>     auto *simdType = malloc<f32v4>({.Alignment = 16});
> 
>     void *memory = malloc<byte>({.Count = 200});
>     void *memory = malloc<byte>({.Count = 200, .Alloc = TemporaryAllocator, .Alignment = 64, .Options = LEAK});
> ```

> ex.3 Example taken from the test suite. Demonstates temporary allocator initialization and how to change the allocator which is used in a scope.
> ```cpp
>     time_t start = os_get_time();
> 
>     // Initialize the temporary allocator by asking the OS for large pool.
>     TemporaryAllocatorData.Block = os_allocate_block(1_MiB);
>     TemporaryAllocatorData.Size  = 1_MiB;
>     
>     auto newContext = Context;
>     newContext.AllocAlignment = 16;          // For SIMD math types
>     newContext.Alloc = TemporaryAllocator;   // Set a new default allocator
> 
>     PUSH_CONTEXT(newContext) {
>         build_test_table();
>         run_tests();
>     }
>     print("\nFinished tests, time taken: {:f} seconds, bytes used: {}\n\n", os_time_to_seconds(os_get_time() - start), TemporaryAllocatorData.Used);
> ```

> ex.4 Example taken from a physics engine simulation with `light-std-graphics`. This demonstates how to allocate global state + a pool and initialize a common general purpose allocator at the beginning of your program.
> ```cpp
>     // We allocate all the state we need next to each other in order to reduce fragmentation.
>     auto [data, m, g, pool] = os_allocate_packed<tlsf_allocator_data, memory, graphics>(MemoryInBytes);
>
>     // TLSF (Two-Level Segregate Fit) is a fast general purpose dynamic memory allocator for real-time systems.
>     // We provide an implementation directly in the library.
>     PersistentAlloc = {tlsf_allocator, data};
>     allocator_add_pool(PersistentAlloc, pool, MemoryInBytes);
> 
>     Memory = m;
>     Graphics = g;
> 
>     auto newContext = Context;
>     newContext.Log = &cout;
>     newContext.Alloc = PersistentAlloc;
>
>     // This macro doesn't change the context in a scope but 
>     // instead overrides the parameter all-together. 
>     // Use this only at the beginning of your program!
>     // Be very careful and never use this inside functions
>     // (the caller might get confused)...
>     OVERRIDE_CONTEXT(newContext); 
> ```

> ex.5 How to implement custom allocators. Here is the implementation of the arena allocator.
> ```cpp
> void *arena_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
>     auto *data = (arena_allocator_data *) context;
>     auto* data = (arena_allocator_data*)context;
>
>     switch (mode) {
>         case allocator_mode::ALLOCATE: {
>             if (data->Used + size >= data->Size) return null;  // Not enough space
>         
>             void* result = (byte*)data->Block + data->Used;
>             data->Used += size;
>             return result;
>         }
>         case allocator_mode::RESIZE: {
>             void* p = (byte*)data->Block + data->Used - oldSize;
>             if (oldMemory == p) {
>                 // We can resize only if it's the last allocation
>                 data->Used += size - oldSize;
>                 return oldMemory;
>             }
>             return null;
>         }
>         case allocator_mode::FREE: {
>             // We don't free individual allocations in the arena allocator
>             return null;
>         }
>         case allocator_mode::FREE_ALL: {
>             data->Used = 0;
>             return null;
>         }
>     }
>     return null;
> }
> ```

### Available types of allocators in the library

> See `"memory.cppm"`.

#### TLSF

Generally malloc implementations do the following:
- Have seperate heaps for different sized allocations
- Call OS functions for very large allocations
- Different algorithms for allocation, e.g. stb_malloc implements the TLSF algorithm for O(1) allocation

Here we provide a wrapper around the TLSF algorithm. Here is how you should use it:
Allocate a large block with the OS allocator (that's usually how everything starts).
Call `allocator_add_pool()` on the TLSF (`ex.4` from the previous section).

You can write a general purpose allocator and do what `stb_malloc` does for different sized allocations
or you can have several TLSF specialized allocators with different blocks. Both are equivalent. Again,
we really try to push you to think about how memory in your program should be structured together,
and having a really general allocator for EVERY type of allocation is not ideal, as it just sweeps 
everything under the rug.

```cpp
// Two-Level Segregated Fit memory allocator implementation. Wrapper around tlsf.h/cpp (in vendor folder),
// written by Matthew Conte (matt@baisoku.org). Released under the BSD license.
//
// * O(1) cost for alloc, free, resize
// * Extremely low overhead per allocation (4 bytes)
// * Low overhead per TLSF management of pools (~3kB)
// * Low fragmentation
//
void *tlsf_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);
```

#### Arena

This type of allocator super fast because it basically bumps a pointer.
With this allocator you don't free individual allocations, but instead free
the entire thing (with `free_all()`) when you are sure nobody uses the memory anymore.
Note that `free_all()` doesn't free the added block, but instead resets its
pointer to the beginning of the buffer.

The arena allocator doesn't handle overflows (when the block doesn't have enough space for an allocation).
When out of memory, you should resize or provide another block.

```cpp
void *arena_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);
```
The temporary allocator is just a variation of the arena allocator. 
It's just placed in global scope and it's thread-local.
The two variables which are of interest are `TemporaryAllocator` and `TemporaryAllocatorData`.
See `ex.3` from previous section to see how to initialize it (or any other arena allocator).

#### Pool allocator 

This is a variation of the fast bump arena allocator.
Allows O(1) allocation and freeing of individual elements.

The limitation is that each allocation must have the same predefined size.
This allocator is useful for managing a bunch of objects of the same type.

Internally it keeps a linked list of added blocks. To provide a block call
`pool_allocator_provide_block(.., block, size)`. Instead of allocating a
linked list at another place in memory, the pool uses the first few bytes
of the provided block as a header.

```cpp
void *pool_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);
```

### Credits

The appropriate licenses are listed alongside this list in the file `CREDITS`.

- [Cephes](https://www.netlib.org/cephes/), Stephen L. Moshier, math functions as a replacement to avoid linking with the CRT.
- [tlsf](https://github.com/mattconte/tlsf), Matthew Conte (http://tlsf.baisoku.org), Two-Level Segregated Fit memory allocator, version 3.1.
- [delegate](https://github.com/tarigo/delegate), Vadim Karkhin, 2015
- Ryan Juckett, 2014, implementation the Dragon4 algorithm (used in the `fmt` module for formatting floats).
- [minlibc]( https://github.com/GaloisInc/minlibc), Galois Inc., 2014, `strtod` and `atof` implementations.
- [LIBFT](https://github.com/beloff-ZA/LIBFT/blob/master/libft.h) beloff, 2018, some implementations of functions found in the CRT.
- Rolf Neugebauer, 2003, `sccanf` implementation.

### Projects using this library

- [light-std-graphics](https://github.com/Repertoi-e/light-std-graphics) - high-level windowing API (like GLFW) and a high-level graphics API (currently has only DirectX bindings).
- [Physics engine](https://repertoi-e.github.io/Portfolio/Simulating-Rigid-Body-Physics.html) - tutorial about simulating rigid bodies (and using `light-std-graphics` for demos).

