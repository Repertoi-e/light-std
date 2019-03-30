#include "handle.hpp"

LSTD_BEGIN_NAMESPACE

namespace file {

Handle Handle::open_relative(file::Path path) const {
    path = Path / path;
    path.resolve();
    return Handle(path);
}

}  // namespace file

LSTD_END_NAMESPACE
