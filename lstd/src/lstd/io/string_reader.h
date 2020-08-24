#pragma once

#include "reader.h"

LSTD_BEGIN_NAMESPACE

namespace io {

byte string_reader_give_me_buffer(reader *r);

struct string_reader : reader {
    string Source;
    bool Exhausted;

    explicit string_reader(const string &src) : reader(string_reader_give_me_buffer), Source(src), Exhausted(false) {}
};

inline byte string_reader_give_me_buffer(reader *r) {
    auto *sr = (string_reader *) r;

    if (sr->Exhausted) return eof;
    sr->Buffer.Data = (byte *) sr->Source.Data;
    sr->Buffer.Count = sr->Source.Count;
    sr->Exhausted = true;
    return 0;
}

}  // namespace io

LSTD_END_NAMESPACE