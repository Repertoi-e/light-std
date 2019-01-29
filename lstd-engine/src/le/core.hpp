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

// Used for keyboard/mouse input
enum : u32 {
    Modifier_Shift = BIT(0),
    Modifier_Control = BIT(1),
    Modifier_Alt = BIT(2),
    Modifier_Super = BIT(3),
};

// 'x' needs to have dll-interface to be used by clients of struct 'y'
// This will never be a problem since nowhere do we change struct sizes based on debug/release/whatever conditions
#if COMPILER == MSVC
#pragma warning(disable : 4251)
#endif