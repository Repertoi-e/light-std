#include "writer.hpp"

LSTD_BEGIN_NAMESPACE

namespace io {
void string_writer_write(void *data, const Memory_View &str) {
    auto *writer = (String_Writer *) data;
    writer->Builder.append_pointer_and_size(str.Data, str.ByteLength);
}

void counter_writer_write(void *data, const Memory_View &writeData) {
    auto *writer = (Counter_Writer *) data;
    writer->Count += writeData.ByteLength;
}

}  // namespace io

LSTD_END_NAMESPACE