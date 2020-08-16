#pragma once

#include "file/catalog.h"
#include "file/handle.h"
#include "file/path.h"
#include "fmt.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

template <>
struct formatter<file::path> {
    void format(const file::path &path, format_context *f) {
        string displayWorthy = path.unified();
        f->write(displayWorthy);
        displayWorthy.release();
    }
};

template <>
struct formatter<file::handle> {
    void format(const file::handle &handle, format_context *f) {
        string displayWorthy = handle.Path.unified();
        f->write(displayWorthy);
        displayWorthy.release();
    }
};
}  // namespace fmt

LSTD_END_NAMESPACE
