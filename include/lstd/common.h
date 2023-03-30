#pragma once

//
// A header which imports common types, numeric info,
//     common math functions, definitions for the macros:
//	assert, defer, For, For_enumerate ...
//		static_for, range
// ... and others.
//
// And also memory stuff:
//     memcpy, memset, memset0, memcmp
//
// Really very common lightweight stuff that's used all the time.
//

#include "common/cpp/arg.h"
#include "common/cpp/compare.h"
#include "common/cpp/initializer_list.h"
#include "common/cpp/source_location.h"

#include "common/assert.h"
#include "common/debug_break.h"
#include "common/defer.h"
#include "common/enumerate.h"
#include "common/for.h"
#include "common/namespace.h"
#include "common/platform.h"
#include "common/semantic.h"
