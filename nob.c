// This is your build script. You only need to "bootstrap" it once with `cc -o nob nob.c`
// (you can call the executable whatever actually) or `cl nob.c` on MSVC. After that every
// time you run the `nob` executable if it detects that you modifed nob.c it will rebuild
// itself automatically thanks to NOB_GO_REBUILD_URSELF (see below)

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include <string.h>

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define INCLUDE_FOLDER "include/"
#define TEST_SUITE_FOLDER "test-suite/"
#define EXAMPLE_FOLDER "example/"

typedef enum {
    CONFIG_DEBUG,
    CONFIG_DEBUG_OPTIMIZED,
    CONFIG_RELEASE
} Config;

const char *config_names[] = {
    [CONFIG_DEBUG] = "Debug",
    [CONFIG_DEBUG_OPTIMIZED] = "DebugOptimized", 
    [CONFIG_RELEASE] = "Release"
};

void print_usage(const char *program_name) {
    nob_log(INFO, "Usage: %s [config]\n", program_name);
    nob_log(INFO, "\nConfigurations:\n");
    nob_log(INFO, "  debug      - Debug build with bounds checking\n");
    nob_log(INFO, "  optimized  - Debug build with optimizations\n");
    nob_log(INFO, "  release    - Release build (default)\n");
}

// Thin wrapper functions for cross-platform compiler options
// These map premake5 options to their equivalent compiler flags:
//
// Premake5 -> nob_* function -> Compiler flags
// ============================================
// 
// buildoptions { "/utf-8" } -> nob_utf8_support() -> MSVC: /utf-8, GCC/Clang: (default)
// characterset "Unicode" -> nob_character_set("Unicode") -> MSVC: /DUNICODE /D_UNICODE, GCC/Clang: -DUNICODE -D_UNICODE
// optimize "Off" -> nob_optimize_level("Off") -> MSVC: /Od, GCC/Clang: -O0
// optimize "On" -> nob_optimize_level("On") -> MSVC: /O2, GCC/Clang: -O2
// optimize "Full" -> nob_optimize_level("Full") -> MSVC: /Ox, GCC/Clang: -O3
// rtti "Off" -> nob_rtti(false) -> MSVC: /GR-, GCC/Clang: -fno-rtti  
// exceptionhandling "Off" -> nob_exceptions(false) -> MSVC: /EHs-c-, GCC/Clang: -fno-exceptions
// staticruntime "On" -> nob_static_runtime(true) -> MSVC: /MT, GCC/Clang: -static-libgcc
// symbols "On" -> nob_debug_info(true) -> MSVC: /Zi, GCC/Clang: -g
// flags { "NoIncrementalLink" } -> nob_incremental_linking(false) -> MSVC: /INCREMENTAL:NO
// flags { "NoRuntimeChecks" } -> nob_runtime_checks(false) -> MSVC: (no /RTC1), GCC/Clang: (no -fstack-protector-strong)
// flags { "NoBufferSecurityCheck" } -> nob_buffer_security_check(false) -> MSVC: /GS-, GCC/Clang: -fno-stack-protector
// entrypoint "main_no_crt" -> nob_entry_point("main_no_crt") -> MSVC: /ENTRY:main_no_crt, GCC/Clang: -Wl,-e,main_no_crt
// linkoptions { "/subsystem:windows" } -> nob_subsystem("WINDOWS") -> MSVC: /SUBSYSTEM:WINDOWS
// linkoptions { "/stack:\"0x100000\",\"0x100000\"" } -> nob_stack_size("0x100000", "0x100000") -> MSVC: /STACK:0x100000,0x100000, GCC/Clang: -Wl,-z,stack-size=0x100000
// flags { "OmitDefaultLibrary" } -> nob_no_default_libs() -> MSVC: /NODEFAULTLIB, GCC/Clang: -nostdlib
// floatingpoint "Strict" -> nob_floating_point_model("strict") -> MSVC: /fp:strict, GCC/Clang: -ffp-contract=off
// buildoptions { "/Gs9999999" } -> nob_stack_probe_threshold("9999999") -> MSVC: /Gs9999999, GCC/Clang: -fstack-clash-protection

void nob_utf8_support(Cmd *cmd) {
#ifdef _WIN32
    cmd_append(cmd, "/utf-8");
#else
    // UTF-8 is default on Unix systems
#endif
}

