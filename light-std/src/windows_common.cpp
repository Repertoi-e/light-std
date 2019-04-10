#include "lstd/common.h"

#if OS == WINDOWS

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <crtdbg.h>
#include <stdio.h>

void *os_alloc(size_t size) { return HeapAlloc(GetProcessHeap(), 0, size); }

void os_free(void *ptr) { HeapFree(GetProcessHeap(), 0, ptr); }

void os_assert_failed(const byte *file, s32 line, const byte *message) {
    // @Temp
	byte buffer[500];
    sprintf_s(buffer, ">>> %s:%d, Assert failed: %s\n", file, line, message);
    if (buffer[0] != 0) {
        OutputDebugStringA(buffer);
	} else {
        OutputDebugStringA("Assert failed, but buffer was too small...");
	} 

	// @TODO
    // print("{}>>> {}:{}, Assert failed: {}{}\n", fmt::fg::Red, file, line, condition, fmt::fg::Reset);

	(void) message;
    __debugbreak();
}

#endif