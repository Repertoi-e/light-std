#include "handle.hpp"

LSTD_BEGIN_NAMESPACE

namespace file {

handle handle::open_relative(path path) const {
    path = Path / path;
    path.resolve();
    return handle(path);
}

}  // namespace file

LSTD_END_NAMESPACE
