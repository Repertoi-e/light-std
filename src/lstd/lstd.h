#pragma once

//
// We can't get away with just modules because we define certain 
// macros (e.g. defer, For ..) which we can't export. Use the header like this:
//
// import "lstd.h";
// 
// This imports lstd.basic (usual memory stuff, arrays, strings, etc.)
//
// for other modules in the library do the usual import:
//
// import lstd.fmt;
// import lstd.os;
//

#include "lstd/common.h"

import lstd.basic;
