#pragma once

#include <game.h>

#if COMPILER == MSVC || COMPILER == GCC
#pragma push_macro("allocate")
#pragma push_macro("free")
#undef allocate
#undef free

// pybind defines a macro called COMPILER...............
#define POP_ALLOCATE
#endif

#include "vendor/pybind11/eval.h"
#include "vendor/pybind11/numpy.h"
#include "vendor/pybind11/pybind11.h"

#if defined POP_ALLOCATE
#undef COMPILER
#include <lstd/platform.h>
#pragma pop_macro("allocate")
#pragma pop_macro("free")
#undef POP_ALLOCATE
#endif
