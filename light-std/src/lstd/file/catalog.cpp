#include "catalog.h"

#include "handle.h"

LSTD_BEGIN_NAMESPACE

catalog::catalog(file::path root) { ensure_initted(root); }

void catalog::ensure_initted(file::path root) {
    if (Root.UnifiedPath.Length) return;
    assert(root.is_pointing_to_content() && "Create a catalog which points to a folder, not a file");
    clone(&Root, root);
}

void catalog::load(array<file::path> files, delegate<void(array<file::path>)> callback, bool watch, allocator alloc) {
    entity *e = Entities.add({}, alloc);
    e->FilesAssociated.reserve(files.Count);
    clone(&e->Callback, callback);
    e->Watched = watch;
    e->LastWriteTimes.reserve(files.Count);

    For(files) {
        file::path path = Root;
        path.combine_with(it);
        move(e->FilesAssociated.append(), &path);

        e->LastWriteTimes.append(file::handle(path).last_modification_time());
    }
    callback(e->FilesAssociated);
}

LSTD_END_NAMESPACE
