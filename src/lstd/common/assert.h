#pragma once

LSTD_BEGIN_NAMESPACE

//
// Defines the normal debug assert you'd expect to see.
//
#undef assert

#if !defined NDEBUG
#define assert(condition) (!!(condition)) ? (void) 0 : debug_break()
#else
#define assert(condition) ((void) 0)
#endif

LSTD_END_NAMESPACE
