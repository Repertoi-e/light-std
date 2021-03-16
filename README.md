
# light-std
A C++20 library created for personal use that aims to replace the standard C/C++ library. It's performance-oriented and designed for general programming.
It attempts to mimic some of Jai's (Jonathan Blow's new language) features (implicit context and allocator system).

This library is supposed to be a replacement of C/C++'s standard library in but designed entirely differently. 

It is completely stand-alone - it doesn't include ANY headers from the default standard library (some C++ language features require certain definitions but
we provide our own). 

Anybody who thinks modern C++ is stupid, you've come to the right place! std:: is way too complicated and messy and a lot of C++'s features add unnecessary complications to the library code.
My view on some of those features are visible in the type policy.

## Type policy

### Consequence of this policy
- Dramatically reduce complexity and code size (both library AND user side!) UNLESS that comes at a run-time cost

### Rules
- Always provide a default constructor (implicit or by `T() = default`)
- Every data member (which makes sense) should be public. Do not write useless getters/setters.
- Strive to make classes/structures/objects (whatever you wanna call them) data oriented.
  Programs work with data. Design your data so it makes the solution straightforward and minimize abstraction layers.
- No user defined copy/move constructors.
- No throwing of exceptions, .. ever, .. anywhere. No excuse.

### "No user defined copy/move constructors":
_This may sound crazy if you have a type that owns memory. How would you deep copy the contents and not just the pointer when you copy the object? 
How do you handle dangling pointers to unfreed memory of shallow copies when the original destructor fires? There is a solution which we implement in objects
that deal with dynamically allocated buffers (like string, array, etc.)_

This library implements `string` the following way:
- The object is designed with no ownership in mind. That is determined explictly in code and by the programmer. `string`'s destructor is EMPTY. It doesn't free any buffers. The user is responsible for freeing the string when that is required. We make this easier by providing a defer macro which runs at the end of the scope (like a destructor). An example is provided below.

- A string has either allocated memory or it has not. We make this very clear when returning strings from functions in the library. If the procedure is marked as `[[nodiscard]]` that means that the returned string should be freed.
- All of this allows us to skip writing copy/move constructors and assignment operators, we avoid unnecessary copies by making the programmer think harder about managing the memory, while being explicit and concise, so you know what your program is doing.
```cpp
        // Constructed from a zero-terminated string buffer (stored in read only memory when the program launches). Doesn't allocate memory.
        string path = "./data/"; 

        // _string_ includes both constexpr methods and runtime-only methods which modify the contents (and might allocate memory).
        // It's like a mixed type between std::string_view and std::string from the STL, but with way better design and API.
        // When a string needs to allocate memory it copies the old contents of the string.
        path.append("output.txt");

        // Doesn't release the string here, but instead runs at scope exit.
        // "defer" runs exactly like a destructor, but it's explicit and not hidden.
        // This style of programming makes you write code which doesn't allocate strings superfluously.
        defer(path.release());   

        // This doesn't allocate memory but it points to the buffer in _path_.
        // The substring is valid as long as the original string is valid.
        // Also all operations with indices on strings support negative indexing (python-style). 
        // So `-1` is translated to `str.Length - 1`.
        string pathWithoutDot = path.substring(1, -1);    
```

This example demonstrates the interactions with this fluid idea of ownership:
```cpp
        string get_data_root_dir() {
            string path = os_get_working_dir();

            // You don't want to use strings here directly (instead use the path module
            // which is more robust with handling slashes) but for the sake of example..
            //
            // The line below allocates memory!
            path.append("/data");
            return path; // Doesn't call copy constructor, _path_ doesn't call it's destructor

            // The fact that you can do this means that you don't rely on C++ compiler optimization 
            // like "copy elision" when returning strings from functions. This means that your debug 
            // builds run way way way faster (anybody who has tried launching a game in debug build knows..
            // Hopefully not many people - if you are making a serious game with STL you are a crazy person).
        }
        
        // ...
        string levelData = get_data_root_dir(); 

        // We want to free the memory when this scope exists and we don't need it anymore.
        defer(levelData.release());          

        // The buffer allocated in get_data_root_dir() has "leaked" here and is still usable!
        // Although this string (_levelData_) is a different object, it still just holds a buffer
        // which doesn't have a specific owner.
        levelData.append("/levels/level1.dat"); 
        
        // ...
        
        // When the scope exits, _levelData_ is freed with the defer statement.
```

