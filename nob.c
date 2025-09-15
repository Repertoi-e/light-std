// This is your build script. You only need to "bootstrap" it once with `cc -o nob nob.c`
// (you can call the executable whatever actually) or `cl nob.c` on MSVC. After that every
// time you run the `nob` executable if it detects that you modifed nob.c it will rebuild
// itself automatically thanks to NOB_GO_REBUILD_URSELF (see below)

#define NOB_EXPERIMENTAL_DELETE_OLD
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include <string.h>

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define INCLUDE_FOLDER "include/"
#define TEST_SUITE_FOLDER "test-suite/"

typedef enum
{
    CONFIG_DEBUG,
    CONFIG_DEBUG_OPTIMIZED,
    CONFIG_RELEASE
} Config;

const char *config_names[] = {
    [CONFIG_DEBUG] = "Debug",
    [CONFIG_DEBUG_OPTIMIZED] = "DebugOptimized",
    [CONFIG_RELEASE] = "Release"};

// Helper function to get config-specific build folder
const char *get_build_folder(Config config)
{
    switch (config)
    {
    case CONFIG_DEBUG:
        return BUILD_FOLDER "debug/";
    case CONFIG_DEBUG_OPTIMIZED:
        return BUILD_FOLDER "optimized/";
    case CONFIG_RELEASE:
        return BUILD_FOLDER "release/";
    default:
        return BUILD_FOLDER "release/";
    }
}

void print_usage(const char *program_name)
{
    nob_log(INFO, "Usage: %s [config]\n", program_name);
    nob_log(INFO, "\nConfigurations:\n");
    nob_log(INFO, "  debug      - Debug build with bounds checking\n");
    nob_log(INFO, "  optimized  - Debug build with optimizations\n");
    nob_log(INFO, "  release    - Release build (default)\n");
}

void add_common_flags(Cmd *cmd, Config config)
{
    // Language and standard
    nob_cc_flags(cmd);
    nob_language_cpp(cmd, "c++20");

    // To see compile time breakdown:
    // cmd_append(cmd, "-Xclang", "-H", "-ftime-report");
    
    // Configuration-specific flags
    switch (config)
    {
    case CONFIG_DEBUG:
        nob_optimize_level(cmd, NOB_OPTIMIZATION_O0);
        cmd_append(cmd, "-DDEBUG");
        cmd_append(cmd, "-DLSTD_ARRAY_BOUNDS_CHECK", "-DLSTD_NUMERIC_CAST_CHECK");
        nob_debug_info(cmd, true);
        break;
    case CONFIG_DEBUG_OPTIMIZED:
        nob_optimize_level(cmd, NOB_OPTIMIZATION_O2);
        cmd_append(cmd, "-DDEBUG", "-DDEBUG_OPTIMIZED");
        cmd_append(cmd, "-DLSTD_ARRAY_BOUNDS_CHECK", "-DLSTD_NUMERIC_CAST_CHECK");
        nob_debug_info(cmd, true);
        break;
    case CONFIG_RELEASE:
        nob_optimize_level(cmd, NOB_OPTIMIZATION_O3);
        cmd_append(cmd, "-DNDEBUG", "-DRELEASE");
        nob_debug_info(cmd, false);
        break;
    }

    nob_rtti(cmd, true);
    nob_exceptions(cmd, false);

#if defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
    cmd_append(cmd, "-pthread");

    cmd_append(cmd, "-Wno-unused-but-set-variable");
    cmd_append(cmd, "-Wno-unused-variable");
    cmd_append(cmd, "-Wno-unused-parameter");
    cmd_append(cmd, "-Wno-unused-function");
    cmd_append(cmd, "-Wno-sign-compare");
#elif defined(_WIN32)
    cmd_append(cmd, "-DNOMINMAX", "-DWIN32_LEAN_AND_MEAN", "-D_CRT_SUPPRESS_RESTRICT");
    cmd_append(cmd, "-DLSTD_NO_CRT");

    cmd_append(cmd, "/utf-8");
    cmd_append(cmd, "/DUNICODE", "/D_UNICODE");

    cmd_append(cmd, "/MT");

    cmd_append(cmd, "/INCREMENTAL:NO");
    cmd_append(cmd, "/GS-");
    cmd_append(cmd, temp_sprintf("/Gs%s", 9999999));
#endif

    // Library-specific defines
    cmd_append(cmd, "-DLSTD_NO_NAMESPACE");
    // cmd_append(cmd, "-DLSTD_UNICODE_FULL_RANGE"); This adds around 25 MB to the binary size
    cmd_append(cmd, "-DPLATFORM_TEMPORARY_STORAGE_STARTING_SIZE=16_KiB");
    cmd_append(cmd, "-DPLATFORM_PERSISTENT_STORAGE_STARTING_SIZE=1_MiB");

    // Include directories
    cmd_append(cmd, "-I" INCLUDE_FOLDER);
}

