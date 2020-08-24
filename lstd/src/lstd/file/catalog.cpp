#include "catalog.h"

#include "handle.h"

LSTD_BEGIN_NAMESPACE

catalog::catalog(file::path root) { ensure_initted(root); }

void catalog::ensure_initted(const file::path &root) {
    if (Root.Str.Length) return;
    assert(root.is_pointing_to_content() && "Create a catalog which points to a folder, not a file");
    clone(&Root, root);
}

void catalog::load(const array_view<file::path>& files, const delegate<void(const array_view<file::path> &)> &callback, bool watch, allocator alloc) {
    entity *e = append(Entities, {}, alloc);
    reserve(e->FilesAssociated, files.Count);
    e->Callback = callback;
    e->Watched = watch;
    reserve(e->LastWriteTimes, files.Count);

    For(files) {
        file::path path = Root;
        path.combine_with(it);
        append(e->FilesAssociated, path);
        append(e->LastWriteTimes, file::handle(path).last_modification_time());
    }
    callback(e->FilesAssociated);
}

LSTD_END_NAMESPACE
