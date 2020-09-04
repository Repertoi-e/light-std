#pragma once

#include "../memory/guid.h"
#include "../memory/string.h"

#if defined EOF
#undef EOF
#endif

LSTD_BEGIN_NAMESPACE

// @AvoidInclude
template <typename T>
struct array;

namespace io {

// Special constant to signify end of file.
constexpr byte eof = (byte) -1;

//
// The design here is that whenever possible we shouldn't do allocations, because that makes this API slow, and people
// will write specialized faster versions and then what's the point of having this reader in the library anyway?
//

//
// Provides a way to parse types and request bytes.
// Holds a pointer to a _request_byte_t_ procedure. Every other function in this class is implemented by calling that.
// That function should use the _Buffer_, _Current_ and _Available_ members to cache multiple bytes when available.
//
// Readers aren't thread-safe. That's the callers responsibility! (why would you want to read from multiple threads anyway... that seems weird?).
struct reader : non_copyable, non_movable, non_assignable {
    // This is the only function required for the reader to work, it is called only when there are no more bytes available.
    // We pass _this_ as first argument. You can cast this pointer to the reader object implementation - e.g. (console_reader *) r;
    //
    // The buffer should be stored as a view in _Buffer_.
    // Return value doesn't matter unless it's _eof_ (equivalent to (byte) -1, defined above this struct).
    // In the case in which you return _eof_ we just set the _EOF_ flag in this reader.
    // We can require you to do that manually but this seems less error-prone (the compiler throws an error if you forget to return something).
    using give_me_buffer_t = byte (*)(reader *r);
    give_me_buffer_t GiveMeBuffer = null;

    // Since the functions defined below (which all call _GiveMeBuffer_ when there are no more bytes available)
    // may return strings that point inside the buffer, reader implementations must keep the buffers alive until they are no longer used.
    // We do this to prevent unnecessary string copies. If we try to be extremely robust and fail-safe and assume we have no knowledge of
    // the implementation, we must copy the string and then return it. But then these functions become extremely slow for most cases.
    //
    // The reader implementation should be very clear about when the caller needs to copy the string
    // and when buffers get invalidaded, so the user doesn't get confused.
    //
    // This is stored as view itself. When reading we bump it's data pointer and reduce it's _Count_.
    bytes Buffer;

    // Whether this reader has reached "end of file"
    bool EOF = false;

    reader() {}
    reader(give_me_buffer_t giveMeBuffer);

    // A bit more verbose and clear.
    s64 bytes_available() { return Buffer.Count; }

    // Calls _GiveMeBuffer_ and checks for _eof_.
    // Instead of calling this automatically (which might be wrong in some cases) we make that the responsibility of the user.
    // Call this whenever there are no bytes available or to discard the current buffer.
    //
    // This function also sets the EOF flag of the reader if the implementation (GiveMeBuffer) returns _eof_.
    // In that case there still may be a valid buffer (because we don't explicitly clear it).
    // The console reader, for example, continues to work after _eof_ (because in that context _eof_ means end of just one user input).
    //
    // If the old buffer gets invalidated (freed or overwriten) then any results by read functions
    // in this reader may be invalid since we return views of the buffer.
    void request_next_buffer();

    struct read_byte_result {
        byte Byte;     // The value
        bool Success;  // False if buffer is exhausted
    };

    // Attemps to read just a single byte.
    read_byte_result read_byte();

    struct read_n_bytes_result {
        bytes Bytes;    // The stuff that was read
        s64 Remaining;  // How many bytes WERENT'T read
    };

    // Attemps to read _n_ bytes.
    //
    // If we run out of data we can't call _GiveMeBuffer_ and continue, because we can't return a view across two buffers, so we bail out.
    //
    // There are two cases in which the second return value is non-zero:
    //  * The reader reached "end of file" (in which case the _EOF_ flag above is set to true).
    //  * We got a new buffer but in order to avoid allocating a concatenated array we bailed out of the function.
    //
    // We leave it up to the caller to decide what to do in case this function fails.
    // In case _EOF_ is still false, you can call read_bytes again and again and concatenate
    // the arrays to get the full _n_ bytes. In that case an allocation is inevitable anyway.
    //
    // Don't release the array returned by this function. It's just a view.
    // This library follows the convention that when a function is marked as [[nodiscard]] the returned value should be freed by the caller.
    read_n_bytes_result read_bytes(s64 n);

