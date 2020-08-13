#pragma once

#include "reader.h"

LSTD_BEGIN_NAMESPACE

namespace io {
    
    // Defined in *platform*_common.cpp
    char console_reader_request_byte(reader *r);
    
    struct console_reader : reader {
        // By default, we are thread-safe.
        // If you don't use seperate threads and aim for max performance, set this to false.
        bool LockMutex = true;
        
        // console_reader() : reader(console_reader_request_byte) {}
        
        private:
        /*
        reader *read(char32_t *out);
    
        // Assumes there is enough space in _out_.
        reader *read(char *out, s64 n);
        reader *read(array<char> *out, s64 n);
    
        // Assumes there is enough space in _out_.
        // _delim_ is not included in the string.
        reader *read_until(char *out, char32_t delim);
    
        // _delim_ is not included in the string.
        reader *read_until(array<char> *out, char32_t delim);
    
        // Assumes there is enough space in _out_.
        // The delim is not included in the string.
        reader *read_until(char *out, const string &delims);
    
        // The delim is not included in the string.
        reader *read_until(array<char> *out, const string &delims);
    
        // Assumes there is enough space in _out_.
        // Doesn't put the terminating byte/s in the buffer.
        reader *read_while(char *out, char32_t eat);
    
        // Doesn't put the terminating byte/s in the buffer.
        reader *read_while(array<char> *out, char32_t eat);
    
        // Assumes there is enough space in _out_.
        // Doesn't put the terminating byte/s in the buffer.
        reader *read_while(char *out, const string &eats);
    
        // Doesn't put the terminating byte/s in the buffer.
        reader *read_while(array<char> *out, const string &eats);
    
        // Reads bytes until a newline character and puts them in _str_.
        // '\n' is not included in the string.
        reader *read_line(string *str);
    
        // Ignore available characters, read until a newline character and don't return the result
        reader *ignore();
    
        // Parse an integer
        // You can supply a custom base the integer is encoded in.
        // base 0 means this function tries to automatically determine the base by looking for a prefix:
        //     0x - hex, 0 - oct, otherwise - decimal
        //
        // If the parsing fails:
        // - the integer is outside range:               the value returned is the min/max value for that integer type
        // - the buffer doesn't contain a valid integer: the value returned is '0'
        // In both cases the _LastParseFailed_ flag is set to true (the flag gets reset before any parse function)
        //
        // Note:
        // If T is unsigned, but the buffer contains a '-', the value returned is underflowed
        //
        // @Locale This doesn't parse commas
        template <typename T>
        enable_if_t<is_integral_v<T>> read(T *value, s32 base = 0) {
            auto [parsed, success] = parse_int<T>(base);
            LastParseFailed = !success;
            *value = parsed;
        }
    
        // Parse a bool
        // Valid strings are: "0" "1" "true" "false" (ignoring case)
        //
        // @Bug There's an edge case in which this doesn't work. We have to extend the API a whole bunch in order to make
        // parsing in general easier and fix this. The bug is that sometimes the buffer might get cut of (e.g. "..tru" and
        // the next chunk would contain "e" but we would fail parsing because we check if the whole word is available
        // in the current chunk).
        //
        void read(bool *value);
    
        // Parse a float
        // If the parsing fails the _LastParseFailed_ flag is set to true (gets reset before any parsing operation)
        void reader::read(f32 *value) {
            if (!value) return;
            auto [parsed, success] = parse_float();
            LastParseFailed = !success;
            *value = (f32) parsed;
        }
    
        // Parse a float
        // If the parsing fails the _LastParseFailed_ flag is set to true (gets reset before any parsing operation)
        void reader::read(f64 *value) {
            if (!value) return;
            auto [parsed, success] = parse_float();
            LastParseFailed = !success;
            *value = parsed;
        }
    
        //
        // If the parsing fails the _LastParseFailed_ flag is set to true (gets reset before any parsing operation) and the guid is set to all 0
        void reader::read(guid *value) {
            if (!value) return;
            auto [parsed, success] = parse_guid();
            LastParseFailed = !success;
            *value = parsed;
        }
    
        template <typename T>
        enable_if_t<!is_arithmetic_v<T> && !is_same_v<T, string>> read(T *value) {
            LastParseFailed = !deserialize(value, this);
        }
    
        void read_byte(char *value, bool noSkipWhitespaceSingleTime = false) {
            if (!test_state_and_skip_ws(noSkipWhitespaceSingleTime)) {
                LastParseFailed = true;
                *value = eof;
                return;
            }
            *value = bump_byte();
            if (*value == eof) {
                LastParseFailed = true;
                EOF = true;
            }
        }
    
        bool test_state_and_skip_ws(bool noSkipSingleTime = false);
        */
    };
    
    // Standard input. Normally thread safe, optionally not (set LockMutex flag to false)
    inline console_reader cin;
    
}  // namespace io

LSTD_END_NAMESPACE
