#pragma once

#include "memory/allocator.h"
#include "storage/string.h"
#include "thread.h"

LSTD_BEGIN_NAMESPACE

namespace io {
struct writer;
}

namespace internal {
extern io::writer *g_ConsoleLog;
}  // namespace internal

// @Cleanup: Move these somewhere else?
struct os_function_call {
    string Name;
    string File;
    u32 LineNumber;
};

inline os_function_call *clone(os_function_call *dest, os_function_call src) {
    clone(&dest->Name, src.Name);
    clone(&dest->File, src.File);
    dest->LineNumber = src.LineNumber;
    return dest;
}

inline os_function_call *move(os_function_call *dest, os_function_call *src) {
    move(&dest->Name, &src->Name);
    move(&dest->File, &src->File);
    dest->LineNumber = src->LineNumber;
    return dest;
}
//

template <typename T>
struct array;

typedef void os_unexpected_exception_handler_t(string message, array<os_function_call> callStack);

void default_unexpected_exception_handler(string message, array<os_function_call> callStack);

// @TODO: By default our alloc alignment is 16 (simd friendly).
// Maybe that's too big and we should have a context variable to control it.
struct implicit_context {
    ~implicit_context() { release_temporary_allocator(); }

    // When allocating you should use the context's allocator
    // This makes it so when users call your functions they
    // can specify an allocator beforehand by pushing a new context variable,
    // without having to pass you anything as a parameter for example.
    //
    // The idea for this comes from the implicit context in Jai.
    allocator Alloc = Malloc;

    // This allocator gets initialized the first time it gets used in a thread.
    // Each thread gets a unique temporary allocator to prevent data races and to remain fast.
    temporary_allocator_data TemporaryAllocData;
    allocator TemporaryAlloc = {temporary_allocator, &TemporaryAllocData};

    // Frees the memory held by the temporary allocator (if any).
    // Gets called by the context's destructor.
    void release_temporary_allocator() const;

    // When printing you should use this variable.
    // This makes it so users can redirect logging output.
    // By default it points to io::cout (the console).
    io::writer *Log = internal::g_ConsoleLog;

    // Gets called when the program encounters an unhandled expection.
    // This can be used to view the stack trace before the program terminates.
    // The default handler prints the crash message and stack trace to _Log_.
    os_unexpected_exception_handler_t *UnexpectedExceptionHandler = default_unexpected_exception_handler;

    // @TODO Posix implementation
    // _pthread_t_to_ID(pthread_self());
    /*
    static thread::id _pthread_t_to_ID(const pthread_t &aHandle)
    {
      static mutex idMapLock;
      static std::map<pthread_t, unsigned long int> idMap;
      static unsigned long int idCount(1);

      lock_guard<mutex> guard(idMapLock);
      if(idMap.find(aHandle) == idMap.end())
        idMap[aHandle] = idCount ++;
      return thread::id(idMap[aHandle]);
    }
    */

    // The current thread's ID
    thread::id ThreadID;

    // Yield execution to another thread.
    // Offers the operating system the opportunity to schedule another thread
    // that is ready to run on the current processor.

    // sched_yield();
    void thread_yield() const;

    // Blocks the calling thread for at least a given period of time in ms.

    // usleep()
    void thread_sleep_for(u32 ms) const;
};

// Immutable context available everywhere
// The current state gets copied from parent thread to the new thread when creating a thread
inline thread_local const implicit_context Context;

LSTD_END_NAMESPACE

// This is a helper macro to safely modify a variable in the implicit context in a block of code.
// Usage:
//    PUSH_CONTEXT(variable, newVariableValue) {
//        ... code with new context variable ...
//    }
//    ... old context variable value is restored ...
//
#define PUSH_CONTEXT(var, newValue)                                     \
    auto LINE_NAME(oldVar) = Context.var;                               \
    auto LINE_NAME(restored) = false;                                   \
    auto LINE_NAME(context) = const_cast<implicit_context *>(&Context); \
    defer({                                                             \
        if (!LINE_NAME(restored)) {                                     \
            LINE_NAME(context)->##var = LINE_NAME(oldVar);              \
        }                                                               \
    });                                                                 \
    if (true) {                                                         \
        LINE_NAME(context)->##var = newValue;                           \
        goto LINE_NAME(body);                                           \
    } else                                                              \
        while (true)                                                    \
            if (true) {                                                 \
                LINE_NAME(context)->##var = LINE_NAME(oldVar);          \
                LINE_NAME(restored) = true;                             \
                break;                                                  \
            } else                                                      \
                LINE_NAME(body) :

#define ALLOC(newAlloc) PUSH_CONTEXT(Alloc, newAlloc)
