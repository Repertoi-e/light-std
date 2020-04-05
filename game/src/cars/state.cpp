#include "state.h"

void reload_global_state() {
    MANAGE_GLOBAL_STATE(GameState);
    MANAGE_GLOBAL_STATE(Scene);
    MANAGE_GLOBAL_STATE(Models);
    MANAGE_GLOBAL_STATE(Shaders);
    MANAGE_GLOBAL_STATE(AssetCatalog);
    AssetCatalog->ensure_initted(file::path("data/"));

    reload_scene();
}
