
# light-std
A C++20 library created for personal use that aims to replace the standard C/C++ library. It's performance-oriented and designed for general programming.

This library is supposed to be a replacement of C/C++'s standard library in but designed entirely differently. 

It is completely stand-alone - it doesn't include ANY headers from the default standard library. Some C++ language features (like the spaceship operator, initializer lists, etc.) require certain definitions in the std:: namespace, but we provide our own placeholders (tested on the MSVC compiler). 

## Manifest

Memory layout is very important for modern CPUs. The programmer should be very aware of that when writing code.

C++ is a low-level language (was high-level in old days when ASM was low-level, but now we consider it low-level).
Usually modern high-level languages put much of the memory management behind walls of abstraction.
Somewhere in all that progress of abstraction we began writing very very slow software. Hardware has gotten thousands
times faster but software has kept up with the pace and it doesn't feel like we are using supercomputers anymore. Take a 
look at a complicated modern piece of software, it's not hard to find examples. 

I am writing this in 2021 - in the midst of the COVID-19 crisis. We have been using online video conferencing software for school
and work for a year now. The worst software I have ever iused is Microsoft Teams. Clicking a button takes a good second to open a chat. 
It lags, it's buggy. No. Your computer is not slow. Your computer is a super-computer compared to what people had 30-40 years ago.

Note: Games are an exception to this trend, because engine programmers always try to push the hardware limits.  

Hardware has gotten blazingly fast, but software has deteriorated.
Modern CPUs can make a million billion calculations per second, but reading even ONE byte from RAM can take hundreds of clock cycles
(if the memory is not in the cache). You MUST think about the cache if you want to write fast software.

Once you start thinking about the cache you start programming in a data oriented way.
You don't try to model the real world in the program, but instead structure the program in the way that
the computer will work with the data. Data that is processes together should be close together in memory.

And that's why we should remove some of the abstraction. We should think about the computer's memory.
We should write fast software. We can slow down global warming by not wasting CPU clock cycles.

Of course, abstractions which enable us more rapid programming are not always bad. After all we must get work done and we can't
reinvent everything from scratch every time. But being too careless results in the modern mess of software that wastes most of CPU time
doing nothing, because people decided to abstract stuff too much.

Not only you waste electricity by being a careless programmer, you also waste USER'S TIME!
If your program is used by millions of PC, 1 second to click a SIMPLE BUTTON quickly becomes hours and then days.

This library is a call out and an attempt to bring people's attention to these problems.
It's designed around a different *culture* than normal C++ programming.

## What this library provides

