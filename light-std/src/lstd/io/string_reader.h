#pragma once

#include "reader.h"

LSTD_BEGIN_NAMESPACE

namespace io {

byte string_reader_request_byte(reader *r);

struct string_reader : reader {
    string Src;
    bool Exhausted;

    explicit string_reader(string src) : reader(string_reader_request_byte), Src(src), Exhausted(false) {}
};

inline byte string_reader_request_byte(reader *r) {
    auto *sr = (string_reader *) r;

    if (sr->Exhausted) return eof;
    sr->Buffer = sr->Src.Data;
    sr->Current = sr->Buffer;
    sr->Available = sr->Src.ByteLength;
    sr->Exhausted = true;
    return *sr->Current;
}

}  // namespace io

LSTD_END_NAMESPACE