
# light-std
A C++20 library created for personal use that aims to replace the standard C/C++ library. It's data-oriented and useful for game programming.

This library is supposed to be a replacement of C/C++'s standard library in but designed entirely differently. 

It is completely stand-alone - it doesn't include any headers from the default standard library. Some C++ language features (like the spaceship operator, initializer lists, etc.) require certain definitions in the std:: namespace, but we provide our own placeholders (tested on the MSVC compiler). 

## Manifest

Memory layout is very important for modern CPUs. The programmer should be very aware of that when writing code.

> Ulrich Drepper, What Every Programmer Should Know About Memory, 2007 - https://people.freebsd.org/~lstewart/articles/cpumemory.pdf

C++ is a low-level language.
Usually modern high-level languages put much of the memory management behind walls of abstraction.
Hardware has gotten blazingly fast, but software has deteriorated. Modern CPUs can make billions
of calculations per second, but reading even ONE byte from RAM can take hundreds of clock cycles
(if the memory is not in the cache). You MUST think about the cache if you want to write fast software.

Once you start thinking about the cache you start programming in a data oriented way.
You don't try to model the real world in the program, but instead structure the program in the way that
the computer will work with the data. Data that is processes together should be close together in memory.

And that's why we should remove some of the abstraction. We should think about the computer's memory.
We should write fast software. We can slow down global warming by not wasting CPU clock cycles.

Caveat: Of course, writing abstractions which allows more rapid programming is the rational thing to do.
After all we can do so much more stuff instead of micro-optimizing everything. But being a bit toocareless
results in the modern mess of software that wastes most of CPU time doing nothing, because people decidedto
abstract too much stuff.

I am writing this in 2021 - in the midst of the COVID-19 crisis. We have been using online videoconferencing
software for school and work for a year now. The worst example is MS Teams. Clicking a button takes a goosecond
to open a chat. It lags constantly, bugs everywhere. No. Your computer is not slow. Your computer is a
super-computer compared to what people had 30-40 years ago.

Not only you waste electricity by being a careless programmer, you also waste USER'S TIME!
If your program is used by millions of PC, 1 second to click a SIMPLE BUTTON quickly becomes hours and then days.

Note: Games are an exception to this trend, because engine programmers always try to push the hardware limits.  

## This library provides:

- A memory model inspired by Jonathan Blow's Jai - implicit context system, overridable allocators.
- Data structures - utf-8 non-null-terminated string, dynamic array, hash table, etc.
- `os` module - common operations that require querying the OS.
- `path` module - procedures that work with file paths. 
- `fmt` module - a formatting library inspired by Python's formatting syntax (that's even faster than printf).
- Threads, mutexes, atomic operations, lock-free data structures.

## Principles 

- **Clean code.**
> Readibility is most important. Comments are a powerful tool. 

- **Less code is better.**
> Every line of code is a liability and a possible source of code. Avoid big dependencies, 
> if you need just one function - write it. Don't import a giant library.

- Closer to C than to C++.
> Ditch copy/move constructors, destructors, exceptions. This reduces the amount of bloat code and supposedly increases confidence that your code is doing what you expect.

- Code reusability. 
> A trick: conditional procedure compilation. You can enable or disable features for a function depending
> on template parameters. e.g. our `parse_int` function - one piece of code but it suits many cases.
> It performs as if it's a specialized function, even though it's very general.

> Another trick: with C++20's concepts we can implement one set of functions that work with all arrays (`array_like.h`).
> The following: search functions (`find`, `find_not`, `has`, etc.), `compare`, and operators `==`, `!=`, `<`, `<=`, `>`, `>=`
> are written once and work with all array-like data structures (even comparing different types of arrays works, for 
> which otherwise we need a combinatorial amount of code).

- Written for the future.
> Some sacrifices are made. This library isn't backwards compatible, it uses C++20 `modules` and `concepts`, C++17 `if constexpr`, etc.
> Eventually I plan to write a compiler that translates a custom language (an extension to C++) to C
> which supports meta-programming and changes the syntax to avoid vexing parses. 

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

