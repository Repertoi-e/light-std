#include "string.hpp"

#include <algorithm>

CPPU_BEGIN_NAMESPACE

string_view::string_view(const string &str) : Data(str.Data), ByteLength(str.ByteLength), Length(str.Length) {}

string::Code_Point &string::Code_Point::operator=(char32_t other) {
    Parent.set((s64) Index, other);
    return *this;
}

string::Code_Point::operator char32_t() const { return ((const string &) Parent).get((s64) Index); }

string::string(const byte *str) : string(str, str ? cstring_strlen(str) : 0) {}

string::string(const byte *str, size_t size) {
    ByteLength = size;
    if (ByteLength > SMALL_STRING_BUFFER_SIZE) {
        Data = New_and_ensure_allocator<byte>(ByteLength, Allocator);
        Reserved = ByteLength;
    }
    if (str && ByteLength) {
        copy_memory(Data, str, ByteLength);

        const byte *end = str + size;
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
        Data = New_and_ensure_allocator<byte>(ByteLength, Allocator);
        Reserved = ByteLength;
    }
    if (other.Data && ByteLength) {
        copy_memory(Data, other.Data, ByteLength);
    }
}

string::string(string &&other) { other.swap(*this); }

void string::swap(string &other) {
    if (Data != StackData && other.Data != other.StackData) {
        std::swap(Data, other.Data);
    } else {
        For(range(SMALL_STRING_BUFFER_SIZE)) {
            auto temp = StackData[it];
            StackData[it] = other.StackData[it];
            other.StackData[it] = temp;
        }

        bool isOtherSmall = other.Data == other.StackData;
        if (Data != StackData || !isOtherSmall) {
            if (Data == StackData) {
                auto temp = other.Data;
                other.Data = other.StackData;
                Data = temp;
            }
            if (isOtherSmall) {
                auto temp = Data;
                Data = StackData;
                other.Data = temp;
            }
        }
    }
    std::swap(Allocator, other.Allocator);

    std::swap(Reserved, other.Reserved);
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
    if (Data && Data != StackData && Reserved) {
        Delete(Data, Reserved, Allocator);
        Data = StackData;

        Reserved = 0;
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
    if (Data == StackData) {
        // Return if there is enough space
        if (size <= string::SMALL_STRING_BUFFER_SIZE) return;

        // If we are small but we need more size, it's time to convert
        // to a dynamically allocated memory.
        Data = New_and_ensure_allocator<byte>(size, Allocator);
        copy_memory(Data, StackData, ByteLength);
        Reserved = size;
    } else {
        // Return if there is enough space
        if (size <= Reserved) return;

        Data = Resize_and_ensure_allocator(Data, Reserved, size, Allocator);
        Reserved = size;
    }
}

void string::set(s64 index, char32_t codePoint) {
    const byte *target = get_pointer_to_code_point_at(Data, Length, index);

    uptr_t offset = (uptr_t)(target - Data);
    assert(offset < ByteLength);

    byte *at = Data + offset;

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

        move_memory(at + codePointSize, at + sizeAtTarget, ByteLength - (at - Data) - sizeAtTarget);
    } else if (difference > 0) {
        move_memory(at + codePointSize, at + sizeAtTarget, ByteLength - (at - Data) - sizeAtTarget);
    }
    ByteLength -= difference;

    encode_code_point(at, codePoint);
}

void string::append(const string &other) {
    size_t neededCapacity = ByteLength + other.ByteLength;
    reserve(neededCapacity);

    copy_memory(Data + ByteLength, other.Data, other.ByteLength);

    ByteLength += other.ByteLength;
    Length += other.Length;
}

void string::append(char32_t codePoint) {
    size_t codePointSize = get_size_of_code_point(codePoint);
    size_t neededCapacity = ByteLength + codePointSize;
    reserve(neededCapacity);

    byte *s = Data + ByteLength;

    encode_code_point(s, codePoint);

    ByteLength += codePointSize;
    Length++;
}

void string::append_cstring(const byte *other) { append_pointer_and_size(other, cstring_strlen(other)); }

void string::append_pointer_and_size(const byte *data, size_t size) {
    size_t neededCapacity = ByteLength + size;
    reserve(neededCapacity);

    copy_memory(Data + ByteLength, data, size);

    ByteLength = neededCapacity;

    const byte *end = data + size;
    while (data < end) {
        data += get_size_of_code_point(data);
        Length++;
    }
}

string string::repeated(size_t n) {
    string result = *this;
    result.reserve(n * ByteLength);
    For(range(1, n)) result.append(*this);
    return result;
}

string string::get_upper() const {
    string result = *this;
    For(range(Length)) result[it] = to_upper(result[it]);
    return result;
}

string string::get_lower() const {
    string result = *this;
    For(range(Length)) result[it] = to_lower(result[it]);
    return result;
}

CPPU_END_NAMESPACE
