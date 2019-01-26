#pragma once

#include <lstd/common.hpp>

#if OS == WINDOWS
#if defined LE_BUILD_DLL
#define LE_API __declspec(dllexport)
#else
#define LE_API __declspec(dllimport)
#endif
#else
#if COMPILER == GCC || COMPILER == CLANG
#if defined LE_BUILD_DLL
#define LE_API __attribute__((visibility("default")))
#else
#define LE_API
#endif
#else
#define LE_API
#pragma warning Unknown dynamic link import/export semantics.
#endif
#endif