#### Example
'array' is this library implemented the following way:
    A struct that contains 2 fields (Data and Count).

    It has no sense of ownership. That is determined explictly in code and by the programmer.

    By default arrays are views, to make them dynamic, call `make_dynamic(&arr)`.
    After that you can modify them (add/remove elements)

    You can safely pass around copies and return arrays from functions because
    there is no hidden destructor which will free the memory.

    When a dynamic array is no longer needed call `free(arr.Data)`;

    We provide a defer macro which runs at the end of the scope (like a destructor),
    you can use this for functions which return from multiple places,
    so you are absolutely sure `free` gets called and there were no leaks.

    e.g.
    ```cpp
    array<string> pathComponents;
    make_dynamic(&pathComponents, 8);
    defer(free(pathComponents.Data));
    ```

`string`s are just `array<char>`. All of this applies to them as well.
They are not null-terminated, which means that taking substrings doesn't allocate memory.


    // Constructed from a zero-terminated string buffer. Doesn't allocate memory.
    // Like arrays, strings are views by default.
    string path = "./data/";
    make_dynamic(&path);         // Allocates a buffer and copies the string it was pointing to
    defer(free(path.Data));

    string_append(&path, "output.txt");

    string pathWithoutDot = substring(path, 2, -1);

To make a deep copy of an array use clone().
e.g. `string newPath = clone(path) // Allocates a new buffer and copies contents in _path_`

### Arrays by example

> ex.1 Arrays as views. Constructing an array object doesn't allocate any memory (since it's detached from the idea of ownership).
> ```cpp
>      byte data[100];
>      auto subArray = array<byte>(data + 20, 30); 
> 
>      auto shallowCopy = subArray;
> ```

> ex.2 Arrays allocate memory explicitly.
> ```cpp
>      array<char> sequence;
>      make_dynamic(&sequence 8);
>      add(sequence, '0');
>      add(sequence, 'b');
>      add(sequence, '1');
>      // ... futher appends may require resizing which is handled automatically
>  
>      array<char> otherSequence = clone(&sequence);  // Deep-copy
>      defer(free(otherSequence.Data));               // Runs at the end of the scope
>  
>      auto shallowCopyOfSequence = sequence;
>      free(shallowCopyOfSequence); // 'sequence' is also freed (they point to the same memory)
>  
>      // Here 'sequence' is no longer valid, but 'otherSequence' still is.
>  
>      // Attempting to free a subarray is undefined.
>      // It is guaranteed to crash the allocator
>      auto subData = array<byte>(otherSequence.Data + 1, 2);
>      free(subData.Data); 
> ```

> ex.3 Arrays allocate functions by the implicit context allocator (more on that later). 
> Here `PERSISTENT` is an allocator that the platform layer uses for internal allocations (example is taken from the `os` module).
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
>     return result;
> }
> ```

> ex.4 We also provide some syntactic sugar: terse `for` macros and a Python-like `range` function.
> ```cpp
>      array<s32> integers;
>
>      for (s32 it : range(50)) { // Every integer in [0, 50) 
>          add(&integers, it);
>      }
>
>      for (s32 it : range(40, 20, -2)) { // Every second integer in [40, 20) in reverse
>          add(&integers, it);
>      }
>
>      For(range(10, 20)) { // Every integer in [10, 20).. you get the idea
>          add(&integers, it);
>      }
>
>
>      // Avoid variable name collisions
>      For_as(i, range(10)) { 
>          For_as(j, range(20)) { .. } 
>      }
>      
>
>      array<string> args = os_get_command_line_arguments();
>      defer(free(args.Data));
>      For(args) print("{}\n", it); 
>     
>
>      For_enumerate(args) {
>          // Here we have 2 implicit variables - 'it' and 'it_index', which contain the current object and it's respective index.
>
>          if (it == "--option") {
>              assert(it_index + 1 < args.Count); // Probably tell the user...
>              value = args[it_index + 1];
>              continue;
>          }
>      }
>
>      // For_enumerate_as(my_it_index_name, my_it_name, args) { .. }
> ```

> ex.5 Arrays support Python-like negative indexing.
> ```cpp
>      auto list = make_stack_array(1, 2, 3, 4, 5);
>
>      // Negative index is the same as "Length - index", 
>      // so -1 is translated to the index of the last element.
>      print("{}", list[-1]); // 5
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

Note: Functions like `insert_at_index` have a different suffix for strings `string_insert_at_index`, because we treat indices for strings as pointing to code points. If you want to work with the raw bytes of the string, you can use the array routines.