- Memory model (inspired by Jonathan Blow's Jai) - implicit context system, overridable allocators.
- Data structures - utf8 string, dynamic array, hash table, etc.
- `os` module - common operations that require querying the OS.
- `path` module - robust procedures that work with files and file paths. 
- `fmt` module - a formatting library inspired by Python (that's even faster than printf).
- `math` module - vectors and matrices and operations with them.
- Threads, mutexes, atomic operations, lock-free data structures.
- Some of the most-useful templates used for metaprogramming (to avoid including the STL).

## Principles 

- **Clean code.**
> Readibility is most important. Comments are a powerful tool. 

- **Less code is better.**
> Every line of code is a liability and a possible source of code. Avoid big dependencies, 
> if you need just one function - write it. Don't import a giant library.

- Closer to C than to C++.
> We ditch copy/move constructors, destructors, exceptions. This dramatically reduces the amount of bloat code.

- Code reusability. 
> A trick: conditional procedure compilation. You can enable or disable features for a function depending
> on template parameters. e.g. our `parse_int` function - one piece of code but it suits many cases.
> It performs as if it's a specialized function, even though it's very general.

> Another trick: with C++20 concepts we can implement one set of functions that work with all arrays (`array_like.h`).
> The following: search functions (`find`, `find_not`, `has`, etc.), `compare`, and operators `==`, `!=`, `<`, `<=`, `>`, `>=`
> are written once (cleanly) and work with all array-like data structures (even comparing different types of arrays works, for 
> which otherwise we need a combinatorial amount of code).

- Designed with the future in mind.
> Some sacrifices are made. We aren't backwards compatible, we use C++20 `modules`, `concepts`, C++17 `if constexpr`, etc.

## Documentation

### Type policy

#### Aim of this policy
- Dramatically reduce complexity and code size (both library AND user side!) UNLESS that comes at a run-time cost

#### Guidelines
- Always provide a default constructor (implicit or by `T() = default`)
- Every data member (which makes sense) should be public. Do not write useless getters/setters.
- Strive to make classes/structures/objects (whatever you wanna call them) data oriented.
  Programs work with data. Design your data so it makes the solution straightforward and minimize abstraction layers.
- No user defined copy/move constructors.
- Avoid destructors. Detach objects from memory ownership - memory should be managed explictly by the programmer.
- No throwing of exceptions, .. ever, .. anywhere. No excuse.

#### "No copy/move constructors/destructors":
_This may sound crazy if you have a type that is supposed to own memory. How would you deep copy the contents and not just the pointer when you copy the object? 
How do you handle dangling pointers to unfreed memory of shallow copies when the original destructor fires?_

Here's how we implement objects that deal with dynamically allocated buffers (like strings, arrays, etc.)

- Objects are designed with no ownership in mind. That is determined explictly in code and by the programmer. `string`'s destructor is EMPTY. It doesn't free any buffers. The call site is responsible for freeing the string when that is required. We make this easier by providing a `defer` macro which runs at the end of the scope (exactly like a destructor).
- A function which returns object has either allocated memory during it's run or it has not. We make this very clear by marking procedures as `[[nodiscard("Leak")]]` (which means that the returned object should be freed, the compiler issues a warning if the result is discarded).
- Since objects don't own memory, we don't have to write copy/move constructors and assignment operators.

- Memory is always freed with a `free` function, which should be overload for a struct that may own memory. i.e. `free` works both for pointers - in that case we simply call the allocator free, or for objects (passed by reference) - in that case any memory owned by the object is freed.

### Arrays by example

> ex.1 Arrays as views. Constructing an array object doesn't allocate any memory (since it's detached from the idea of ownership).
> ```cpp
>      byte data[100];
>      auto subData = array<byte>(data + 20, 30); 
> 
>      auto shallowCopy = subData; // This also applies for copying arrays
> ```

> ex.2 Arrays may allocate memory if they are modified. Memory is managed explicitly.
> ```cpp
>      array<char> sequence;
>      array_append(sequence, '0'); // First block is allocated here (space for 8 elements by default)
>      array_append(sequence, 'b');
>      array_append(sequence, '1');
>      // ... futher appends may require reallocation (that works as expected)
>  
>      array<char> otherSequence;
>      clone(&otherSequence, sequence);  // Deep-copy
>      defer(free(otherSequence));       // Runs at the end of the scope
>  
>      auto shallowCopy = sequence;
>      free(shallowCopy); // 'sequence' is also freed (they point to the same memory)
>  
>      // Here 'sequence' is no longer valid, but 'otherSequence' still is.
>  
>      // Attempting to free a subarray is undefined.
>      // It is guaranteed to crash the allocator
>      auto subData = array<byte>(otherSequence.Data + 20, 30);
>      free(subData); 
> ```

-  `clone(T *dest, T src)` is a global function that ensures a deep copy of the argument passed. Objects like `array` and `string` overload `clone()` and make sure a deep clone is done. This is like a classic copy constructor but much cleaner. Note: `clone` works on all types (unless overloaded the default implementation does a shallow copy). It is this library's recommended way to implement functionality normally written in copy c-tors.

> ex.3 Arrays allocate functions by the implicit context allocator (more on that later). 
> Here 'PERSISTENT' is an allocator that the platform layer uses for internal allocations (example is taken from there).
> ```cpp
> [[nodiscard("Leak")]] array<string> get_command_line_arguments() {
>     s32 argc;
>     utf16 **argv = CommandLineToArgvW(GetCommandLineW(), &argc); // Win32 API
>   
>     if (argv == null) return {}; // We may return an empty array safely, because `free` on an object 
>                                  // that hasn't allocated memory does nothing.
>  
>     array<string> result;
>     PUSH_ALLOC(PERSISTENT) {
>         array_reserve(result, argc - 1);
>
>         // Loop over all arguments but skip the .exe name
>         For(range(1, argc)) {
>             array_append(result, utf16_to_utf8(argv[it]));
>         }
>     }
>     return result;
> }
> ```

> ex.4 We also provide some syntactic sugar: terse `for` macros and a Python-like `range` function. Implicit variable names can be used with extended macros.
> ```cpp
>      array<s32> integers;
>
>      for (s32 it : range(50)) { // Every integer in [0, 50) 
>          array_append(integers, it);
>      }
>
>      for (s32 it : range(40, 20, -2)) { // Every second integer in [40, 20) in reverse
>          array_append(integers, it);
>      }
>
>      For(range(10, 20)) { // Every integer in [10, 20).. you get the idea
>          array_append(integers, it);
>      }
>
>
>      // Avoid variable name collisions
>      For_as(i, range(10)) { 
>          For_as(j, range(20)) { .. } 
>      }
>      
>
>      array<string> args = get_command_line_arguments();
>      For(args) print("{}\n", it); // Debug
>     
>      For_enumerate(args) {
>          // Here we have 2 implicit variables - 'it' and 'it_index', which contain the current object and it's respective index.
>
>          if (it == "--option") {
>              value = args[it_index + 1];
>              continue;
>          }
>      }
>
>      // For_enumerate_as(my_it_index_name, my_it_name, args) { .. }
> ```

> ex.5 Arrays support Python-like negative indexing.
> ```cpp
>      array<s32> integers;
>      append_array(integers, {1, 2, 3, 4, 5});
>
>      // Negative index is the same as "Length - index", 
>      // so -1 is translated to the index of the last element.
>      print("{}", integers[-1]); // 5
> ```

> ex.6 We also have a wrapper for a fixed-size array (equivalent to std::array).
> ```cpp
>      s32 data[5];               // These two variables have the same size, but the second one
>      stack_array<s32, 5> data;  // can be passed as a parameter to functions without decaying to a pointer.
>
>      s64 data[] = {1, 2, 3, 4, 5};
>      auto data = to_stack_array<s64>(1, 2, 3, 4, 5);
> ```

> ex.7 Arrays are mostly constexpr (except modifying functions - `array_insert`, `array_remove_at`, `array_append`, ..., which require an allocation)
> ```cpp
>      constexpr auto data = to_stack_array<s64>(1, 2, 3, 4, 5);
>      constexpr array<s64> view = data;
> ```


### Strings by example

Strings in this library support unicode (UTF-8) and aren't null-terminated.
They are just arrays of `utf8` - a typedef for `char8_t` (an unsigned byte).

A string also stores the pre-calculated amount of code points in the string.
The number of code points is smaller or equal to the number of bytes in string,
because a certain code point can be encoded in up to 4 bytes.

Array examples from the previous section are relevant here.

Note: Functions like `array_append` have a different suffix for strings `string_append`, because we need to keep track of the code point length. This is subject to change in the future.

> ex.1 This string is constructed from a zero-terminated string buffer (which the compiler stores in read-only memory when the program launches). 
> This doesn't allocate memory, `free(path)` will do nothing.
> ```cpp
>         string path = "./data/"; 
> ```

> ex.2 Substrings don't cause allocations.
> ```cpp
>     string path = "./data/output.txt";
>     
>     s64 dot = find_cp(path, '.', true);        // Reverse find
>     string pathExt = substring(path, dot, -1); // ".txt"    
> ```

> ex.3 Modifying individual code points.
> ```cpp
>     string greeting = "ЗДРАСТИ";
>     For(greeting) {
>         it = to_lower(it);
>         // Here the string is non-const so 'it' is actually a reference 
>         // to the code point in the string, so you can modify it directly.
>     }
>     greeting; // "здрасти"
> ```

> ex.4 Taken from "path.general.ixx"
> ```cpp
> [[nodiscard("Leak")]] array<string> path_split_into_components(const string &path) {
>     array<string> result;
>     s64 start = 0, prev = 0;
>     while ((start = find_any_of(path, "\\/", start + 1)) != -1) {
>         array_append(result, path(prev, start));
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
>         array_append(result, substring(path, prev, path.Length));
>     }
>     return result;
> }
> ```

> ex.4 Taken from "path.nt.ixx"
> ```cpp
> [[nodiscard("Leak")]] string path_join(const array<string> &paths) {
>     auto [result_drive, result_path] = path_split_drive(paths[0]);
> 
>     string result;
>     clone(&result, result_path);
> 
>     For(range(1, paths.Count)) {
>         auto p = paths[it];
>         auto [p_drive, p_path] = path_split_drive(p);
>         if (p_path && path_is_sep(p_path[0])) {
>             // Second path is absolute
>             if (p_drive || !result_drive) {
>                 result_drive = p_drive;  // These are just substrings so it's fine
>             }
>             clone(&result, p_path);
>             continue;
>         } else if (p_drive && p_drive != result_drive) {
>             if (compare_ignore_case(p_drive, result_drive) != -1) {
>                 // Different drives => ignore the first path entirely
>                 result_drive = p_drive;
>                 clone(&result, p_path);
>                 continue;
>             }
>             // Same drives, different case
>             result_drive = p_drive;
>         }
> 
>         // Second path is relative to the first
>         if (result && !path_is_sep(result[-1])) {
>             string_append(result, '\\');
>         }
>         string_append(result, p_path);
>     }
> 
>     // Add separator between UNC and non-absolute path if needed
>     if (result && !path_is_sep(result[0]) && result_drive && result_drive[-1] != ':') {
>         string_insert_at(result, 0, '\\');
>     } else {
>         string_insert_at(result, 0, result_drive);
>     }
>     return result;
> }
> ```

### Context and allocators by example

The `Context` is a global thread local variable that contains certain variables that change the behaviours of the program.

It stores the allocator and the alignment which is used for allocating new blocks, the logger which is used to print messages (pointer to a writer, e.g. `cout`).

Here is the full structure (defined in `"internal/context.h"`): 
```cpp
// @Volatile
struct context {
    thread::id ThreadID;  // The current thread's ID (Context is thread-local)

    //
    // :TemporaryAllocator: Take a look at the docs of this allocator in `"allocator.h"`
    // (or the allocator module if you are living in the future).
    //
    // We store a arena allocator in the Context that is meant to be used as temporary storage.
    // It can be used to allocate memory that is not meant to last long (e.g. returning arrays or strings from functions
    // that don't need to last long and you shouldn't worry about freeing it - e.g. converting utf8 to utf16 to pass to a windows call).
    //
    // One very good example use case for the temporary allocator: if you are programming a game and you need to calculate
    //   some mesh stuff for a given frame, using this allocator means having the freedom of dynamically allocating
    //   without compromising performance. At the end of the frame when the memory is no longer used you call free_all(Context.TempAlloc)
    //   and start the next frame.
    //
    // This gets initialized the first time it gets used in a thread.
    // Each thread gets a unique temporary allocator to prevent data races and to remain fast.
    // Default size is 8 KiB but you can increase that by adding a pool with allocator_add_pool().
    // When out of memory, it allocates and adds a new bigger pool.
    //
    // We print warnings when allocating new pools. Use that as a guide to see where you need to pay more attention
    // - perhaps increase the pool size or call free_all() more often.
    //
    allocator TempAlloc;

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

Feel free to add more variables to your version of the library.

The context gets initialized when the program runs for the main thread and on `tls_init` (the callback that runs when any new thread is created).
If creating a thread with our API we can know the parent thread, so we copy the options from it (otherwise it's default-initialized).

> ex.1 Allocating memory.
> ```cpp
> void *memory;
> memory = allocate_array<char>(150);                    // Using Context.Alloc (make sure you have set it beforehand)
> 
> memory = allocate_array<char>(150, Context.TempAlloc); // Uses the temporary allocator explicitly
> 
> // You can also implement custom allocators:
> // my_allocator_function is a function which implements all the functionality of the allocator (allocating, resizing, freeing, etc.)
> allocator myAlloctor = { my_allocator_function, null /* This pointer is used for any data (state) the allocator needs. */};
> memory = allocate_array<char>(150, myAlloctor);;
> ```

We don't use new and delete.
1.  The syntax is ugly in my opinion.
2.  You have to be careful not to mix "new" with "delete[]"/"new[]" and "delete".
3.  It's an operator and can be overriden by structs/classes.
4.  Modern C++ people say not to use new/delete as well, so ..

Now seriously, there are two ways to allocate memory in C++: malloc and new.
Both are far from optimal, so let's introduce another way (haha)!
> Note: We override the default operator new/delete to call our version. When we don't link with the CRT, malloc is undefined, we provide a replacement.\
> When we link with the CRT, we do it dynamically, so we can redirect calls to malloc to our replacement. @TODO: We don't do that yet..

The following functions are defined: `allocate`, `allocate_array`, `reallocate_array`, `free`. All work as expected.

> Note: `allocate` and `allocate_array` call constructors on non-scalar values, `free` calls destructors (make sure you pass the right pointer type to free!)

We allocate a bit of space before the block to store a header with information (the size of the allocation, the alignment,
the allocator with which it was allocated, and debugging info if `DEBUG_MEMORY` is defined - see comments in `"allocator.h"`).

There are is big assumption we make: your types are "trivially copyable" which means that they can be copied byte by byte to
another place and still work. We don't call copy/move constructors when reallocating.

We are following a data oriented design with this library.
Our `array` type also expects type to be simple data. If you need extra logic when copying memory, implement that explicitly
(not with a copy constructor). We do that to avoid C++ complicated shit that drives sane people insane.

Here we don't do C++ style exceptions, we don't do copy/move constructors, we don't do destructors, we don't do any of that.

> Note: I said we don't do destructors but we still call them. That's because sometimes they are useful and can really simplify the code.
> However since arrays and even the basic allocation functions copy byte by byte, that means that types that typically own memory (like `string`) must
> be implemented differently - which means that destructors become useless. Take a look after the "manifest" to see how we handle that.
> The recommended way to implement "destructors" is to override the free() function with your type as a reference parameter.

> ```cpp
> void free(string &s) { 
>     free((array<utf8> &) s); // Free as an array
>     s.Length = 0; 
> }
> 
> // ...
> 
> string a;
> free(a);       // Not free(&a)!
> ```

The memory functions also allow certain options to be specified.
Implementations of this is in `"context.h"`.

> ex.2 Using C++20 syntax for allocator options.
> ```cpp
>     void *node;
>     node = allocate<ast_binop>();
>     node = allocate<ast_binop>({.Alloc = AstNodeAllocator});
>     node = allocate<ast_binop>({.Options = LEAK});
> 
>     auto *simdType = allocate<f32v4>({.Alignment = 16});
> 
>     void *memory;
>     memory = allocate_array<byte>(200);
>     memory = allocate_array<byte>(200, {.Alloc = Context.TempAlloc, .Alignment = 64, .Options = LEAK});
> ```

The functions also take `source_location` as a final parameter. This is set to `current()` automatically which means the caller's site.
This info is saved to the header when `DEBUG_MEMORY` is defined.

> ex.3 From the test suite. Demonstates temporary allocator initialization and how to change the allocator which is used in a scope.
> ```cpp
>     time_t start = os_get_time();
> 
>     auto newContext = Context;
>     newContext.AllocAlignment = 16; // For SIMD math types
>     newContext.Alloc = Context.TempAlloc;
> 
>     allocator_add_pool(Context.TempAlloc, os_allocate_block(1_MiB), 1_MiB);
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
>             allocator_pool_add_to_linked_list(&data->Base, pool);
>             if (pool) {
>                 ++data->PoolsCount;
>                 return pool;
>             }
>             return null;
>         }
>         case allocator_mode::REMOVE_POOL: {
>             auto *pool = (allocator_pool *) oldMemory;
> 
>             void *result = allocator_pool_remove_from_linked_list(&data->Base, pool);
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
>             // We simply return null and let the reallocate function allocate a new block and copy the contents.
>             //
>             // If you are dealing with very very large blocks and copying is expensive, you should
>             // implement a specialized allocator. If you are dealing with appending to strings
>             // (which causes string to try to reallocate), we provide a string_builder utility which will help with that.
>             return null;
>         }
>         case allocator_mode::FREE: {
>             // We don't free individual allocations in the arena allocator
> 
>             // null means success FREE
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
> 
>             // null means successful FREE_ALL
>             // (void *) -1 means that the allocator doesn't support FREE_ALL (by design)
>             return null;
>         }
>         default:
>             assert(false);
>     }
>     return null;
> }
> ```


### Available allocators

See `"allocator.h"`.

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

- [Cephes](https://www.netlib.org/cephes/) Stephen L. Moshier, math functions as a replacement to avoid linking with the CRT.
- [tlsf](https://github.com/mattconte/tlsf), Matthew Conte, Two-Level Segregated Fit memory allocator implementation.

### Projects using this library

- [light-std-graphics](https://github.com/Repertoi-e/light-std-graphics) - high-level windowing API (like GLFW) and a high-level graphics API.
- [Physics engine](https://repertoi-e.github.io/Portfolio/Simulating-Rigid-Body-Physics.html) - simulating rigid bodies.
- [Graphing calculator](https://github.com/Repertoi-e/light-std-graphics/tree/main/game/src).

My portfolio: https://repertoi-e.github.io/Portfolio/blog.html

