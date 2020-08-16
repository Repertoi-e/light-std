#include "writer.h"

#include "../memory/array.h"

LSTD_BEGIN_NAMESPACE

namespace io {
void writer::write(const array<char> &data) { WriteFunction(this, data.Data, data.Count); }
}  // namespace io

LSTD_END_NAMESPACE