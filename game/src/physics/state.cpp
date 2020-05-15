#include "state.h"

void reload_global_state() {
    MANAGE_GLOBAL_STATE(GameState);

    MANAGE_GLOBAL_STATE(AssetCatalog);

    MANAGE_GLOBAL_STATE(Shaders);
    MANAGE_GLOBAL_STATE(Texture2Ds);

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

    if (!Py_IsInitialized()) Py_Initialize();
}