void nob_rtti(Cmd *cmd, bool enabled) {
#ifdef _WIN32
    cmd_append(cmd, enabled ? "/GR" : "/GR-");
#else
    cmd_append(cmd, enabled ? "-frtti" : "-fno-rtti");
#endif
}

void nob_exceptions(Cmd *cmd, bool enabled) {
#ifdef _WIN32
    cmd_append(cmd, enabled ? "/EHsc" : "/EHs-c-");
#else
    cmd_append(cmd, enabled ? "-fexceptions" : "-fno-exceptions");
#endif
}

void nob_static_runtime(Cmd *cmd, bool enabled) {
#ifdef _WIN32
    cmd_append(cmd, enabled ? "/MT" : "/MD");
#else
#endif
}

void nob_debug_info(Cmd *cmd, bool enabled) {
#ifdef _WIN32
    cmd_append(cmd, enabled ? "/Zi" : "");
#else
    cmd_append(cmd, enabled ? "-g" : "");
#endif
}

void nob_incremental_linking(Cmd *cmd, bool enabled) {
#ifdef _WIN32
    cmd_append(cmd, enabled ? "/INCREMENTAL" : "/INCREMENTAL:NO");
#else
    // Not applicable to GCC/Clang
#endif
}

void nob_runtime_checks(Cmd *cmd, bool enabled) {
#ifdef _WIN32
    cmd_append(cmd, enabled ? "/RTC1" : "");
#else
    // Runtime checks are different on GCC/Clang
    if (enabled) {
        cmd_append(cmd, "-fstack-protector-strong");
    }
#endif
}

void nob_buffer_security_check(Cmd *cmd, bool enabled) {
#ifdef _WIN32
    cmd_append(cmd, enabled ? "/GS" : "/GS-");
#else
#endif
}

void nob_entry_point(Cmd *cmd, const char *entry_name) {
#ifdef _WIN32
    cmd_append(cmd, temp_sprintf("/ENTRY:%s", entry_name));
#else
    cmd_append(cmd, temp_sprintf("-Wl,-e,%s", entry_name));
#endif
}

void nob_subsystem(Cmd *cmd, const char *subsystem) {
#ifdef _WIN32
    cmd_append(cmd, temp_sprintf("/SUBSYSTEM:%s", subsystem));
#else
    // Not directly applicable to Unix systems
#endif
}

void nob_stack_size(Cmd *cmd, const char *reserve, const char *commit) {
#ifdef _WIN32
    cmd_append(cmd, temp_sprintf("/STACK:%s,%s", reserve, commit));
#else
    cmd_append(cmd, temp_sprintf("-Wl,-z,stack-size=%s", reserve));
#endif
}

void nob_no_default_libs(Cmd *cmd) {
#ifdef _WIN32
    cmd_append(cmd, "/NODEFAULTLIB");
#else
    cmd_append(cmd, "-nostdlib");
#endif
}

void nob_floating_point_model(Cmd *cmd, const char *model) {
#ifdef _WIN32
    cmd_append(cmd, temp_sprintf("/fp:%s", model));
#else
    if (strcmp(model, "strict") == 0) {
        cmd_append(cmd, "-ffp-contract=off");
    } else if (strcmp(model, "fast") == 0) {
        cmd_append(cmd, "-ffast-math");
    }
#endif
}

void nob_stack_probe_threshold(Cmd *cmd, const char *size) {
#ifdef _WIN32
    cmd_append(cmd, temp_sprintf("/Gs%s", size));
#else
#endif
}

void nob_character_set(Cmd *cmd, const char *charset) {
#ifdef _WIN32
    if (strcmp(charset, "Unicode") == 0) {
        cmd_append(cmd, "/DUNICODE", "/D_UNICODE");
    } else if (strcmp(charset, "MBCS") == 0) {
        cmd_append(cmd, "/D_MBCS");
    }
#else
    // Unicode is default on Unix systems
    if (strcmp(charset, "Unicode") == 0) {
        cmd_append(cmd, "-DUNICODE", "-D_UNICODE");
    }
#endif
}