bool build_lstd_library(Config config)
{
    nob_log(INFO, "Building lstd library (%s)\n", config_names[config]);

    const char *build_folder = get_build_folder(config);
    
    if (!mkdir_if_not_exists(BUILD_FOLDER))
        return false;
    if (!mkdir_if_not_exists(build_folder))
        return false;
    if (!mkdir_if_not_exists(temp_sprintf("%sobj/", build_folder)))
        return false;
    if (!mkdir_if_not_exists(temp_sprintf("%slib/", build_folder)))
        return false;

    const char *input = SRC_FOLDER "lstd/lib.cpp";

    // Ensure generated Unicode tables exist and are up to date w.r.t. the generator script
    const char *unicode_inc = SRC_FOLDER "lstd/unicode_tables.inc";
    const char *unicode_gen = "tools/gen_unicode.py";
    int inc_exists = nob_file_exists(unicode_inc);
    int regen_needed = !inc_exists || nob_needs_rebuild1(unicode_inc, unicode_gen);
    if (regen_needed) {
        nob_log(INFO, "Generating Unicode tables (%s)\n", unicode_inc);
        Cmd gen = {0};
        cmd_append(&gen, "python3", unicode_gen);
        if (!cmd_run_sync(gen)) return false;
    }

    // Generate object file path
    const char *obj_file = temp_sprintf("%sobj/lstd_lib.o", build_folder);

    Nob_File_Paths source_dirs = {0};
    da_append(&source_dirs, SRC_FOLDER "lstd", INCLUDE_FOLDER "lstd");

    bool needs_rebuild_obj = needs_rebuild_cpp_sources(obj_file, source_dirs);
    // Also trigger rebuild if the generated unicode tables changed
    if (!needs_rebuild_obj) {
        needs_rebuild_obj = nob_needs_rebuild1(obj_file, unicode_inc);
    }
    if (needs_rebuild_obj)
    {
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

        if (!cmd_run_sync(cmd))
            return false;
    }

    const char *lib_path = temp_sprintf("%slib/liblstd.a", build_folder);
    if (needs_rebuild1(lib_path, obj_file))
    {
        Cmd cmd = {0};
        cmd_append(&cmd, "ar", "rcs", lib_path);
        cmd_append(&cmd, obj_file);
        if (!cmd_run_sync(cmd))
            return false;
    }
    return true;
}

bool build_test_suite(Config config)
{
    nob_log(INFO, "Building test-suite (%s)\n", config_names[config]);

    const char *build_folder = get_build_folder(config);
    if (!mkdir_if_not_exists(temp_sprintf("%sbin/", build_folder)))
        return false;

    File_Paths test_dirs = {0};
    da_append(&test_dirs, TEST_SUITE_FOLDER);

    const char *unity_cpp = TEST_SUITE_FOLDER "main.cpp";
    const char *exe_path = temp_sprintf("%sbin/%s", build_folder, "test-suite");

    // Check if rebuild is needed against library and source files
    bool needs_rebuild_exe = needs_rebuild1(exe_path, temp_sprintf("%slib/liblstd.a", build_folder));
    if (!needs_rebuild_exe)
    {
        needs_rebuild_exe = needs_rebuild1(exe_path, unity_cpp);
    }
    if (!needs_rebuild_exe)
    {
        needs_rebuild_exe = needs_rebuild_cpp_sources(exe_path, test_dirs);
    }

    if (needs_rebuild_exe)
    {
        Cmd cmd = {0};
        cmd_append(&cmd, "c++");

        add_common_flags(&cmd, config);

        // Source files
        nob_cc_inputs(&cmd, unity_cpp);
        nob_cc_output(&cmd, exe_path);

        // Link with lstd library
        cmd_append(&cmd, temp_sprintf("-L%slib", build_folder));
        cmd_append(&cmd, "-llstd");

        // Platform-specific libraries and linking
#if defined(__linux__) || defined(__APPLE__)
        cmd_append(&cmd, "-lpthread", "-ldl");
#elif defined(_WIN32)
        nob_no_default_libs(cmd);
        nob_subsystem(cmd, "WINDOWS");
        nob_stack_size(cmd, "0x100000", "0x100000");
        cmd_append(cmd, "-lkernel32", "-lshell32");
        nob_entry_point(cmd, "main_no_crt");

        cmd_append(&cmd, "-ldbghelp");
#endif
        if (!cmd_run_sync(cmd))
            return false;
    }
    return true;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    Config config = CONFIG_DEBUG;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "help") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "debug") == 0)
        {
            config = CONFIG_DEBUG;
        }
        else if (strcmp(argv[i], "optimized") == 0)
        {
            config = CONFIG_DEBUG_OPTIMIZED;
        }
        else if (strcmp(argv[i], "release") == 0)
        {
            config = CONFIG_RELEASE;
        }
        else
        {
            nob_log(ERROR, "Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!build_lstd_library(config))
    {
        nob_log(ERROR, "Failed to build lstd library\n");
        return 1;
    }

    if (!build_test_suite(config))
    {
        nob_log(ERROR, "Failed to build test-suite\n");
        return 1;
    }
    return 0;
}