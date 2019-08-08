#pragma once

#include "storage/string_utils.h"

/// Platform specific general functions

// Allocates memory by calling OS functions
void *os_alloc(size_t size);

// Frees a memory block allocated by _os_alloc()_
void os_free(void *ptr);

// Exits the application with the given exit code
void os_exit(s32 exitCode = 0);

// Returns a time stamp that can be used for time-interval measurements
time_t os_get_time();

// Converts a time stamp acquired by _os_get_time()_ to seconds
f64 os_time_to_seconds(time_t time);

// Sleep for _ms_ milliseconds
// void os_sleep(f64 ms);

// Returns the path of the current exe (full dir + name)
string_view os_get_exe_name();
