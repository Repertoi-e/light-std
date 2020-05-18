#include "common.h"

#include <math.h>  // @DependencyCleanup

LSTD_BEGIN_NAMESPACE

f32 min(f32 x, f32 y) { return fminf(x, y); }
f32 max(f32 x, f32 y) { return fmaxf(x, y); }
f64 min(f64 x, f64 y) { return fmin(x, y); }
f64 max(f64 x, f64 y) { return fmax(x, y); }

LSTD_END_NAMESPACE
