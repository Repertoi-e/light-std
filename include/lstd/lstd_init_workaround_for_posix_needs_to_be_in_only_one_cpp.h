#pragma once 

// @TODO @Hack

#if OS == MACOS || OS == LINUX
__attribute__((constructor)) inline void gcc_clang_initialize_state() { 
    void platform_state_init();
    platform_state_init(); 
}
#endif
