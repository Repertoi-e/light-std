module;

#include "../common.h"

export module lstd.context;

export import lstd.string;
export import lstd.writer;

LSTD_BEGIN_NAMESPACE

export {
    struct os_function_call {
        string Name;
        string File;
        u32 LineNumber = -1;
    };

    os_function_call clone(os_function_call src) {
        os_function_call result;
        result.Name       = clone(src.Name);
        result.File       = clone(src.File);
        result.LineNumber = src.LineNumber;
        return result;
    }

    using panic_handler_t = void (*)(string message, array<os_function_call> callStack);
    void default_panic_handler(string message, array<os_function_call> callStack);

    using fmt_parse_error_handler_t = void (*)(string message, string formatString, s64 position);
    void fmt_default_parse_error_handler(string message, string formatString, s64 position);

    //
    // Thread local global variable to control the behavior of a piece of code from outside.
    // A way to store options without passing parameters to routines.
    //
    // The idea for this comes from the implicit context in Jai.
    //
    // Gets initialized when the program runs for the main thread and for each new
    // thread created (the new thread copies the context from the parent thread at the time of creation).
    //
    // Probably the most useful thing about this is the allocator.
    //
    //
    struct context {
        u32 ThreadID;  // The current thread's ID

        ///////////////////////////////////////////////////////////////////////////////////////
        //
        // :ThreadsContext:
        //
        // Threads created with lstd API get proper treatment. Their context is copied from
        // the thread that spawned them. However if the programmer (or an external library)
        // creates a thread with the OS api, then we can't (as far as I know) know the parent
        // thread. In that case we don't initialize the Context at all (to make it unusable).
        // See note in tlsdyn.cpp.
        //
        // In the past we used to provide a valid context with a valid temporary allocator
        // and default values, however I think that letting the programmer manually initialize/copy
        // the context is a better way to handle this. Otherwise we aren't following our own promise
        // that you should be able to robustly control the Context in a given scope.
        // Being explicit when we don't know what to do is better I think.
        //
        ///////////////////////////////////////////////////////////////////////////////////////

        //
        // Each allocator holds a function pointer and an optional data pointer.
        // The function contains all the functionality (allocate, reallocate, free).
        //
        // The variable here controls all allocations done in a given piece of code.
        //
        // Change this (recommended way is to use the PUSH_ALLOC macro) in order to
        // change the allocator which a piece of code uses without that piece of
        // code having to ever pay attention.
        //
        // To it as if nothing changed.
        //
        // The context is thread local and each new thread gets the parent's context.
        // This means that you can control all allocations in a given scope.
        //
        // We override malloc, calloc, realloc and free to use the context's allocator,
        // any external static library also uses those. DLLs may have already been linked
        // with the C runtime library so we can't do much about that.
        // @TODO Maybe on load we can overwrite the symbols??
        //
        //
        // We don't provide a default allocator.
        // We encourage using a specialized allocator depending on the memory requirements and the specific use case.
        // See :BigPhilosophyTime: in allocator.h for the reasoning behind this.
        allocator Alloc;  // = null by default. The user should provide an allocator at the start of the program.

        //
        // Controls how newly allocated memory gets aligned.
        // You can control individual allocations with the .Alignment option in malloc<>().
        // This is here so you can change alignment for every allocation
        // in an entire scope (or an entire run of a program).
        //
        u16 AllocAlignment = POINTER_SIZE;

        //
        // When doing allocations we provide an optional parameter that is meant to be used as flags.
        // What each bit means is specific to the allocator function that is being used.
        // However some bits are reserved and we handle them internally.
        //
        // One such bit is the LEAK flag (the 64th bit). Any allocation marked
        // with LEAK doesn't get reported when calling DEBUG_memory->report_leaks().
        //
        // This variable here gets OR'd when doing new allocations,
        // so you can e.g. mark an entire scope of allocations with LEAK
        // (or your own specific use case with custom allocator).
        //
        u64 AllocOptions = 0;

        // Used for debugging. Every time an allocation is made, logs info about it.
        bool LogAllAllocations = false;

        //
        // Gets called when the program encounters an unhandled expection.
        // This can be used to view the stack trace before the program terminates.
        // The default handler prints the crash message and stack trace to _Log_.
        //
        panic_handler_t PanicHandler = default_panic_handler;

        //
        // Similar to _Alloc_, you can transparently redirect output
        // with this variable. By default we print to the console (the global variable cout).
        // You can change this to a file or your custom game engine console.
        //
        // The print function in the fmt module uses this.
        // Currently that is the only logging facility we provide.
        // However you should use this variable
        // if you have your own logging functions
        //
        writer *Log = null;

        //
        // fmt module:
        // Disable stylized text output (colors, background colors, and bold/italic/strike-through/underline text).
        // This is useful when logging to files/strings and not the console. The ansi escape codes look like garbage
        // in files/strings.
        //
        bool FmtDisableAnsiCodes = false;

        //
        // fmt module:
        // By default when we encounter an invalid format string we panic the program.
        // One might want to silence such errors and just continue executing, or redirect the error - like we do in the tests.
        //
        fmt_parse_error_handler_t FmtParseErrorHandler = fmt_default_parse_error_handler;

        // Internal.
        bool _HandlingPanic       = false;  // Don't set. Used to avoid infinite looping when handling panics. Don't touch!
        bool _LoggingAnAllocation = false;  // Don't set. Used to avoid infinite looping when logging allocations. Don't touch!
    };

    //
    // Thread local global variable to control the behavior of a piece of code from outside.
    // A way to store options without passing parameters to routines.
    //
    // The idea for this comes from the implicit context in Jai.
    //
    // Gets initialized when the program runs for the main thread and for each new
    // thread created (the new thread copies the context from the parent thread at the time of creation).
    //
    // Probably the most useful thing about this is the allocator.
    //
    // Modify this variable with the macros PUSH_CONTEXT or OVERRIDE_CONTEXT, the first one restores
    // the old value at end of the following scope (or when breaking out the scope, e.g. returning from
    // a function), while the latter changes the context globally for the entire run of the program.
    // These are defined in common/context.h
    //
    // The reason this is a const variable is that it may prevent unintended bugs.
    // A malicious author of a library can use a const_cast to change a variable and not restore
    // it in the end, but he can also do 1000 other things that completely break your program, so...
    inline const thread_local context Context = {};

    void panic(string message) {
        // @TODO Get callstack!
        Context.PanicHandler(message, {});
    }

    //
    // :TemporaryAllocator: Take a look at the docs of this allocator in "allocator.h"
    // (or the allocator module if you are living in the future).
    //
    // We store an arena allocator in the Context that is meant to be used as temporary storage.
    // It can be used to allocate memory that is not meant to last long (e.g. converting utf-8 to utf-16
    // to pass to a windows call).
    //
    // If you are programming a game and you need to do some calculations each frame,
    // using this allocator means having the freedom of dynamically allocating without
    // compromising performance. At the end of the frame when the memory is no longer
    // used you call free_all(TemporaryAllocator) (which is extremely cheap - bumps a single pointer).
    //
    // This gets initialized the first time it gets used in a thread.
    // Each thread gets a unique temporary allocator to prevent data races and to remain fast.
    // Default size is 8 KiB but you can increase that by adding a pool with allocator_add_pool().
    //
    // When out of memory, it automatically allocates a new bigger pool.
    // We print warnings when allocating new pools. Use that as a guide to see when you need
    // to pay more attention: perhaps increase the starting pool size or call free_all() more often.
    //
    // Note: This is the only variable that doesn't get copied from the parent thread.
    // Instead each thread gets its own temporary allocator data.
    // _TempAllocData_ is stored outside of the context, because having a member
    // variable pointing to another member variable is dangerous.
    //
    inline const thread_local arena_allocator_data TemporaryAllocatorData;
    inline const thread_local allocator TemporaryAllocator;

    // Allocates a buffer, copies the string's contents and also appends a zero terminator.
    // Uses the temporary allocator.
    char *string_to_c_string_temp(string s) { return string_to_c_string(s, TemporaryAllocator); }
}

LSTD_END_NAMESPACE
