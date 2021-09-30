#include "lstd/common.h"

#if OS == WINDOWS

//
// This is here to assist cases where you want to share the memory between two modules (e.g. an exe and a dll or multiple dlls, etc.)
// By default, when you link lstd with the dll, each dll gets its own global state (global allocator, debug memory info, etc.),
// which means that allocations done in different modules are incompatible. If you provide a function lstd_init_global
// which returns "false" we don't initialize that global state (instead we leave it as null). That means that YOU MUST initialize it yourself!
// You must initialize the following global variables by passing the values from the "host" to the "guest" module:
//  - DEBUG_memory     (a global pointer, by default we allocate it)
//
// @Volatile: As we add more global state.
//
// Why do we do this?
// In another project (light-std-graphics) I have an exe which serves as the driver, and loads dlls (the engine). We do this to support hot-loading
// so we can change the game code without closing the window. The game (dll) allocates memory and needs to do that from the engine's allocator 
// and debug memory, otherwise problems occur when hot-loading a new dll.
//
// @Cleanup @Cleanup @Cleanup @Cleanup @Cleanup @Cleanup @Cleanup @Cleanup  There should be a better way and we should get rid of this.
//                                                                          I haven't thought much about it yet.
extern "C" bool lstd_init_global() { return true; }

// If the user didn't provide a definition for lstd_init_global, the linker shouldn't complain,
// but instead provide a stub function which returns true.
extern "C" bool lstd_init_global_stub() { return true; }
#pragma comment(linker, "/ALTERNATENAME:lstd_init_global=lstd_init_global_stub")

#endif
