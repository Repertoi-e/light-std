#pragma once

#include "../memory/array.h"
#include "reader.h"

LSTD_BEGIN_NAMESPACE

namespace io {

template <s64 ChunkSize>
char chunked_reader_give_me_buffer(reader *r);

// Reads in chunks from another reader.
// If the source reader reaches eof _chunked_reader_give_me_buffer_ returns eof but provides a partial chunk.
//
// You need to call release() on this to prevent leaks.
// We may have read a chunk across buffer boundaries and needed to concat the two in continuous memory.
template <s64 ChunkSize>
struct chunked_reader : reader {
    static constexpr s64 CHUNK_SIZE = ChunkSize;

    reader *Source;

    // We may need to allocate a buffer because we read across boundaries but we need to provide a continuous chunk size.
    // By default we use the context allocator.
    allocator Alloc;

    array<char> HelperBuffer;

    explicit chunked_reader(reader *src, allocator alloc = {}) : reader(chunked_reader_give_me_buffer<ChunkSize>), Source(src), Alloc(alloc) {
        if (!Alloc) Alloc = Context.Alloc;
    }

    void release() { HelperBuffer.release(); }

    // Call this right after _request_next_buffer_ to see if the chunk is only partial or not.
    bool is_chunk_whole() { return Buffer.Count == CHUNK_SIZE; }
};

template <s64 ChunkSize>
inline char chunked_reader_give_me_buffer(reader *r) {
    auto *sr = (chunked_reader<ChunkSize> *) r;

    if (sr->HelperBuffer.Count) sr->HelperBuffer.reset();
    if (!sr->Buffer.Data) sr->Source->request_next_buffer();

    auto [buffer, rest] = sr->Source->read_bytes(ChunkSize);
    if (rest != 0) {
        // If we reach EOF on the source reader we provide a partial chunk.
        if (sr->Source->EOF) {
            sr->Buffer = buffer;
            return eof;
        }

        // Continue requesting buffers from the source until we have the requested chunk size
        // Abort if we reach EOF
        WITH_ALLOC(sr->Alloc) { sr->HelperBuffer.append(buffer); }

        s64 totalRead = ChunkSize - rest;
        while (totalRead != ChunkSize) {
            sr->Source->request_next_buffer();
            auto [anotherBuffer, anotherRead] = sr->Source->read_bytes(ChunkSize - totalRead);

            // If we reach EOF on the source reader we provide a partial chunk.
            if (sr->Source->EOF) {
                sr->Buffer = sr->HelperBuffer;
                return eof;
            }

            WITH_ALLOC(sr->Alloc) { sr->HelperBuffer.append(anotherBuffer); }
            totalRead += anotherRead;
        }
        sr->Buffer = sr->HelperBuffer;
    } else {
        sr->Buffer = buffer;
    }
    return 0;
}

}  // namespace io

LSTD_END_NAMESPACE