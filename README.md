
# light-std
A C++20 data-oriented library replaces the standard C++ library.

With the goal of providing a stripped-down alternative to the standard C/C++ library. The library is designed to be self-contained, meaning it does not rely on any headers from the default standard library, and does not link with the runtime library.

## Why

TLDR; the philosophy of this library follows a data-oriented approach.

> Ulrich Drepper, What Every Programmer Should Know About Memory, 2007 - https://people.freebsd.org/~lstewart/articles/cpumemory.pdf

Modern CPUs rely heavily on efficient memory layout, making it crucial for programmers to consider this aspect when writing code. High-level languages, including "Modern C++," often abstract memory management, which results in slower software. Hardware has gotten blazingly fast, but software has deteriorated drastically. Just following principles of "clean code" can [erase 4-14 years of hardware evolution](https://www.computerenhance.com/p/clean-code-horrible-performance?utm_source=post-email-title&publication_id=865289&post_id=102168145)!

Writing readable, well-documented, and performant code is possible without resorting to low-level instructions or assembly. The modern emphasis on clean code can sometimes miss the mark, as it often focuses on aesthetics and abstractions rather than addressing the core issues at hand. Object-oriented programming and extensive abstraction layers might appear elegant, but as I don't see anybody measuring the claims that are so vigorously defended, in contrast: performance cost of such an approach is quantifiable and can be significant.

Despite modern CPUs being capable of performing billions of calculations per second, reading a single byte from RAM can take hundreds of clock cycles if the memory isn't in the cache. This highlights the importance of thinking about cache usage. When programmers adopt a data-oriented mindset, they prioritize structuring programs based on how computers process data. This means keeping data that is processed together in close proximity within memory.

## This library currently provides:

- A memory model inspired by Jonathan Blow's Jai - simple overridable allocators with an implicit global thread-local context variable, including a debug memory switch that has checks for double frees, cross-thread frees, block overlaps from allocations and etc.
- Support for memory arenas, inspired by [Ryan Fleury](https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator).
- Utf-8 non-null-terminated string with unicode support.
- Linked-list "hygienic macros" using C++'s concepts to access a structure's "Next" and "Prev" fields.
- Simple, fast and hackable dynamic array, [exponential array (xar)](https://azmr.uk/dyn/), hash table etc.
- `os` module - common operations that require querying the OS.
- `path` module - procedures that work with Windows and Unix file paths. 
- `fmt` module - a formatting library inspired by Python's formatting syntax, prints faster than printf.
- `parse` module - for parsing strings, integers, booleans, GUIDs.
- Threads, mutexes, atomic operations.
- Console input/output.

## Build

You need a compiler which fully supports C++20 concepts. Visual Studio 2019 16.11 works fine. I've also tested Clang 14.

This project uses Tsoding's [nob.c](https://github.com/tsoding/nob.h) as its build system - a single-file build script that automatically rebuilds itself when modified.

### All Platforms (Recommended)

1. **Bootstrap the build system** (one time only):
   ```bash
   cc -o nob nob.c        # On Unix-like systems 
   cl nob.c               # On Windows with MSVC
   ```

2. **Build the library**:
   ```bash
   ./nob                  # Default release build
   ./nob debug            # Debug build with bounds checking
   ./nob optimized        # Debug build with optimizations
   ./nob release          # Release build
   ```

The `nob` executable will automatically rebuild itself if you modify `nob.c`, so you only need to bootstrap it once.

### Platform-Specific Notes

#### Windows
On Windows we try our hardest to avoid linking with the CRT and provide common functionality to external libraries that may require it (for example functions like `strncpy`, defined in `lstd/platform/windows/no_crt/common_functions.h/.cpp`).

#### Linux & Mac
We can't not-link with `glibc`, because it's coupled with the POSIX operating system calls library, although in my opinion they really should be separate. For example, how can we be sure we get the same answer to a math function (e.g. `sin`) since virtually every compiler and platform may define its own version. Insane.

## Zen 

- **Less code is better**
> Every line of code is a liability and a possible source of bugs and security holes. Automatic package managers are a curse.

- **Closer to C than to modern C++**
> Ditch copy/move constructors, destructors, exceptions. This reduces the amount of abstractions and bloat code and increases confidence that the code is doing what is expected.

- **Code reusability**
> Using C++20 features: terse templates, concepts, we can do conditional procedure compilation and combine functions in ways to reduce code. Examples: `parse.h` and `array_like.h`. Conditional procedure compilation also leads to performance improvement (even in Debug), and in C++20 it doesn't require ugly macro magic which makes code difficult to reason about.

## Documentation

### Type policy

#### Ideal
- Keep it simple and POD (plain old data). Every structure should work when memcpy-ed 
- Use `struct` instead of `class`, and keep everything public.
- Provide a default constructor that does minimal work.
- Don't use copy/move constructors, no destructors. To implement "destructor functionality" (uninitializing the type, freeing members, etc.), provide a function, such as:
>```cpp
>    free(string ref s) {
>        free(s.Data);   // Free buffer as normal
>        atomic_add(&GlobalStringCount, -1);
>    }
>```
- Never throw exceptions. Instead, return multiple values using structured bindings (C++17).
  They make code complicated. When you can't handle an error and need to exit from a function, return multiple values.
>          auto [content, success] = os_read_entire_file("data/hello.txt");
- In general, error conditions (which require returning a status) should be rare. The code should just do the correct stuff. Using exceptions leads to this mentality of "giving up and passing the responsibility to handle error cases to the caller". However, that quickly becomes complicated and confidence is lost on what could happen and where. Code in general likes to grow in complexity combinatorially as more functionality is added, if we also give up the linear structure of code by using exceptions then that's a disaster waiting to happen.

#### Example
The array in this library is a basic wrapper around contiguous memory with three fields (Data, Count, and Allocated). It does not have ownership; the programmer determines it explicitly.

By default, arrays are views. To make them dynamic and owned, call `reserve(arr)` or `make_array(...)`. Arrays can be passed around and returned by value, as they have no hidden destructor and are cheap to copy. To free, call `free(arr)` or use `defer(free(arr))`. The latter runs like a destructor, but it's written *explicitly* in code.

> ```cpp
> array<string> path;
> defer(free(path));
>
> add(path, "./");
> add(path, "build");
> ...
> ```

To make a deep copy of an array, use clone(): `newPath = clone(path)`.

> ```cpp
> string path = make_string("./data/"); 
> // Equivalent to 
> //     string path = "./data/"; 
> //     reserve(path);
>
> defer(free(path));
>
> add(path, "output.txt");
> string pathWithoutDot = slice(path, 2, -1); // No allocations
> ```

strings behave like arrays but have different types to avoid conflicts. They take indices to code points (as they are UTF-8 by default) and are not null-terminated, so substrings don't allocate memory. 

### Array Examples

> ex.1
> ```cpp
>      byte data[100];
>      auto subArray = array<byte>(data + 20, 30); 
> 
>      auto shallowCopy = subArray;
>      // Both subArray and shallowCopy point to the same 
>      // memory location, sharing the same underlying data.
> ```

> ex.2 Allocating memory explicitly.
>
> ```cpp
>      array<char> sequence;
>      reserve(sequence); // by default space for 8 elements is reserved
>      sequence += {'0', 'b', '1'};
>
>      // Or equivalently:
>      // add(sequence, '0');
>      // add(sequence, 'b');
>      // add(sequence, '1');
>      // ... futher appends may require reallocating
>      // which is handled automatically (next power of 2 of the current allocated size)
>  
>      array<char> otherSequence = clone(sequence);  // Deep-copy
>      defer(free(otherSequence));                   // Runs at the end of the scope
>  
>      auto shallowCopyOfSequence = sequence;
>      free(shallowCopyOfSequence); // 'sequence' is also freed (they point to the same memory)
>  
>      // Here 'sequence' is no longer valid, but 'otherSequence' (which was deep copied) still is.
>  
>      // Attempting to free a subarray is guaranteed to trip an assert and crash
>      // in our debug memory layer, if the define switch is turned on.
>      auto subData = array<byte>(otherSequence.Data + 1, 2);
>      free(subData.Data); 
> ```

> ex.3 Here `PERSISTENT` is an allocator that the platform layer uses for internal allocations. The example is taken directly from the `os` module.
> ```cpp
> void parse_arguments() {
>     wchar **argv;
>     s32 argc;
> 
>     argv = CommandLineToArgvW(GetCommandLineW(), &argc);
>     if (argv == null) {
>         report_warning_no_allocations("Couldn't parse command line arguments, os_get_command_line_arguments() will return an empty array in all cases");
>         return;
>     }
> 
>     defer(LocalFree(argv));
> 
>     // Although reserve is getting called without an explicit
>     // allocator, it get's the one from the context (pushed PERSISENT)
>     PUSH_ALLOC(PERSISTENT) {
>         s32 n = argc - 1;
>         if (n > 0) {
>             reserve(S->Argv, n);
>         }
>
>         // Loop over all arguments and add them
>         For(range(0, argc)) add(S->Argv, utf16_to_utf8(argv[it]));
>     }
> }
> ```

> ex.4 We also provide some syntactic sugar: terse `for` macros and a Python-like `range` function.
> ```cpp
>      array<s32> integers;
>
>      for (s32 it : range(50)) { // Means "for every integer in [0, 50)" 
>          add(integers, it);
>      }
>
>      for (s32 it : range(40, 20, -2)) { // Every second integer in [40, 20) in reverse
>          add(integers, it);
>      }
>
>      For(range(10, 20, 3)) { // Every third integer in [10, 20).. you get the idea
>          add(integers, it);
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
>      For(args) print("{}\n", it); 
>     
>
>      For_enumerate(args) {
>          // Here we have 2 implicit variables - 'it' 
>          // and 'it_index', which contain the 
>          // current object and its respective index.
>
>          if (it == "--option") {
>              if (it_index + 1 >= args.Count) continue;
>              value = args[it_index + 1];
>              continue;
>          }
>      }
>
>      // Here you can also specify custom variable names
>      // For_enumerate_as(my_it_index_name, my_it_name, args) { .. }
> ```

> ex.5 We also have a wrapper for a fixed-size array.
> ```cpp
>      s32 data[5];               
>      stack_array<s32, 5> data;
>      // These above two variables have the same size, 
>      // but the second one can be passed as a parameter
>      // to functions without decaying to a pointer.
>
>      s64 data[] = {1, 2, 3, 4, 5};
>      auto data = make_stack_array<s64>(1, 2, 3, 4, 5);
> ```

> ex.6 Arrays and strings support Python-like negative indexing.
> ```cpp
>      auto nums = make_stack_array(1, 2, 3, 4, 5); 
>
>      // Negative index is the same as "Length - index", 
>      // so -1 is translated to the index of the last element.
>      print("{}", nums[-1]); // 5
> ```

### Strings Examples

Strings in this library support unicode by default (UTF-8) and aren't null-terminated.

`str.Count` tells you the number of bytes stored in the string. To get
the number of code points call `length(str)`, which has to calculate 
the UTF-8 length by iterating the string. The number of code points is 
smaller or equal to the number of bytes, because a certain code point 
can be encoded in up to 4 bytes.

Strings are compared using `strings_match` and not with `==` since 
the latter in C/C++ has the convention of comparing the string pointers.

Array examples from the previous section are relevant here.

> ex.1 This string is constructed from a zero-terminated string buffer (which the compiler stores in read-only memory when the program launches). 
> This doesn't allocate memory, `free(path)` or `free(path.Data)` will crash.
> ```cpp
>     string path = "./data/"; 
> ```

> ex.2 Substrings don't cause allocations.
> ```cpp
>     string path = "./data/output.txt";
>     
>     // Reverse search to find the last dot
>     s64 dot = search(path, '.', .Start = length(path), .Reversed = true);
>     string pathExtension = slice(path, dot, -1); // ".txt"    
> ```

> ex.3 This example is taken directly from the `path` module.
> ```cpp
> mark_as_leak array<string> path_split_into_components(string path, string seps = "\\/") {
>     array<string> result;
>     reserve(result);
> 
>     s64 start = 0, prev = 0;
>
>     auto matchSep = [=](code_point cp) { return has(seps, cp); };
>     while ((start = search(path, &matchSep, .Start = start + 1 )) != -1) {
>       result += { slice(path, prev, start) };
>       prev = start + 1;
>     }
> 
>     // There is an edge case in which the path ends with a slash, 
>     // in that case there is no "another" component. The if is
>     // here so we don't crash with index out of bounds.
>     //
>     // Note that both /home/user/dir and /home/user/dir/ mean the 
>     // same thing. You can use other functions to check if the former
>     // is really a directory or a file (querying the OS).
>     if (prev < length(path)) {
>       // Add the last component - from prev to path.Length
>       result += { slice(path, prev, length(path)) };
>     }
>     return result;
> }
> ```

> ex.4 Also taken from the `path` module.
> ```cpp
> mark_as_leak string path_join(array<string> paths) {
>     assert(paths.Count >= 2);
> 
>     auto [result_drive, result_path] = path_split_drive(paths[0]);
> 
>     string result = clone(result_path);
> 
>     For(range(1, paths.Count)) {
>         auto p = paths[it];
>         auto [p_drive, p_path] = path_split_drive(p);
>         if (p_path.Count && path_is_sep(p_path[0])) {
>             // Second path is absolute
>             if (p_drive.Count || !result_drive.Count) {
>                 result_drive = p_drive;  // These are just substrings so it's fine
>             }
> 
>             free(result);
>             result = clone(p_path);
> 
>             continue;
>         } else if (p_drive.Count && !strings_match(p_drive, result_drive)) {
>             if (!strings_match_ignore_case(p_drive, result_drive)) {
>                 // Different drives => ignore the first path entirely
>                 result_drive = p_drive;
> 
>                 free(result);
>                 result = clone(p_path);
> 
>                 continue;
>             }
>             // Same drives, different case
>             result_drive = p_drive;
>         }
> 
>         // Second path is relative to the first
>         if (result.Count && !path_is_sep(result[-1])) {
>             result += '\\';
>         }
>         result += p_path;
>     }
> 
>     // Add separator between UNC and non-absolute path if needed
>     if (result.Count && !path_is_sep(result[0]) && result_drive.Count && result_drive[-1] != ':') {
>         insert_at_index(result, 0, '\\');
>     } else {
>         insert_at_index(result, 0, result_drive);
>     }
>     return result;
> }
> ```

### Memory Examples

Avoid using new and delete for several reasons:

1) The syntax is ugly.
2) Care must be taken not to mix new with delete[]/new[] with delete.
3) It's an operator and can be overriden by structs/classes.
4) Modern C++ programmers also recommend against using new/delete, so ... 