> ex.1 This string is constructed from a zero-terminated string buffer (which the compiler stores in read-only memory when the program launches). 
> This doesn't allocate memory, `free(path.Data)` will crash.
> ```cpp
>         string path = "./data/"; 
> ```

> ex.2 Substrings don't cause allocations.
> ```cpp
>     string path = "./data/output.txt";
>     
>     s64 dot = string_find(path, '.', string_length(path), true); // Reverse find (API subject to change...)
>     string pathExtension = substring(path, dot, -1); // ".txt"    
> ```

> ex.3 Modifying individual code points.
> ```cpp
>     string greeting = "ЗДРАСТИ";
>     For(greeting) {
>         it = to_lower(it);
>         // Here the string is non-const so 'it' is actually a reference 
>         // to the code point in the string, so you can modify it directly.
>         // This also correctly handles replacing a code point with a larger one. 
>         // See the implementation of string_iterator.
>     }
>     greeting; // "здрасти"
> ```

> ex.4 Taken from the `path` module.
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

> ex.5 Taken from the `path` module.
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
>         // Second path is relative to the first
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

### Allocating memory

Don't use new and delete.
1) You have to be careful not to mix `new` with `delete[]`/`new[]` with `delete`.
2) It's an operator and can be overriden by structs/classes.
3) Modern C++ people say not to use new/delete as well, so ..

Now seriously, there are two ways to allocate memory in C++: `malloc` and `new`.
For the sake of not introducing a THIRD way, we override malloc.
We do that because :STANDARDLIBRARYISBANNED:.
Since we don't link with the CRT, `malloc` is undefined, so we need to
provide a replacement anyway (or modify code which is annoying and
not always possible, e.g. a prebuilt library).

> Caveat: A DLL may already have linked with the CRT, which means that in
> that case problems occur. There are two options: rebuild your DLLs
> to not use the standard library/or we could do some hacks
> and redirect calls to malloc to our replacement (@TODO It may actually be possible).

`new` and `delete` actually have some useful semantics (`new` - initializing the values,
`delete` - calling a destructor if defined). So we provide templated versions of malloc/free:

`malloc<T>`:
- Calls constructors on non-scalar values.
- Returns T* so you don't have to cast from void*
- Can't call with T == void (use non-templated malloc in that case!)

`realloc<T>`
- Assumes your type can be copied to another place in memory and just work.
- Doesn't call copy/move constructors.

`free<T>`
- Also calls the destructor (we are against destructors usually but some syntax-sugar code might rely on that).

