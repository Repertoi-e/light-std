#pragma once

#include "reader.h"

LSTD_BEGIN_NAMESPACE

namespace io {

char string_reader_give_me_buffer(reader *r);

struct string_reader : reader {
    string Source;
    bool Exhausted;

    explicit string_reader(const string &src) : reader(string_reader_give_me_buffer), Source(src), Exhausted(false) {}
};

inline char string_reader_give_me_buffer(reader *r) {
    auto *sr = (string_reader *) r;

    if (sr->Exhausted) return eof;
    sr->Buffer.Data = (char *) sr->Source.Data;
    sr->Buffer.Count = sr->Source.ByteLength;
    sr->Exhausted = true;
    return 0;
}

}  // namespace io

LSTD_END_NAMESPACE