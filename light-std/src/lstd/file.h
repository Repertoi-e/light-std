#pragma once

#include "io/fmt.h"

#include "file/handle.h"
#include "file/path.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

template <>
struct formatter<file::path> {
    void format(file::path path, format_context *f) { f->write(path.UnifiedPath); }
};

template <>
struct formatter<file::handle> {
    void format(file::handle handle, format_context *f) { f->write(handle.Path.UnifiedPath); }
};
}  // namespace fmt

LSTD_END_NAMESPACE
