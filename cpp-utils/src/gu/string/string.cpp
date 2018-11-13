#include "string.h"

#include "string_view.h"

#include <algorithm>

GU_BEGIN_NAMESPACE

string::string(const char *str) : string(str, str ? cstring_strlen(str) : 0) {}

string::string(const char *str, size_t size) {
    BytesUsed = size;
    if (BytesUsed > SMALL_STRING_BUFFER_SIZE) {
        Data = New_And_Ensure_Allocator<char>(BytesUsed, Allocator);
        _Reserved = BytesUsed;
    }
    if (str && BytesUsed) {
        CopyMemory(Data, str, BytesUsed);

        const char *end = str + size;
        while (str < end) {
            str += get_size_of_code_point(str);
            Length++;
        }
    }
}

string::string(const string_view &view) : string(view.Data, view.BytesUsed) {}

string::string(const string &other) {
    BytesUsed = other.BytesUsed;
    Length = other.Length;
    Allocator = other.Allocator;

    if (BytesUsed > SMALL_STRING_BUFFER_SIZE) {
        Data = New_And_Ensure_Allocator<char>(BytesUsed, Allocator);
        _Reserved = BytesUsed;
    }
    if (other.Data && BytesUsed) {
        CopyMemory(Data, other.Data, BytesUsed);
    }
}

string::string(string &&other) { other.swap(*this); }

void string::swap(string &other) {
    if (Data != _StackData && other.Data != other._StackData) {
        std::swap(Data, other.Data);
    } else {
        std::swap_ranges(_StackData, _StackData + SMALL_STRING_BUFFER_SIZE, other._StackData);
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
    std::swap(BytesUsed, other.BytesUsed);
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

string::Iterator_Mut string::begin() { return Iterator_Mut(*this, 0); }
string::Iterator_Mut string::end() { return Iterator_Mut(*this, npos); }
string::Iterator_Const string::begin() const { return Iterator_Const(*this, 0); }
string::Iterator_Const string::end() const { return Iterator_Const(*this, npos); }

string::Code_Point_Ref string::operator[](s64 index) { return Code_Point_Ref(*this, get(index), index); }
const char32_t string::operator[](s64 index) const { return get(index); }

string_view string::operator()(s64 begin, s64 end) const { return substring(begin, end); }

void string::reserve(size_t size) {
    if (Data == _StackData) {
        // Return if there is enough space
        if (size <= string::SMALL_STRING_BUFFER_SIZE) return;

        // If we are small but we need more size, it's time to convert
        // to a dynamically allocated memory.
        Data = New_And_Ensure_Allocator<char>(size, Allocator);
        CopyMemory(Data, _StackData, BytesUsed);
        _Reserved = size;
    } else {
        // Return if there is enough space
        if (size <= _Reserved) return;

        Data = Resize_And_Ensure_Allocator(Data, _Reserved, size, Allocator);
        _Reserved = size;
    }
}

void string::set(s64 index, char32_t codePoint) {
    assert(index < (s64) Length);

    char *target = (char *) string_view(*this)._get_pointer_to_index(index);

    size_t sizeAtTarget = get_size_of_code_point(target);
    assert(sizeAtTarget);

    size_t codePointSize = get_size_of_code_point(codePoint);

    s32 difference = (s32) sizeAtTarget - (s32) codePointSize;
    if (difference < 0) {
        // If we get here, the size of the codepoint we want to encode
        // is larger than the code point already there, so we need to move
        // the data to make enough space.
        reserve(BytesUsed - difference);
        // We need to recalculate target, because the reserve call above
        // might have moved the Data to a new memory location.
        target = (char *) string_view(*this)._get_pointer_to_index(index);

        MoveMemory(target + codePointSize, target + sizeAtTarget, BytesUsed - (target - Data) - sizeAtTarget);
    } else if (difference > 0) {
        MoveMemory(target + codePointSize, target + sizeAtTarget, BytesUsed - (target - Data) - sizeAtTarget);
    }
    BytesUsed -= difference;

    encode_code_point(target, codePoint);
}

void string::append(const string &other) {
    size_t neededCapacity = BytesUsed + other.BytesUsed;
    reserve(neededCapacity);

    CopyMemory(Data + BytesUsed, other.Data, other.BytesUsed);

    BytesUsed += other.BytesUsed;
    Length += other.Length;
}

void string::append(char32_t codePoint) {
    size_t codePointSize = get_size_of_code_point(codePoint);
    size_t neededCapacity = BytesUsed + codePointSize;
    reserve(neededCapacity);

    char *s = Data + BytesUsed;

    encode_code_point(s, codePoint);

    BytesUsed += codePointSize;
    Length++;
}

void string::append_cstring(const char *other) { append_pointer_and_size(other, cstring_strlen(other)); }

void string::append_pointer_and_size(const char *data, size_t size) {
    size_t neededCapacity = BytesUsed + size;
    reserve(neededCapacity);

    CopyMemory(Data + BytesUsed, data, size);

    BytesUsed = neededCapacity;

    const char *end = data + size;
    while (data < end) {
        data += get_size_of_code_point(data);
        Length++;
    }
}

void string::repeat(size_t n) {
    assert(n > 0);
    reserve(n * BytesUsed);
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

GU_END_NAMESPACE
