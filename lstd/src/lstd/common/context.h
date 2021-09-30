#pragma once

//
// This is a helper macro to safely modify a variable in the implicit context in a block of code.
// Usage:
//    auto newContext = Context;
//    newContext.var1 = newValue1;
//    newContext.var2 = newValue2;
//
//    PUSH_CONTEXT(newContext) {
//        ... code with new context variables ...
//    }
//    ... old context variables are restored ...
//
//    // This changes the context variables globally.
//    // Useful at program startup e.g to set an allocator or a logging output.
//    OVERRIDE_CONTEXT(newContext);
//

#define OVERRIDE_CONTEXT(newContext) *((LSTD_NAMESPACE::context *) &LSTD_NAMESPACE::Context) = (newContext)

#define PUSH_CONTEXT(newContext)                          \
    auto LINE_NAME(oldContext) = LSTD_NAMESPACE::Context; \
    auto LINE_NAME(restored)   = false;                   \
    defer({                                               \
        if (!LINE_NAME(restored)) {                       \
            OVERRIDE_CONTEXT(LINE_NAME(oldContext));      \
        }                                                 \
    });                                                   \
    if (true) {                                           \
        OVERRIDE_CONTEXT(newContext);                     \
        goto LINE_NAME(body);                             \
    } else                                                \
        while (true)                                      \
            if (true) {                                   \
                OVERRIDE_CONTEXT(LINE_NAME(oldContext));  \
                LINE_NAME(restored) = true;               \
                break;                                    \
            } else                                        \
                LINE_NAME(body) :

// Shortcut for just modifying the allocator
#define PUSH_ALLOC(newAlloc)                               \
    auto LINE_NAME(newContext)  = LSTD_NAMESPACE::Context; \
    LINE_NAME(newContext).Alloc = newAlloc;                \
    PUSH_CONTEXT(LINE_NAME(newContext))

