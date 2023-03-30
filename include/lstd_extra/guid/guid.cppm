export module lstd.guid;

export import lstd.guid.general;

#if OS == WINDOWS
export import lstd.guid.win32;
#else
#error Implement.
#endif