-  `clone(T *dest, T src)` is a global function that ensures a deep copy of the argument passed. Objects that own memory (like `string`) overload `clone()` and make sure a deep clone is done. This is like a classic copy constructor but much cleaner. Note: `clone` works on all types (unless overloaded the default implementation does a shallow copy). It is this library's recommended way to implement functionality normally written in copy c-tors.

### "No throwing of exceptions, anywhere"
Exceptions make your code complicated. 
You should design your code in such a way that errors can't occur (or if they do - handle them, not just bail, and when even that is not possible - stop execution).
I can't be bothered to write about exceptions. Really, they shouldn't exist in C++, forget about exception safety, `noexcept` and all that bullshit.

## Examples

Container API is inspired in parts by Rust and Python. 

### Example usage of data structures:
```cpp
// string uses utf8 encoding out of the box
string a = "ЗДРАСТИ";
for (auto ch : a) {
    // Here _a_ is non-const so _ch_ is actually a reference to the code point in the string and you can modify it directly
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
// _to_array_ here just makes a stack array from the listed integers so they can be compared to _array_
assert(integers == to_array(0, 1, 2, 3, 4));
```

### Example of custom allocators and implicit context:

The thread-local global context variable includes these variables related to allocating:
```cpp
    
    allocator Alloc = DefaultAlloc; // The allocator used when allocations without an explicit allocator are requested.
    u16 AllocAlignment = POINTER_SIZE;  // The default alignment

    // Any options that get OR'd with the options in any allocation (options are implemented as flags).
    // e.g. using the LEAK flag, you can mark the allocation as a leak (doesn't get reported when calling DEBUG_memory_info::report_leaks()).
    u64 AllocOptions = 0;
```

```cpp
memory = allocate_array<char>(150);                // Using Context.Alloc

memory = allocate_array<char>(150, DefaultAlloc);           // Using the "malloc" allocator explictly (this overrides Context.Alloc which may be totally different)
memory = allocate_array<char>(150, Context.TemporaryAlloc); // Uses the temporary allocator (this, again, overrides Context.Alloc)

// You can also implement custom allocators:
// my_allocator_function is a function which implements all the functionality of the allocator (allocating, resizing, freeing, etc.)
// Take a look at _default_allocator_, _temporary_allocator_, to see examples on how to implement one.
allocator myAlloctor = { my_allocator_function, null /* This pointer is used for any data (state) the allocator needs. */};
memory = allocate_array<char>(150, myAlloctor);;
```

```cpp
char *memory1 = allocate_array<char>(150); 

// The temporary allocator is provided by the library, it's essentially a fast arena allocator that's available globally and supports only "free all".
// WITH_ALLOC is a scoped way to change the value of a variable. The old value of the context variable is restored when the scope defined after the macro exits.
WITH_ALLOC(Alloc, Context.TemporaryAlloc) {
    char *memory2 = allocate_array<char>(150);  // Any code in this scope uses the temporary allocator by default
    // ...
}

// Any allocations done afterwards use the allocator which was in the Context before the block.

// Free always calls the allocator the memory was allocated with (we save that information in a header before the block)
free(memory1);
```

More examples will be provided when the library API is in a more stable state. Right now I've done one school project with it: a little game engine which runs a physics simulation with a python interpreter and a hot-loaded dll which contains the game code. 

I plan to write some serious using it and if I run into some friction with the way I've implemented something I might change it and break literally everything. Also this README might go stale and outdated from time to time but I try to update it often.
