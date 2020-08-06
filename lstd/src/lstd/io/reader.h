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
constexpr char eof = (char) -1;

//
// @TODO: We should have a compile time macro to remove strict parsing checks.
// For example we check for the positions of the hyphens in the GUID and if they don't match we fail parsing.
// But if the user is parsing a lot of them and perfomance matters while also knowing that they are the right format
// and just wants to gets them as fast as possible we can have a flag (e.g. PARSE_NO_STRICT_CHECKS).
// (and "-000000-000--0000000-00000000-0000-00-00" would be considered a valid GUID cause we relax hyphen checking).
//

//
// @Locale
// I thought about something. If the input is the following: 100,141,542 how does our parse_int function know if
// that's three numbers of one but divided with commas for readibilty. There can be other such cases where the
// user expects certain behaviour but gets the wrong result because of our functions.
// We should have a way to force a format when parsing.
//

// The design here is that whenever possible we shouldn't do allocations, because that makes this API slow, and people
// will write specialized faster versions and then what's the point of having this reader in the library anyway?

//
// Provides a way to parse types and request bytes.
// Holds a pointer to a _request_byte_t_ procedure. Every other function in this class is implemented by calling that.
// That function should use the _Buffer_, _Current_ and _Available_ members to cache multiple bytes when available.
//
// @TODO: Tests tests tests!
struct reader : non_copyable, non_movable, non_assignable {
    // This is the only function required for the reader to work, it is called only when there are no more bytes available.
    // We pass _this_ as first argument. You can cast this pointer to the reader object implementation - e.g. (console_reader *) r;
    //
    // The buffer should be stored as a view in _Buffer_.
    // Return value doesn't matter unless it's _eof_ (equivalent to (char) -1, defined above this struct).
    // In the case in which you return _eof_ we just set the _EOF_ flag in this reader.
    // We can require you to do that manually but this seems less error-prone (the compiler throws an error if you forget to return something).
    using give_me_buffer_t = char (*)(reader *r);
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
    array<char> Buffer;

    // Whether this reader has reached "end of file"
    bool EOF = false;

    reader() = default;
    reader(give_me_buffer_t giveMeBuffer);

    // A bit more verbose and clear.
    s64 bytes_available() { return Buffer.Count; }

    // Calls _GiveMeBuffer_ and checks for _eof_.
    // Instead of calling this automatically (which might be wrong in some cases) we make that the responsibility of the user.
    // Call this whenever there are no bytes available or to discard the current buffer.
    //
    // @TODO: This is not a good explanation
    // This function also sets the EOF flag if the implementation returns _eof_. In that case there still may be a valid buffer,
    // because we don't explicitly clear it. The console reader, for example, continues to work after _eof_.
    // In that context _eof_ means end of one user input.
    void request_next_buffer();

    // Attemps to read _n_ bytes.
    //
    // If we run out of data we can't call _GiveMeBuffer_ and continue, because we can't return a view across two buffers, so we bail out.
    // The first return value is the stuff that was read and the second is how many bytes weren't read.
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
    pair<array<char>, s64> read_bytes(s64 n);

    // Attemps to read bytes until _delim_ is encountered. Return value doesn't include _delim_.
    // This is an optimized version that works on a couple bytes at a time instead of comparing byte by byte.
    // We have a similar function in "parse.h" called _eat_bytes_until_ which works on arrays.
    // We also have one for utf8 strings: _eat_code_points_until_.
    //
    // If we run out of data we can't call _GiveMeBuffer_ and continue, because we can't return a view across two buffers, so we bail out.
    // The first return value is the stuff that was read and the second is whether _delim_ was actually encountered.
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
    pair<array<char>, bool> read_bytes_until(char delim);

    // Attemps to read bytes until a byte that is in _delims_ is encountered. Return value doesn't include the delimeter.
    // @Speed This is the obvious version for now that checks byte by byte. Maybe we can optimize it?
    //
    // !!! Read the documentation for _read_bytes_until(char delim)_ above!
    pair<array<char>, bool> read_bytes_until(const array<char> &delims);

    // Attemps to read bytes until a byte that is different from _eats_ is encountered. Return value doesn't include the different byte.
    // This is an optimized version that works on a couple bytes at a time instead of comparing byte by byte.
    // We have a similar function in "parse.h" called _eat_bytes_while_ which works on arrays.
    // We also have one for utf8 strings: _eat_code_points_while_.
    //
    // If we run out of data we can't call _GiveMeBuffer_ and continue, because we can't return a view across two buffers, so we bail out.
    // The first return value is the stuff that was read and the second is whether a char different from _eats_ was actually encountered.
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
    pair<array<char>, bool> read_bytes_while(char eats);

    // Attemps to read bytes until a byte that is not in _anyOfThese_ is encountered. Return value doesn't include the different byte.
    // @Speed This is the obvious version for now that checks byte by byte. Maybe we can optimize it?
    //
    // !!! Read the documentation for _read_bytes_while(char eats)_ above!
    pair<array<char>, bool> read_bytes_while(const array<char> &anyOfThese);

    // !!! Doesn't safety check.
    // !!! Assumes there is enough data in _Buffer.
    //
    // Don't release the array returned by this function. It's just a view.
    // This library follows the convention that when a function is marked as [[nodiscard]] the returned value should be freed by the caller.
    array<char> read_bytes_unsafe(s64 n);

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

    /*
    // Returns the value in _Current_ and then increments it (WITHOUT safety checks!)
    const char *incr() {
        --Available;
        return Current++;
    }

    // Increments _Current_ and returns the value in it (WITHOUT safety checks!)
    const char *pre_incr() {
        --Available;
        return ++Current;
    }

    // Retrieves the current byte
    char peek_byte() {
        if (Available == 0) {
            return RequestByteFunction(this);
        }
        return *Current;
    }

    // Returns the current byte and increments _Current_
    char bump_byte() {
        if (Available == 0) {
            return RequestByteFunction(this);
        }
        return *incr();
    }

    // Returns the byte after the current one and also increments _Current_
    char next_byte() {
        if (Available <= 1) {
            if (bump_byte() == eof) return eof;
            return peek_byte();
        }
        return *pre_incr();
    }

   private:
    pair<bool, bool> parse_bool();
    pair<f64, bool> parse_float();
    pair<guid, bool> parse_guid();

#define check_eof(x)       \
    if (x == eof) {        \
        EOF = true;        \
        return {0, false}; \
    }


    */
};

// Specialize this for custom types that may not be POD or have data that isn't serialized, e.g. pointers
// template <typename T>
// bool deserialize(T *dest, reader *r) {
//     r->read((char *) dest, sizeof(T));
//     return true;
// }

}  // namespace io

LSTD_END_NAMESPACE