void nob_optimize_level(Cmd *cmd, const char *level) {
#ifdef _WIN32
    if (strcmp(level, "Off") == 0) {
        cmd_append(cmd, "/Od");
    } else if (strcmp(level, "On") == 0) {
        cmd_append(cmd, "/O2");
    } else if (strcmp(level, "Full") == 0) {
        cmd_append(cmd, "/Ox");
    }
#else
    if (strcmp(level, "Off") == 0) {
        cmd_append(cmd, "-O0");
    } else if (strcmp(level, "On") == 0) {
        cmd_append(cmd, "-O2");
    } else if (strcmp(level, "Full") == 0) {
        cmd_append(cmd, "-O3");
    }
#endif
}

void add_common_flags(Cmd *cmd, Config config) {
    // Language and standard
    cmd_append(cmd, "-std=c++20");
    cmd_append(cmd, "-Wall", "-Wextra");
    cmd_append(cmd, "-Wno-unused-but-set-variable");
    cmd_append(cmd, "-Wno-unused-variable");
    cmd_append(cmd, "-Wno-unused-parameter");
    cmd_append(cmd, "-Wno-missing-braces");
    cmd_append(cmd, "-Wno-unused-function");
    cmd_append(cmd, "-Wno-sign-compare");

    // UTF-8 support and character set (from premake characterset "Unicode")
    nob_utf8_support(cmd);
    nob_character_set(cmd, "Unicode");
    
    // Configuration-specific flags  
    switch (config) {
        case CONFIG_DEBUG:
            nob_optimize_level(cmd, "Off");
            cmd_append(cmd, "-DDEBUG");
            cmd_append(cmd, "-DLSTD_ARRAY_BOUNDS_CHECK", "-DLSTD_NUMERIC_CAST_CHECK");
            nob_debug_info(cmd, true);
            break;
        case CONFIG_DEBUG_OPTIMIZED:
            nob_optimize_level(cmd, "On");
            cmd_append(cmd, "-DDEBUG", "-DDEBUG_OPTIMIZED");
            cmd_append(cmd, "-DLSTD_ARRAY_BOUNDS_CHECK", "-DLSTD_NUMERIC_CAST_CHECK");
            nob_debug_info(cmd, true);
#ifdef _WIN32
            nob_floating_point_model(cmd, "strict");
#endif
            break;
        case CONFIG_RELEASE:
            nob_optimize_level(cmd, "Full");
            cmd_append(cmd, "-DNDEBUG", "-DRELEASE");
            nob_debug_info(cmd, false);
#ifdef _WIN32
            nob_floating_point_model(cmd, "strict");
#endif
            break;
    }
    
    nob_rtti(cmd, false);
    nob_exceptions(cmd, false);
    nob_static_runtime(cmd, true);
    nob_incremental_linking(cmd, false);
    nob_runtime_checks(cmd, false);

    // Platform-specific defines
#ifdef __APPLE__
    // macOS specific flags
    cmd_append(cmd, "-pthread");
#elif defined(__linux__)
    // Linux specific flags  
    cmd_append(cmd, "-pthread");
#elif defined(_WIN32)
    cmd_append(cmd, "-DNOMINMAX", "-DWIN32_LEAN_AND_MEAN", "-D_CRT_SUPPRESS_RESTRICT");
    cmd_append(cmd, "-DLSTD_NO_CRT");

    nob_buffer_security_check(cmd, false);
    nob_stack_probe_threshold(cmd, "9999999");
#endif
    
    // Library-specific defines
    cmd_append(cmd, "-DLSTD_NO_NAMESPACE");
    cmd_append(cmd, "-DPLATFORM_TEMPORARY_STORAGE_STARTING_SIZE=16_KiB");
    cmd_append(cmd, "-DPLATFORM_PERSISTENT_STORAGE_STARTING_SIZE=1_MiB");
    
    // Include directories
    cmd_append(cmd, "-I" INCLUDE_FOLDER);
}

void add_windows_no_crt_link_flags(Cmd *cmd) {
#ifdef _WIN32
    nob_no_default_libs(cmd);
    nob_subsystem(cmd, "WINDOWS");
    nob_stack_size(cmd, "0x100000", "0x100000");
    cmd_append(cmd, "-lkernel32", "-lshell32");
#endif
}

void add_windows_exe_entry_point(Cmd *cmd) {
#ifdef _WIN32
    nob_entry_point(cmd, "main_no_crt");
#endif
}