    struct read_bytes_result {
        bytes Bytes;   // The stuff read
        bool Success;  // True if the terminator was actually encountered - false if buffer was exhausted before that
    };

    // Attemps to read bytes until _delim_ is encountered. Return value doesn't include _delim_.
    // This is an optimized version that works on a couple bytes at a time instead of comparing byte by byte.
    // We have a similar function in "parse.h" called _eat_bytes_until_ which works on arrays.
    // We also have one for utf8 strings: _eat_code_points_until_.
    //
    // If we run out of data we can't call _GiveMeBuffer_ and continue, because we can't return a view across two buffers, so we bail out.
    // The first return value is the stuff that was read and the second
    //
    // There are two cases in which the second return value is false:
    //  * The reader reached "end of file" (in which case the _EOF_ flag above is set to true).
    //  * We got a new buffer but in order to avoid allocating a concatenated array we bailed out of the function.
    //
    // We leave it up to the caller to decide what to do in case this function fails.
    // In case _EOF_ is still false, you can call read_bytes_until again and again and concatenate
    // the arrays to get the full however many bytes until _delim_. In that case an allocation is inevitable anyway.
    //
    // Don't release the array returned by this function. It's just a view.
    // This library follows the convention that when a function is marked as [[nodiscard]] the returned value should be freed by the caller.
    read_bytes_result read_bytes_until(byte delim);

    // Attemps to read bytes until a byte that is in _delims_ is encountered. Return value doesn't include the delimeter.
    // @Speed This is the naive version for now that checks byte by byte. Maybe we can optimize it?
    read_bytes_result read_bytes_until(bytes delims);

    // Attemps to read bytes until a byte that is different from _eats_ is encountered. Return value doesn't include the different byte.
    // This is an optimized version that works on a couple bytes at a time instead of comparing byte by byte.
    // We have a similar function in "parse.h" called _eat_bytes_while_ which works on arrays.
    // We also have one for utf8 strings: _eat_code_points_while_.
    //
    // If we run out of data we can't call _GiveMeBuffer_ and continue, because we can't return a view across two buffers, so we bail out.
    // The first return value is the stuff that was read and the second is whether a byte different from _eats_ was actually encountered.
    //
    // There are two cases in which the second return value is false:
    //  * The reader reached "end of file" (in which case the _EOF_ flag above is set to true).
    //  * We got a new buffer but in order to avoid allocating a concatenated array we bailed out of the function.
    //
    // We leave it up to the caller to decide what to do in case this function fails.
    // In case _EOF_ is still false, you can call read_bytes_while again and again and concatenate
    // the arrays to get the full however many bytes until _eats_ is not encountered. In that case an allocation is inevitable anyway.
    //
    // Don't release the array returned by this function. It's just a view.
    // This library follows the convention that when a function is marked as [[nodiscard]] the returned value should be freed by the caller.
    read_bytes_result read_bytes_while(byte eats);

    // Attemps to read bytes until a byte that is not in _anyOfThese_ is encountered. Return value doesn't include the different byte.
    // @Speed This is the obvious version for now that checks byte by byte. Maybe we can optimize it?
    //
    // !!! Read the documentation for _read_bytes_while(byte eats)_ above!
    read_bytes_result read_bytes_while(bytes anyOfThese);

    // !!! Doesn't safety check.
    // !!! Assumes there is enough data in _Buffer.
    //
    // Don't release the array returned by this function. It's just a view.
    // This library follows the convention that when a function is marked as [[nodiscard]] the returned value should be freed by the caller.
    bytes read_bytes_unsafe(s64 n);

    // !!! Doesn't safety check.
    // !!! Assumes _n_ bytes don't underflow the buffer.
    //
    // Call this incase you want to return the cursor to a previous state (rollback _n_ bytes).
    // The caller is responsible for not messing up this call.
    //
    // Since there are no hidden calls to _request_next_buffer_ and the caller
    // controls explictly all operations, this function is not scary.
    // It would've been a nightmare to implement robustly if we didn't decide on this API design.
    //
    // Of course we should maybe consider doing some bookkeeping to make sure this really never underflows,
    // but is it worth the overhead? And how do we make that optional?
    void go_backwards(s64 n);
};
}  // namespace io

LSTD_END_NAMESPACE
