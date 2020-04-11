#include "state.h"

void reload_global_state() {
    MANAGE_GLOBAL_STATE(GameState);
    MANAGE_GLOBAL_STATE(Scene);
    
    MANAGE_GLOBAL_STATE(Models);
    MANAGE_GLOBAL_STATE(Shaders);
    MANAGE_GLOBAL_STATE(Texture2Ds);
    
    MANAGE_GLOBAL_STATE(AssetCatalog);

    PUSH_CONTEXT(Alloc, GameMemory->Alloc) {
        AssetCatalog->ensure_initted(file::path("data/"));

        reload_scene();
    }
}
