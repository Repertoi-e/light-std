#pragma once

#include "reader.h"

LSTD_BEGIN_NAMESPACE

namespace io {

byte string_reader_request_byte(reader *data);

struct string_reader : reader {
    string Src;
    bool Exhausted;

    explicit string_reader(string src) : reader(string_reader_request_byte), Src(src), Exhausted(false) {}
};

// :ExplicitDeclareIsPod
DECLARE_IS_POD(string_reader, true);

inline byte string_reader_request_byte(reader *data) {
    auto *reader = (string_reader *) data;

    if (reader->Exhausted) return eof;
    reader->Buffer = reader->Src.Data;
    reader->Current = reader->Buffer;
    reader->Available = reader->Src.ByteLength;
    reader->Exhausted = true;
    return *reader->Current;
}

}  // namespace io

LSTD_END_NAMESPACE