bool build_lstd_library(Config config) {
    nob_log(INFO, "Building lstd library (%s)\n", config_names[config]);
    
    if (!mkdir_if_not_exists(BUILD_FOLDER)) return false;
    if (!mkdir_if_not_exists(BUILD_FOLDER "obj/")) return false;
    if (!mkdir_if_not_exists(BUILD_FOLDER "lib/")) return false;

    const char *input = SRC_FOLDER "lstd/lib.cpp";
    
    // Generate object file path  
    const char *obj_file = temp_sprintf("%sobj/lstd_lib.o", BUILD_FOLDER);
    
    if (needs_rebuild1(obj_file, input)) {
        Cmd cmd = {0};
        cmd_append(&cmd, "c++");
        
        add_common_flags(&cmd, config);
        cmd_append(&cmd, "-fPIC");
        cmd_append(&cmd, "-c"); // Compile to object file only
        
        // Additional include for cephes math
        cmd_append(&cmd, "-I" INCLUDE_FOLDER "lstd/vendor/cephes/cmath/");
        
        // Input and output
        nob_cc_inputs(&cmd, input);
        nob_cc_output(&cmd, obj_file);
        
        if (!cmd_run_sync(cmd)) return false;
    }
    
    const char *lib_path = BUILD_FOLDER "lib/liblstd.a";
    if (needs_rebuild1(lib_path, obj_file)) {
        Cmd cmd = {0};
        cmd_append(&cmd, "ar", "rcs", lib_path);
        cmd_append(&cmd, obj_file);
        if (!cmd_run_sync(cmd)) return false;
    }
    
    nob_log(INFO, "lstd library built successfully: %s\n", lib_path);
    return true;
}

bool build_executable(const char *name, const char *unity_cpp, Config config) {
    nob_log(INFO, "Building %s (%s)\n", name, config_names[config]);
    
    if (!mkdir_if_not_exists(BUILD_FOLDER "bin/")) return false;
    
    const char *exe_path = temp_sprintf("%sbin/%s", BUILD_FOLDER, name);
    
    bool needs_rebuild_exe = needs_rebuild1(exe_path, BUILD_FOLDER "lib/liblstd.a");
    if (!needs_rebuild_exe) {
        needs_rebuild_exe = needs_rebuild1(exe_path, unity_cpp);
    }
    
    if (needs_rebuild_exe) {
        Cmd cmd = {0};
        cmd_append(&cmd, "c++");
        
        add_common_flags(&cmd, config);
        
        // Source files
        nob_cc_inputs(&cmd, unity_cpp);
        nob_cc_output(&cmd, exe_path);
        
        // Link with lstd library
        cmd_append(&cmd, "-L" BUILD_FOLDER "lib");
        cmd_append(&cmd, "-llstd");
        
        // Platform-specific libraries and linking
#ifdef __linux__
        cmd_append(&cmd, "-lpthread", "-ldl");
#elif defined(__APPLE__)
        cmd_append(&cmd, "-lpthread", "-ldl");
#elif defined(_WIN32)
        add_windows_no_crt_link_flags(&cmd);
        add_windows_exe_entry_point(&cmd);
        cmd_append(&cmd, "-ldbghelp");
#endif
        
        if (!cmd_run_sync(cmd)) return false;
    }
    
    nob_log(INFO, "%s built successfully: %s\n", name, exe_path);
    return true;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    Config config = CONFIG_RELEASE;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "help") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "debug") == 0) {
            config = CONFIG_DEBUG;
        } else if (strcmp(argv[i], "optimized") == 0) {
            config = CONFIG_DEBUG_OPTIMIZED;
        } else if (strcmp(argv[i], "release") == 0) {
            config = CONFIG_RELEASE;
        } else {
            nob_log(ERROR, "Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (!build_lstd_library(config)) {
        nob_log(ERROR, "Failed to build lstd library\n");
        return 1;
    }
    
    if (!build_executable("test-suite", TEST_SUITE_FOLDER "main.cpp", config)) {
        nob_log(ERROR, "Failed to build test-suite\n");
        return 1;
    }

    if (!build_executable("example", EXAMPLE_FOLDER "main.cpp", config)) {
        nob_log(ERROR, "Failed to build example\n");
        return 1;
    }

    nob_log(INFO, "Build completed successfully!\n");
    return 0;
}