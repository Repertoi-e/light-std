#include "string.h"

#include "string_view.h"

#include <algorithm>

CPPU_BEGIN_NAMESPACE

string::Code_Point &string::Code_Point::operator=(char32_t other) {
    Parent.set((s64) Index, other);
    return *this;
}

string::Code_Point::operator char32_t() { return ((const string &) Parent).get((s64) Index); }

string::string(const char *str) : string(str, str ? cstring_strlen(str) : 0) {}

string::string(const char *str, size_t size) {
    ByteLength = size;
    if (ByteLength > SMALL_STRING_BUFFER_SIZE) {
        Data = New_And_Ensure_Allocator<char>(ByteLength, Allocator);
        _Reserved = ByteLength;
    }
    if (str && ByteLength) {
        CopyMemory(Data, str, ByteLength);

        const char *end = str + size;
        while (str < end) {
            str += get_size_of_code_point(str);
            Length++;
        }
    }
}

string::string(const string_view &view) : string(view.Data, view.ByteLength) {}

string::string(const string &other) {
    ByteLength = other.ByteLength;
    Length = other.Length;
    Allocator = other.Allocator;

    if (ByteLength > SMALL_STRING_BUFFER_SIZE) {
        Data = New_And_Ensure_Allocator<char>(ByteLength, Allocator);
        _Reserved = ByteLength;
    }
    if (other.Data && ByteLength) {
        CopyMemory(Data, other.Data, ByteLength);
    }
}

string::string(string &&other) { other.swap(*this); }

void string::swap(string &other) {
    if (Data != _StackData && other.Data != other._StackData) {
        std::swap(Data, other.Data);
    } else {
        for (size_t i = 0; i < SMALL_STRING_BUFFER_SIZE; i++) {
            auto temp = _StackData[i];
            _StackData[i] = other._StackData[i];
            other._StackData[i] = temp;
        }

        b32 isOtherSmall = other.Data == other._StackData;
        if (Data != _StackData || !isOtherSmall) {
            if (Data == _StackData) {
                auto temp = other.Data;
                other.Data = other._StackData;
                Data = temp;
            }
            if (isOtherSmall) {
                auto temp = Data;
                Data = _StackData;
                other.Data = temp;
            }
        }
    }
    std::swap(Allocator, other.Allocator);

    std::swap(_Reserved, other._Reserved);
    std::swap(ByteLength, other.ByteLength);
    std::swap(Length, other.Length);
}

string &string::operator=(const string_view &view) {
    release();

    string(view).swap(*this);
    return *this;
}

string &string::operator=(const string &other) {
    release();

    string(other).swap(*this);
    return *this;
}

string &string::operator=(string &&other) {
    release();

    string(std::move(other)).swap(*this);
    return *this;
}

string::~string() { release(); }

void string::release() {
    if (Data && Data != _StackData && _Reserved) {
        Delete(Data, _Reserved, Allocator);
        Data = _StackData;

        _Reserved = 0;
    }
    clear();
}

string::iterator string::begin() { return iterator(*this, 0); }
string::iterator string::end() { return iterator(*this, Length); }
string::const_iterator string::begin() const { return const_iterator(*this, 0); }
string::const_iterator string::end() const { return const_iterator(*this, Length); }

string::code_point string::operator[](s64 index) { return get(index); }
char32_t string::operator[](s64 index) const { return get(index); }

string_view string::operator()(s64 begin, s64 end) const { return substring(begin, end); }

void string::reserve(size_t size) {
    if (Data == _StackData) {
        // Return if there is enough space
        if (size <= string::SMALL_STRING_BUFFER_SIZE) return;

        // If we are small but we need more size, it's time to convert
        // to a dynamically allocated memory.
        Data = New_And_Ensure_Allocator<char>(size, Allocator);
        CopyMemory(Data, _StackData, ByteLength);
        _Reserved = size;
    } else {
        // Return if there is enough space
        if (size <= _Reserved) return;

        Data = Resize_And_Ensure_Allocator(Data, _Reserved, size, Allocator);
        _Reserved = size;
    }
}

void string::set(s64 index, char32_t codePoint) {
    const char *target = get_pointer_to_code_point_at(Data, Length, index);

    uptr_t offset = (uptr_t)(target - Data);
    assert(offset < ByteLength);

    char *at = Data + offset;

    size_t sizeAtTarget = get_size_of_code_point(at);
    assert(sizeAtTarget);

    size_t codePointSize = get_size_of_code_point(codePoint);

    s32 difference = (s32) sizeAtTarget - (s32) codePointSize;
    if (difference < 0) {
        // If we get here, the size of the codepoint we want to encode
        // is larger than the code point already there, so we need to move
        // the data to make enough space.
        reserve(ByteLength - difference);

        // We need to recalculate _at_, because the reserve call above
        // might have moved the Data to a new memory location.
        at = Data + offset;

        MoveMemory(at + codePointSize, at + sizeAtTarget, ByteLength - (at - Data) - sizeAtTarget);
    } else if (difference > 0) {
        MoveMemory(at + codePointSize, at + sizeAtTarget, ByteLength - (at - Data) - sizeAtTarget);
    }
    ByteLength -= difference;

    encode_code_point(at, codePoint);
}

void string::append(const string &other) {
    size_t neededCapacity = ByteLength + other.ByteLength;
    reserve(neededCapacity);

    CopyMemory(Data + ByteLength, other.Data, other.ByteLength);

    ByteLength += other.ByteLength;
    Length += other.Length;
}

void string::append(char32_t codePoint) {
    size_t codePointSize = get_size_of_code_point(codePoint);
    size_t neededCapacity = ByteLength + codePointSize;
    reserve(neededCapacity);

    char *s = Data + ByteLength;

    encode_code_point(s, codePoint);

    ByteLength += codePointSize;
    Length++;
}

void string::append_cstring(const char *other) { append_pointer_and_size(other, cstring_strlen(other)); }

void string::append_pointer_and_size(const char *data, size_t size) {
    size_t neededCapacity = ByteLength + size;
    reserve(neededCapacity);

    CopyMemory(Data + ByteLength, data, size);

    ByteLength = neededCapacity;

    const char *end = data + size;
    while (data < end) {
        data += get_size_of_code_point(data);
        Length++;
    }
}

void string::repeat(size_t n) {
    assert(n > 0);
    reserve(n * ByteLength);
    string pattern = *this;
    for (size_t i = 1; i < n; i++) {
        append(pattern);
    }
}

string string::get_upper() const {
    string result = *this;
    for (size_t i = 0; i < result.Length; i++) {
        result[i] = to_upper(result[i]);
    }
    return result;
}

string string::get_lower() const {
    string result = *this;
    for (size_t i = 0; i < result.Length; i++) {
        result[i] = to_lower(result[i]);
    }
    return result;
}

CPPU_END_NAMESPACE
