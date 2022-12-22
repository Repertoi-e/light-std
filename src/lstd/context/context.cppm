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

    void free(os_function_call ref src) {
        free(src.Name);
        free(src.File);
    }

    os_function_call clone(os_function_call no_copy src) {
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

    // Hack, the default constructor would otherwise zero init the context's members, which might have been set by other global constructors.
    struct context_dont_init_t {};

    // See note below at the variable declaration... :Context:
    struct context {
        context(context_dont_init_t) {}

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
        // the context is a better way to handle this. Otherwise we aren't following our 
        // own promise that you should be able to robustly control the Context in a given scope.
        // Being explicit when we don't know what to do is better, in my opinion.
        //
        ///////////////////////////////////////////////////////////////////////////////////////

        //
        // Each allocator holds a function pointer and an optional data pointer.
        // The function contains all the functionality (allocate, reallocate, free).
        //
        // The variable here controls all allocations done in a given piece of code.
        //
        // Change this (recommended way is to use the PUSH_ALLOC macro) in order to
        // set the allocator which a piece of code uses without that piece of
        // code having to ever pay attention to it.
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
        // We encourage using a specialized allocator 
        // depending on the memory requirements and 
        // the specific use case.
        // See :BigPhilosophyTime: in allocator.h for 
        // the reasoning behind this.
        //
        // = null by default. The user should manually provide 
        // an allocator at the start of the program.
        //
        // @Hack See the note above:    struct allocator_dont_init_t;...
        allocator Alloc = allocator(allocator_dont_init_t{});

        //
        // Controls how newly allocated memory gets aligned.
        // You can control individual allocations with the .Alignment option in malloc<>().
        // This is here so you can change alignment for every allocation
        // in an entire scope (or an entire run of a program).
        //
        u16 AllocAlignment;  // = POINTER_SIZE (8);     by default

        //
        // When doing allocations we provide an optional parameter that is meant 
        // to be used as flags. What each bit means is specific to the allocator 
        // function that is being used. However some bits are reserved and we handle 
        // them internally.
        //
        // One such bit is the LEAK flag (the 64th bit). Any allocation marked
        // with LEAK doesn't get reported when calling debug_memory_report_leaks().
        //
        // This variable here gets OR'd when doing new allocations,
        // so you can e.g. mark an entire scope of allocations with LEAK
        // (or your own specific use case with custom allocator).
        //
        u64 AllocOptions;  // = 0;     by default

        // Used for debugging. Every time an allocation/reallocation
        // is made, logs info about it.
        bool LogAllAllocations;  // = false;     by default

        //
        // Gets called when the program encounters an unhandled exception.
        // This can be used to view the stack trace before the program terminates.
        // The default handler prints the crash message and stack trace to _Log_.
        //
        panic_handler_t PanicHandler;  // = default_panic_handler;     by default (see context.cpp for source) 

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
        writer *Log;  // = &cout;     by default

        //
        // fmt module:
        // Disable stylized text output (colors, background colors, and 
        // bold/italic/strike-through/underline text). This is useful 
        // if logging has been redicted to files/strings and not the console. 
        // The ansi escape codes look like garbage in files/strings.
        //
        bool FmtDisableAnsiCodes;  // = false;     by default

#if defined DEBUG_MEMORY
        // After every allocation we check the heap for corruption.
        // The problem is that this involves iterating over a possibly 
        // large linked list of every allocation made. We use the frequency 
        // variable below to specify how often we perform that expensive 
        // operation. By default we check the heap every 255 allocations, 
        // but if a problem is found you may want to decrease
        // this to 1 so you catch the corruption at just the right time.
        u8 DebugMemoryHeapVerifyFrequency; // = 255;     by default

        // Self-explanatory 
        bool DebugMemoryPrintListOfUnfreedAllocationsAtThreadExitOrProgramTermination; // = false;     by default
#endif

        //
        // fmt module:
        // By default when we encounter an invalid format string we panic the program.
        // One might want to silence such errors and just continue executing, or 
        // redirect the error - like we do in the tests.
        //
        fmt_parse_error_handler_t FmtParseErrorHandler;  // = fmt_default_parse_error_handler;     by default

        // Internal.
        bool _HandlingPanic;        // = false;   // Don't set. Used to avoid infinite looping when handling panics. Don't touch!
        bool _LoggingAnAllocation;  // = false;   // Don't set. Used to avoid infinite looping when logging allocations. Don't touch!
    };

    // :Context:
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
	// Gets initialized in _platform_init_context()_ (the first thing that runs 
	// in the program) because otherwise the default constructor would override 
	// the values (which may have changed from other global constructors).
    // 
    // Modify this variable with the macros PUSH_CONTEXT or OVERRIDE_CONTEXT, 
    // the first one restores the old value at end of the following scope 
    // (or when breaking out the scope, e.g. returning from a function), while 
    // the latter changes the context globally for the entire run of the program.
    // These are defined in common/context.h
    //
    // The reason this is a const variable is that it may prevent unintended bugs.
    // A malicious author of a library can use a const_cast to change a variable 
    // and not restore it in the end, but he can also do 1000 other things that 
    // completely break your program, so...
    inline const thread_local context Context = context(context_dont_init_t{});

    void panic(string message) {
        // @TODO Get callstack!
        Context.PanicHandler(message, {});
    }

    //
    // :TemporaryAllocator:
    //
    // We optionally store a thread local global arena allocator that is meant to 
    // be used as temporary storage.
    // It can be used to allocate memory that is not meant to last long
    // (e.g. converting utf-8 to utf-16 to pass to a windows call).
    //
    // If you are programming a game and you need to do some calculations each frame,
    // using this allocator means having the freedom of dynamically allocating without
    // compromising performance. At the end of the frame when the memory is no longer
    // used you call free_all(TemporaryAllocator) (both allocate and free all are
    // extremely cheap - they bump a single pointer).
    //
    // You must initialize this by saying what block of memory to use as an arena 
    // before using it in a thread
    // e.g.:       TemporaryAllocator.Block = os_allocate_block(poolSize);
    //             TemporaryAllocator.Size = poolSize;
    //
    inline thread_local arena_allocator_data TemporaryAllocatorData;
    inline const thread_local allocator TemporaryAllocator = allocator(allocator_dont_init_t{});  // Disable the default constructor, this gets initialized with the Context. See hack note above.

    // Allocates a buffer, copies the string's contents and also appends a zero terminator.
    // Uses the temporary allocator.
    char *string_to_c_string_temp(string s) { return to_c_string(s, TemporaryAllocator); }
}

LSTD_END_NAMESPACE