There are two primary ways to allocate memory in C++: malloc and new. To prevent the introduction of a third method, we override malloc.

new and delete have different semantics (new initializes values, delete calls a destructor if defined). Although our type policy discourages destructors, we will call them just in case, as some third-party libraries might rely on them.

So we provide templated versions of malloc/free:

`malloc<T>`:
- Calls constructors on non-scalar values.
- Returns T* so casting from void* is not necessary.
- Cannot call with T == void (use non-templated malloc instead).

`realloc<T>`
- Assumes your type can be byte-by-byte copied to another location in memory and still work.
- Does not call copy/move constructors.

`free<T>`
- Calls the destructor, but note that we generally discourage destructors in favor of explicit code.

#### Context and Allocators 

The Context is a global, thread-local variable containing options that alter the program's behavior.

It stores the default allocator used for allocating new blocks, the logger for printing messages (pointer to a writer, e.g., cout), and other elements.

Here's a part of the structure (the full structure is defined in context.h):
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

The context is initialized when the program runs and when creating a thread with our API. In the second case, we know the parent thread, so we copy the options from it. If the thread is created in another way, we leave it zero-initialized and let the caller decide what should go inside it. (As far as I know, there's no way to identify the parent thread with other APIs., again in the future we might try to hijack OS API calls with symbol patching, to always ensure a proper context).

> ex.1 
> ```cpp
> void *memory;
>
> // Using Context.Alloc (make sure you have set it beforehand)
> memory = malloc<char>({.Count = 150});   
> 
> // Uses another allocator explicitly
> memory = malloc<char>({.Count = 150, .Alloc = TemporaryAllocator}); 
> 
> // You can also implement custom allocators:
> // my_allocator_function is a function which implements 
> // all the functionality of the allocator (allocating, resizing, freeing, etc.)
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
>
>     // Mark explicitly with LEAK so the debug memory layer excludes this from a leak report.
>     void *memory = malloc<byte>({.Count = 200, .Alloc = TemporaryAllocator, .Alignment = 64, .Options = LEAK});
> ```

> ex.3 Example taken from the test suite. Demonstates temporary allocator initialization and how to change the allocator which is used in a scope.
> ```cpp
>     time_t start = os_get_time();
> 
>     // Initialize the temporary allocator by asking the OS for large arena.
>     s64 TEMP_ALLOC_SIZE = 1_MiB;
>     TemporaryAllocatorData.Block = os_allocate_block(TEMP_ALLOC_SIZE); // Does VirtualAlloc/mmap
>     TemporaryAllocatorData.Size  = TEMP_ALLOC_SIZE;
>
>     // Arenas can be left 0 initialized and by default, upon first usage, 
>     // they call os_allocate_block(64_GiB) anyway.
>     // So here TemporaryAllocatorData could've been left alone.
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

> ex.4 Example taken from a physics engine simulation with `light-std-graphics`. This demonstrates how to allocate global state + a pool and initialize a common general purpose allocator at the beginning of your program.
> ```cpp
>     // We allocate a large block for our memory needs.
>     void *memory_pool = os_allocate_block(MemoryInBytes);
>     
>     // Set up the TLSF allocator data at the start of our memory pool
>     auto *data = (tlsf_allocator_data*)memory_pool;
>     auto *pool = (byte*)memory_pool + sizeof(tlsf_allocator_data);
>     s64 pool_size = MemoryInBytes - sizeof(tlsf_allocator_data);
>
>     // TLSF (Two-Level Segregate Fit) is a fast general purpose dynamic memory allocator for real-time systems.
>     // We provide an implementation directly in the library.
>     PersistentAlloc = {tlsf_allocator, data};
>     allocator_add_pool(PersistentAlloc, pool, pool_size);
>
>     auto newContext = Context;
>     newContext.Log = &cout;
>     newContext.Alloc = PersistentAlloc;
>
>     // This macro doesn't change the context in a scope but 
>     // instead overrides the parameter all-together. 
>     // Use this only at the beginning of your program!
>     // Never use this inside functions since the caller will get confused.
>     OVERRIDE_CONTEXT(newContext); 
> ```

> ex.5 How to implement custom allocators. Here is the implementation of the arena allocator.
> ```cpp
> void *arena_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
>     auto *data = (arena_allocator_data *)context;
>     if (!data->Block) {
>        void *os_allocate_block(s64);
>
>        data->Block = os_allocate_block(8_GiB);
>        data->Size = 8_GiB;
>        data->Used = 0;
>     }
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

> See `"memory.h"`.

#### TLSF

Generally malloc implementations do the following:
- Have seperate heaps for different sized allocations
- Call OS functions for very large allocations
- Use different algorithms for allocation, e.g., stb_malloc implements the TLSF algorithm for O(1) allocation.

Here, we provide a wrapper around the TLSF algorithm. To use it, allocate a large block with the OS allocator (that's usually how everything starts) and call `allocator_add_pool()` on the TLSF (`ex.4` from the previous section).

You can write a general-purpose allocator and do what stb_malloc does for different sized allocations or have several TLSF specialized allocators with different blocks. Both are equivalent. We try to push you to think about how memory in your program should be structured together, and having a general-purpose allocator for every type of allocation is not ideal, as it just sweeps everything under the rug.

```cpp
// Two-Level Segregated Fit memory allocator implementation. 
// Wrapper around tlsf.h/cpp (in vendor folder),
// written by Matthew Conte (matt@baisoku.org). 
// Released under the BSD license.
//
// * O(1) cost for alloc, free, resize
// * Extremely low overhead per allocation (4 bytes)
// * Low overhead per TLSF management of pools (~3kB)
// * Low fragmentation
//
void *tlsf_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);
```

#### Arena

This type of allocator is super fast because it basically bumps a pointer. With this allocator, you don't free individual allocations but instead free the entire thing (with `free_all()`) when you are sure nobody uses the memory anymore. Note that `free_all()` doesn't free the added block, but instead resets its pointer to the beginning of the buffer.

The arena allocator doesn't handle overflows (when the block doesn't have enough space for an allocation). When out of memory, you should resize or provide another block.

```cpp
void *arena_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);
```

The temporary allocator is just a variation of the arena allocator. It's placed in global scope and is thread-local. The two variables of interest are `TemporaryAllocator` and `TemporaryAllocatorData`. See `ex.3` from the previous section to see how to initialize an arena allocator.

#### Pool allocator 

This is a variation of the fast bump arena allocator. It allows O(1) allocation and freeing of individual elements.

The limitation is that each allocation must have the same predefined size. This allocator is useful for managing a bunch of objects of the same type.

Internally it keeps a linked list of added blocks. To provide a block, call `pool_allocator_provide_block(.., block, size)`. Instead of allocating a linked list at another place in memory, the pool uses the first few bytes of the provided block as a header.

```cpp
void *pool_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);
```

### Credits

The appropriate licenses are listed alongside this list in the file `LICENSE.md`.

- [Cephes](https://www.netlib.org/cephes/), Stephen L. Moshier, math functions as a replacement to avoid linking with the CRT.
- [tlsf](https://github.com/mattconte/tlsf), Matthew Conte (http://tlsf.baisoku.org), Two-Level Segregated Fit memory allocator, version 3.1.
- [delegate](https://github.com/tarigo/delegate), Vadim Karkhin, 2015
- Ryan Juckett, 2014, implementation the Dragon4 algorithm (used in the `fmt` module for formatting floats).
- [minlibc]( https://github.com/GaloisInc/minlibc), Galois Inc., 2014, `strtod` and `atof` implementations.
- [LIBFT](https://github.com/beloff-ZA/LIBFT/blob/master/libft.h) beloff, 2018, some implementations of functions found in the CRT.
- Rolf Neugebauer, 2003, `sccanf` implementation.

### Projects

- [light-std-graphics](https://github.com/Repertoi-e/light-std-graphics) - high-level windowing API (like GLFW) and a high-level graphics API.
- [Physics engine](https://soti.camp/blog/simulating_rigid_body_physics/) - tutorial about simulating rigid bodies (and using `light-std-graphics` for demos).

