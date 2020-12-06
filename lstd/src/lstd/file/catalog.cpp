#include "catalog.h"

#include "handle.h"

LSTD_BEGIN_NAMESPACE

catalog::catalog(const string &root) { ensure_initted(root); }

void catalog::ensure_initted(const string &root) {
    if (Root.Length) return;

    // @TODO: Replace with is_dir
    assert(path::is_sep(root[-1]) && "Create a catalog which points to a folder, not a file");
    clone(&Root, root);
}

void catalog::load(const array_view<string> &files, const delegate<void(const array_view<string> &)> &callback, bool watch, allocator alloc) {
    entity *e = append(Entities, {}, alloc);
    reserve(e->FilesAssociated, files.Count);
    e->Callback = callback;
    e->Watched = watch;
    reserve(e->LastWriteTimes, files.Count);

    For(files) {
        auto path = path::join(Root, it);
        append(e->FilesAssociated, path);
        append(e->LastWriteTimes, file::handle(path).last_modification_time());
    }
    callback(e->FilesAssociated);
}

LSTD_END_NAMESPACE
