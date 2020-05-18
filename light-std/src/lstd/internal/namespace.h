#pragma once

/// A header which defines macros for wrapping this library in a namespace
/// Define LSTD_NAMESPACE as a preprocessor definition to the value you want the namespace to be called.
/// (By default the libary doesn't have a namespace)

#if defined LSTD_NAMESPACE
#define LSTD_BEGIN_NAMESPACE namespace LSTD_NAMESPACE {
#define LSTD_END_NAMESPACE }
#else
#define LSTD_NAMESPACE
#define LSTD_BEGIN_NAMESPACE
#define LSTD_END_NAMESPACE
#endif