You might be annoyed already but for more philosophy (like why we don't like exceptions, copy or move constructors) you can look at the :TypePolicy:.


### Context and allocators by example

The `Context` is a global thread local variable that contains certain options that change the behaviour of the program.

It stores the allocator that is used by default for allocating new blocks, the logger which is used to print messages (pointer to a writer, e.g. `cout`), and other stuff.

Here is the full structure (defined in `context.cppm`): 
```cpp
// @Volatile
struct context {
    u32 ThreadID;  // The current thread's ID 

    //
    // This is a pair of a function and a data pointer.
    // Change this (recommended way is to use the PUSH_ALLOC macro) in order to
    // change the allocator which a piece of code uses transparently.
    //
    // This makes it so when you call a function, the caller doesn't have to pass a parameter,
    // the function can just allocate normally, without knowing that the caller has changed the allocator.
    //
    // When allocating you should use the context's allocator (allocate<> and allocate_array<> does
    // that by default - unless overriden explicitly, we also override operator new and delete @TODO malloc).
    //
    // The idea for this comes from the implicit context in Jai.

    allocator Alloc;  // = null by default. The user should provide an allocator at the start of the program.
                      // We encourage using several different allocators depending on the memory requirements and the specific use case.
                      // See :BigPhilosophyTime: in "allocator.h" for the reasoning behind this.

    u16 AllocAlignment = POINTER_SIZE;

    // Any options that get OR'd with the options in any allocation (options are implemented as flags).
    // e.g. using the LEAK flag, you can mark the allocations done in a whole scope as leaks (don't get reported when calling DEBUG_memory->report_leaks()).
    u64 AllocOptions = 0;

    bool LogAllAllocations = false;  // Used for debugging. Every time an allocation is made, logs info about it.

    // Gets called when the program encounters an unhandled expection.
    // This can be used to view the stack trace before the program terminates.
    // The default handler prints the crash message and stack trace to _Log_.
    panic_handler_t PanicHandler = default_panic_handler;

    // When printing you should use this variable.
    // This makes it so users can redirect logging output.
    // By default it points to cout (the console).
    writer *Log;

    // By default when we encounter an invalid format string we panic the program.
    // One might want to silence such errors and just continue executing, or redirect the error - like we do in the tests.
    fmt_parse_error_handler_t FmtParseErrorHandler = fmt_default_parse_error_handler;

    // Disable stylized text output (colors, background colors, and bold/italic/strikethrough/underline text).
    // This is useful when logging to files/strings and not the console. The ansi escape codes look like garbage in files/strings.
    bool FmtDisableAnsiCodes = false;

    bool _HandlingPanic = false;        // Don't set. Used to avoid infinite looping when handling panics. Don't touch!
    bool _LoggingAnAllocation = false;  // Don't set. Used to avoid infinite looping when the above bool is true. Don't touch!
};
```

Feel free to add more variables to your copy of the library.

The context gets initialized when the program runs and also when creating a thread with our API. In the second case know the parent thread, so we copy the options from it (otherwise it's zero-initialized leaving the behaviour up to the caller).

> ex.1 Allocating memory.
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


### Memory

The library's allocating functions reserve a bit of space before a block to store a header with information (the size of the allocation, the alignment, the allocator with which it was allocated, and debugging info if DEBUG_MEMORY is defined).

These functions also allow certain options to be specified.

> ex.2 Using C++20 syntax for allocator options.
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

> ex.3 From the test suite. Demonstates temporary allocator initialization and how to change the allocator which is used in a scope.
> ```cpp
>     time_t start = os_get_time();
> 
>     auto newContext = Context;
>     newContext.AllocAlignment = 16;          // For SIMD math types
>     newContext.Alloc = TemporaryAllocator;   // Set a new default allocator
> 
>     allocator_add_pool(TemporaryAllocator, os_allocate_block(1_MiB), 1_MiB);
> 
>     PUSH_CONTEXT(newContext) {
>         build_test_table();
>         run_tests();
>     }
>     print("\nFinished tests, time taken: {:f} seconds, bytes used: {}, pools used: {}\n\n", os_time_to_seconds(os_get_time() - start), __TempAllocData.TotalUsed, __TempAllocData.PoolsCount);
> ```

> ex.4 From the `light-std-graphics`. This demonstates how to allocate global state + a pool and initialize a common general purpose allocator at the beginning of your program.
> ```cpp
>     // We allocate all the state we need next to each other in order to reduce fragmentation.
>     auto [data, m, g, pool] = os_allocate_packed<tlsf_allocator_data, memory, graphics>(MemoryInBytes);
>     PersistentAlloc = {tlsf_allocator, data};
>     allocator_add_pool(PersistentAlloc, pool, MemoryInBytes);
> 
>     Memory = m;
>     Graphics = g;
> 
>     auto newContext = Context;
>     newContext.Log = &cout;
>     newContext.Alloc = PersistentAlloc;
>     // Doesn't change in a scope but instead overrides the parameter all-together. 
>     // Use this only at the beginning of your program!
>     OVERRIDE_CONTEXT(newContext); 
> ```

> ex.5 The implementation of an arena allocator.
> ```cpp
> void *arena_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
>     auto *data = (arena_allocator_data *) context;
> 
>     switch (mode) {
>         case allocator_mode::ADD_POOL: {
>             auto *pool = (allocator_pool *) oldMemory;  // _oldMemory_ is the parameter which should contain the block to be added
>                                                         // the _size_ parameter contains the size of the block
> 
>             if (!allocator_pool_initialize(pool, size)) return null;
>             allocator_pool_add_to_linked_list(&data->Base, pool); // @Cleanup
>             if (pool) {
>                 ++data->PoolsCount;
>                 return pool;
>             }
>             return null;
>         }
>         case allocator_mode::REMOVE_POOL: {
>             auto *pool = (allocator_pool *) oldMemory;
> 
>             void *result = allocator_pool_remove_from_linked_list(&data->Base, pool); // @Cleanup
>             if (result) {
>                 --data->PoolsCount;
>                 assert(data->PoolsCount >= 0);
>                 return result;
>             }
>             return null;
>         }
>         case allocator_mode::ALLOCATE: {
>             auto *p = data->Base;
>             while (p->Next) {
>                 if (p->Used + size < p->Size) break;
>                 p = p->Next;
>             }
> 
>             if (p->Used + size >= p->Size) return null;  // Not enough space
> 
>             void *usableBlock = p + 1;
>             void *result = (byte *) usableBlock + p->Used;
> 
>             p->Used += size;
>             data->TotalUsed += size;
> 
>             return result;
>         }
>         case allocator_mode::RESIZE: {
>             // Implementing a fast RESIZE requires finding in which block the memory is in.
>             // We might store a header which tells us that but right now I don't think it's worth it.
>             // We simply return null and let the realloc function allocate 
>             // a new block and copy the contents.
>             return null;
>         }
>         case allocator_mode::FREE: {
>             // We don't free individual allocations in the arena allocator
>             return null;
>         }
>         case allocator_mode::FREE_ALL: {
>             auto *p = data->Base;
>             while (p) {
>                 p->Used = 0;
>                 p = p->Next;
>             }
> 
>             data->TotalUsed = 0;
>             return null;
>         }
>         default:
>             assert(false);
>     }
>     return null;
> }
> ```


### Available types of allocators

See `"memory.cppm"`.

```cpp
// ...
// Generally malloc implementations do the following:
// - Have seperate heaps for different sized allocations
// - Call OS functions for very large allocations
// - Different algorithms for allocation, e.g. stb_malloc implements the TLSF algorithm for O(1) allocation
//
// Here we provide a wrapper around the TLSF algorithm. Here is how you should use it:
// Allocate a large block with the OS allocator (that's usually how everything starts).
// Call allocator_add_pool on the TLSF
//

struct tlsf_allocator_data {
    tlsf_t State = null;  // We use a vendor library that implements the algorithm.
};

// Two-Level Segregated Fit memory allocator implementation. Wrapper around tlsf.h/cpp (in vendor folder),
// written by Matthew Conte (matt@baisoku.org). Released under the BSD license.
//
// * O(1) cost for alloc, free, resize
// * Extremely low overhead per allocation (4 bytes)
// * Low overhead per TLSF management of pools (~3kB)
// * Low fragmentation
//
void *tlsf_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);

//
// General purpose allocator.
//
// void *default_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 *);
// inline allocator Malloc;
//
// We don't provide this, as explained above.
//

struct arena_allocator_data {
    allocator_pool *Base = null;  // Linked list of pools, see arena_allocator.cpp for example usage of the helper routines we provide to manage this.
                                  // Of course, you can implement an entirely different way to store pools in your custom allocator!
    s64 PoolsCount = 0;
    s64 TotalUsed = 0;
};

//
// Arena allocator.
//
// This type of allocator super fast because it basically bumps a pointer.
// With this allocator you don't free individual allocations, but instead free
// the entire thing (with FREE_ALL) when you are sure nobody uses the memory anymore.
// Note that free_all doesn't free the pools, but instead sets their pointers to 0.
//
// The arena allocator doesn't handle overflows (when no pool has enough space for an allocation).
// When out of memory, you should add another pool (with allocator_add_pool()) or provide a larger starting pool.
// See :BigPhilosophyTime: a bit higher up in this file.
//
// You should avoid adding many pools with this allocator because when we searh for empty
// space we walk the entire linked list (we stop at the first pool which has empty space).
// This is the simplest but not the best behaviour in some cases.
// Be wary that if you have many pools performance will not be optimal. In that case I suggest
// writing a specialized allocator (by taking arena_allocator as an example - implemented in arena_allocator.cpp).
void *arena_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);

//
// :TemporaryAllocator: See context.h
//
// This is an extension to the arena allocator, things that are different:
// * This allocator is not initialized by default, but the first allocation you do with it adds a starting pool (of size 8_KiB).
//   You can initialize it yourself in a given thread by calling allocator_add_pool() yourself.
// * When you try to allocate a block but there is no available space, this automatically adds another pool (and prints a warning to the console).
//
// One good example use case for the temporary allocator: if you are programming a game and you need to calculate
//   some mesh stuff for a given frame, using this allocator means having the freedom of dynamically allocating
//   without compromising performance. At the end of the frame when the memory is no longer used you FREE_ALL and
//   start the next frame.
//
// We print warnings when allocating new pools. Use that as a guide to see where you need to pay more attention 
// - perhaps increase the pool size or call free_all() more often.
//
void *default_temp_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);
```

### License

> MIT License
> 
> Copyright (c) 2021 Dimitar Sotirov
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

### Credits

- [LIBFT](https://github.com/beloff-ZA/LIBFT/blob/master/libft.h) beloff, 2018, some implementations of functions found in the CRT.
- [minlibc]( https://github.com/GaloisInc/minlibc), Galois Inc., 2014, `strtod` and `atof` implementations.
```cpp
// Here is the license that came with it:

/*
 * Copyright (c) 2014 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 * 
 *   * Redistributions in binary form must reproduce the above copyright 
 *     notice, this list of conditions and the following disclaimer in 
 *     the documentation and/or other materials provided with the 
 *     distribution.
 * 
 *   * Neither the name of Galois, Inc. nor the names of its contributors 
 *     may be used to endorse or promote products derived from this 
 *     software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
```

- [Cephes](https://www.netlib.org/cephes/), Stephen L. Moshier, math functions as a replacement to avoid linking with the CRT.

- [tlsf](https://github.com/mattconte/tlsf), Matthew Conte (http://tlsf.baisoku.org), Two-Level Segregated Fit memory allocator, version 3.1.
>> Based on the original documentation by Miguel Masmano:
>>	http://www.gii.upv.es/tlsf/main/docs
```cpp
// Here is the appropriate license:

// This implementation was written to the specification
// of the document, therefore no GPL restrictions apply.

/* 
 * Copyright (c) 2006-2016, Matthew Conte
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MATTHEW CONTE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

- Rolf Neugebauer, 2003, `sccanf` implementation.
```cpp
// Here is the appropriate license:
/*
 ****************************************************************************
 * (C) 2003 - Rolf Neugebauer - Intel Research Cambridge
 ****************************************************************************
 *
 *        File: printf.c
 *      Author: Rolf Neugebauer (neugebar@dcs.gla.ac.uk)
 *     Changes: Grzegorz Milos (gm281@cam.ac.uk) 
 *
 *        Date: Aug 2003, Aug 2005
 *
 * Environment: Xen Minimal OS
 * Description: Library functions for printing
 *              (Linux port, mainly lib/vsprintf.c)
 *
 ****************************************************************************
 */

/*
 * Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */

/*
 * Fri Jul 13 2001 Crutcher Dunnavant <crutcher+kernel@datastacks.com>
 * - changed to provide snprintf and vsnprintf functions
 * So Feb  1 16:51:32 CET 2004 Juergen Quade <quade@hsnr.de>
 * - scnprintf and vscnprintf
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
```

- [delegate](https://github.com/tarigo/delegate), Vadim Karkhin, 2015
```cpp
// Here is the appropriate license:
/* The MIT License (MIT)
 * 
 * Copyright (c) 2015 Vadim Karkhin
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
```

- Ryan Juckett, 2014, implementation the Dragon4 algorithm (used in the `fmt` module for formatting floats).

> See the following papers for more information on the algorithm:
>  "How to Print Floating-Point Numbers Accurately"
>    Steele and White
>    http://kurtstephens.com/files/p372-steele.pdf
>  "Printing Floating-Point Numbers Quickly and Accurately"
>    Burger and Dybvig
>    http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.72.4656
> 
> 
> It is a modified version of numpy's dragon4 implementation,
> ... which is a modification of Ryan Juckett's version.

```cpp
// Here are the appropriate licenses:
/*
 * Copyright (c) 2014 Ryan Juckett
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * Copyright (c) 2005-2021, NumPy Developers.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials provided
 *        with the distribution.
 * 
 *     * Neither the name of the NumPy Developers nor the names of any
 *        contributors may be used to endorse or promote products derived
 *        from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
```

### Projects using this library

- [light-std-graphics](https://github.com/Repertoi-e/light-std-graphics) - high-level windowing API (like GLFW) and a high-level graphics API (currently has only DirectX bindings).
- [Physics engine](https://repertoi-e.github.io/Portfolio/Simulating-Rigid-Body-Physics.html) - simulating rigid bodies.

