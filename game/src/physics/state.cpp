#include "state.h"

void reload_global_state() {
    Context.AllocAlignment = 16;  // For SIMD

    MANAGE_GLOBAL_STATE(GameState);

    MANAGE_GLOBAL_STATE(AssetCatalog);

    MANAGE_GLOBAL_STATE(Shaders);
    MANAGE_GLOBAL_STATE(Texture2Ds);

    // We need these in python.pyb
    GameState->Memory = GameMemory;
#if defined DEBUG_MEMORY
    GameState->DEBUG_Head = allocator::DEBUG_Head;
    GameState->DEBUG_Mutex = &allocator::DEBUG_Mutex;
#endif
    GameState->AllocationCount = allocator::AllocationCount;

    AssetCatalog->ensure_initted(file::path("data/"));

    //
    // This is very specific but still better than nothing...
    // The problem is that Py_Initialize doesn't find "encodings" module even though we have python in our path.
    // Setting PYTHONPATH and PYTHONHOME seems to fix the problem.
    //
    string path;
    if (os_get_env(&path, "PATH")) {
        if (path.Length) {
            array<string> paths;

            size_t semicolon, beginning = 0;
            while ((semicolon = path.find(";", beginning)) != npos) {
                // @Bug Big one... we can't store an array of substrings because when we resize the container and
                // move the elements, old ones attempt to get freed.
                paths.append(path.substring(beginning, semicolon));
                beginning += semicolon - beginning + 1;
            }

            string found;
            For(paths) {
                if (it.ends_with("ProgramData\\Anaconda3")) found = it;
                if (it.ends_with("ProgramData/Anaconda3")) found = it;
            }
            os_set_env("PYTHONPATH", found);
            os_set_env("PYTHONHOME", found);
        }
    }

    // PyMemAllocatorEx a;
    // a.ctx = null;
    // a.malloc = [](void *, size_t size) { return (void *) new char[size == 0 ? 1 : size]; };
    // a.calloc = [](void *, size_t n, size_t s) { return (void *) new (Context.Alloc, DO_INIT_0) char[n * s]; };
    // a.realloc = [](void *, void *ptr, size_t size) { return allocator::reallocate(ptr, size == 0 ? 1 : size); };
    // a.free = [](void *, void *ptr) { delete ptr; };
    // 
    // PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &a);
    // PyMem_SetAllocator(PYMEM_DOMAIN_MEM, &a);  // @Speed These can be different thread-unsafe allocators.
    // PyMem_SetAllocator(PYMEM_DOMAIN_OBJ, &a);  // @Speed These can be different thread-unsafe allocators.
    // 
    // // @Speed This can be an arena allocator
    // PyObjectArenaAllocator aa;
    // aa.ctx = null;
    // a.malloc = [](void *, size_t size) { return (void *) new char[size == 0 ? 1 : size]; };
    // a.realloc = [](void *, void *ptr, size_t size) { return allocator::reallocate(ptr, size == 0 ? 1 : size); };
    // a.free = [](void *, void *ptr) { delete ptr; };
    // PyObject_SetArenaAllocator(&aa);

    // PyMem_SetupDebugHooks();

    Py_Initialize();
    reload_python_script();
    GameState->PyFirstTime = false;
}

void reload_python_script() {
    file::path scripts = os_get_working_dir();
    scripts.combine_with("data/scripts");

    if (!file::handle(scripts).is_directory()) {
        GameState->PyLoaded = false;
        fmt::print(
            ">>>\n>>> Couldn't find {!YELLOW}\"data/scripts\"{!} folder in current working dir ({!GRAY}\"{}\"{!})\n",
            scripts);
        fmt::print(
            ">>> There must be a file named {!YELLOW}data/scripts/physics_main.py{!} relative to the current working "
            "directory in order to run.\n>>>\n");
    } else {
        if (GameState->PyFirstTime) {
            PyObject *sysPath = PySys_GetObject("path");

            const char *path = scripts.UnifiedPath.to_c_string(Context.TemporaryAlloc);
            PyList_Append(sysPath, PyUnicode_FromString(path));

            try {
                GameState->PyModule = py::module::import("physics_main");

                auto *module = &GameState->PyModule;
                module->attr("init")();

                GameState->PyReload = (py::function) module->attr("reload");
                GameState->PyReload((u64) GameState);

                GameState->PyFrame = (py::function) module->attr("frame");
            } catch (py::error_already_set e) {
                report_python_error(e);
            }
        } else {
            try {
                GameState->PyModule.reload();

                auto *module = &GameState->PyModule;

                GameState->PyReload = (py::function) module->attr("reload");
                GameState->PyReload((u64) GameState);

                GameState->PyFrame = (py::function) module->attr("frame");
            } catch (py::error_already_set e) {
                report_python_error(e);
            }
        }
        GameState->PyLoaded = true;
    }
}

void report_python_error(py::error_already_set &e) {
    e.restore();

    fmt::print(">>> An {!RED}error{!} occured in python. Here is the stack trace:\n");
    PyErr_Print();
    fmt::print("\n");
}
