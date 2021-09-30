module;

#include "../common.h"

export module lstd.guid;

#if OS == WINDOWS
export import lstd.guid.win32;
#endif
