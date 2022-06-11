module;

#include "../common.h"

export module lstd.thread;

#if OS == WINDOWS
export import lstd.thread.win32;
#else
#endif
