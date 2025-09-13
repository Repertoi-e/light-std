# Copilot Instructions for light-std

This is a data-oriented C++20 library designed to replace the standard C/C++ library. Understanding the core philosophy and unique patterns is essential for effective development.

## Core Philosophy

### Data-Oriented Design
- Prioritize cache efficiency and memory layout over object-oriented abstractions
- Use `struct` instead of `class`, keep everything public
- Avoid copy/move constructors, destructors, and exceptions
- Return multiple values via structured bindings: `auto [content, success] = function()`

### Memory Management Pattern
- **No malloc/new by default** - all allocations require explicit allocators
- **Context system**: `Context.Alloc` provides the current allocator
- **Arena allocators**: Fast bump-pointer allocation with `free_all()` to reset
- **RAII via defer**: Use `defer(free(arr))` instead of destructors

```cpp
// Typical allocation pattern
array<string> arr;
reserve(arr);                  // Uses Context.Alloc
defer(free(arr));             // Cleanup at scope end

// Explicit allocator context
PUSH_ALLOC(PERSISTENT) {
    auto data = make_array<int>(10);  // Uses PERSISTENT allocator
}
```

## Build System

### NOB Build Tool
- Use `./nob [config]` to build (self-rebuilding C build script)
- Configurations: `debug`, `optimized`, `release`
- Builds: lstd library → Rust FFI → lang tool → examples
- Generated files: Unicode tables (`unicode_tables.inc`) via Python scripts

### Key Components
- **lstd library** (`src/lstd/lib.cpp`): Core C++20 library
- **lang tool** (`lang/`): Language processor with tokenizer
- **Rust FFI** (`rust/src/lib.rs`): Error reporting via annotate-snippets
- **test-suite** (`test-suite/`): Comprehensive tests

## Essential Patterns

### Array Philosophy
```cpp
// Arrays are views by default (Data, Count, Allocated)
array<int> view(data, size);          // Non-owning view
array<int> dynamic = make_array(...); // Allocates memory
auto copy = clone(original);          // Deep copy
```

### String Handling
- UTF-8 by default, not null-terminated
- Use `string_builder` for concatenation
- Substrings are views (no allocation): `slice(str, 2, -1)`
- Convert to C-strings: `to_c_string_temp(str)`

### Context System
```cpp
// Thread-local context contains allocator, logging, etc.
PUSH_CONTEXT(newContext) {
    // Temporary context override
}
PUSH_ALLOC(allocator) {
    // Temporary allocator override
}
```

### Cross-Platform Abstractions
- `OS` constants: `WINDOWS`, `LINUX`, `MACOS`
- Platform-specific code in `src/lstd/platform/`
- Use `os_*` functions for OS calls
- Threading: `create_thread()`, `create_mutex()`

## Development Workflows

### Building
```bash
./nob debug     # Debug with bounds checking
./nob optimized # Debug with O2 optimization  
./nob release   # Release build (default)
```

### Testing
- Run built test-suite: `build/debug/bin/test-suite`
- Tests in `test-suite/tests/` covering core functionality

### FFI Integration
- Rust library provides error reporting via annotate-snippets
- Handle management pattern for C↔Rust communication
- Memory safety through explicit handle cleanup

## Common Gotchas

1. **Always set Context.Alloc** before allocating
2. **Use `defer()` for cleanup** instead of destructors  
3. **Arrays are views by default** - call `reserve()` or `make_array()` to allocate
4. **No exceptions** - check return values or use structured bindings
5. **Unicode normalization** required for text processing
6. **Platform-specific defines** affect compilation (`LSTD_NO_CRT` on Windows)

## Key Files to Understand
- `include/lstd/lstd.h`: Main header with all modules
- `include/lstd/common.h`: Platform detection and core types
- `include/lstd/memory.h`: Allocator system and philosophy
- `include/lstd/array.h` + `array_like.h`: Container implementation
- `src/lstd/context.cpp`: Thread-local context management
- `nob.c`: Self-rebuilding build system

Focus on the memory management patterns and data-oriented design principles - they permeate every aspect of the codebase and differ significantly from standard C++ practices.