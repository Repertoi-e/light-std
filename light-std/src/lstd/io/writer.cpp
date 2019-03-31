#include "writer.hpp"

LSTD_BEGIN_NAMESPACE

namespace io {
void string_writer_write(void *data, const memory_view &memory) {
    auto *writer = (string_writer *) data;
    writer->Builder.append_pointer_and_size(memory.Data, memory.ByteLength);
}

void counter_writer_write(void *data, const memory_view &memory) {
    auto *writer = (counter_writer *) data;
    writer->Count += memory.ByteLength;
}

}  // namespace io

LSTD_END_NAMESPACE