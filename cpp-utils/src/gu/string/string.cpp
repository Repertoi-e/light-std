#include "string.h"

GU_BEGIN_NAMESPACE

size_t cstyle_strlen(const char *str) {
    size_t length = 0;
    while ('\0' != str[length]) {
        length++;
    }
    return length;
}

string::string(const char *str) : string(str, str ? cstyle_strlen(str) : 0) {}

string::string(const char *str, size_t size) {
    BytesUsed = size;
    if (BytesUsed > SMALL_STRING_BUFFER_SIZE) {
        // This ugly cast is worth it because it provides safety to the user !!!
        // I will probably change my mind on that...
        Data = New<char>(BytesUsed, Allocator);

        if (!str) ZeroMemory(Data, BytesUsed);
        if (str && BytesUsed) CopyMemory(Data, str, BytesUsed);

        _Reserved = BytesUsed;
    } else {
        // If we are here, _Data_ points to _StackData_!
        if (str && BytesUsed) CopyMemory(Data, str, BytesUsed);
    }

    const char *end = str + size;
    while (str < end) {
        str += get_size_of_code_point(str);
        Length++;
    }
}

string::string(string const &other) {
    BytesUsed = other.BytesUsed;
    Length = other.Length;

    if (other.Data == other._StackData) {
        // We are dealing with a short string.
        CopyMemory(_StackData, other._StackData, BytesUsed);

        Data = _StackData;
        _Reserved = 0;
    } else {
        if (other.Data && BytesUsed) {
            Data = New<char>(BytesUsed, Allocator);
            CopyMemory(Data, other.Data, BytesUsed);
        }
        _Reserved = BytesUsed;
    }
}

string::string(string &&other) { *this = std::move(other); }

string &string::operator=(string const &other) {
    release(*this);
    BytesUsed = other.BytesUsed;
    Length = other.Length;

    if (other.Data == other._StackData) {
        // We are dealing with a short string.
        CopyMemory(_StackData, other._StackData, BytesUsed);

        Data = _StackData;
        _Reserved = 0;
    } else {
        if (other.Data && BytesUsed) {
            Data = New<char>(BytesUsed, Allocator);
            CopyMemory(Data, other.Data, BytesUsed);
        }
        _Reserved = BytesUsed;
    }
    return *this;
}

string &string::operator=(string &&other) {
    if (this != &other) {
        release(*this);

        Allocator = other.Allocator;
        BytesUsed = other.BytesUsed;
        Length = other.Length;

        if (other.Data == other._StackData) {
            CopyMemory(_StackData, other._StackData, BytesUsed);
            Data = _StackData;

            ZeroMemory(other._StackData, BytesUsed);
        } else {
            if (other.Data && other.BytesUsed) {
                Data = New<char>(BytesUsed, Allocator);
                CopyMemory(Data, other.Data, BytesUsed);
            }
            _Reserved = BytesUsed;
        }

        other.Data = null;
        other.BytesUsed = 0;
    }
    return *this;
}

string::~string() { release(*this); }

void release(string &str) {
    if (str.Data != str._StackData && str.Data) {
        Delete(str.Data, str._Reserved, str.Allocator);
        str.Data = null;

        str._Reserved = 0;
        str.BytesUsed = 0;
        str.Length = 0;
    }
}

Code_Point_Ref &Code_Point_Ref::operator=(char32_t other) {
    set(Parent, Index, other);
    CodePoint = other;
    return *this;
}

Code_Point_Ref string::operator[](size_t index) { return Code_Point_Ref(*this, get(*this, index), index); }

const char32_t string::operator[](size_t index) const { return get(*this, index); }

void reserve(string &str, size_t size) {
    if (str.Data == str._StackData) {
        // Return if there is enough space
        if (size <= string::SMALL_STRING_BUFFER_SIZE) return;

        // If we are small but we need more size, it's time to convert
        // to a dynamically allocated memory.
        str.Data = New<char>(size, str.Allocator);
        CopyMemory(str.Data, str._StackData, str.BytesUsed);
        str._Reserved = size;
    } else {
        // Return if there is enough space
        if (size <= str._Reserved) return;

        str.Data = Resize(str.Data, str._Reserved, size, str.Allocator);
        str._Reserved = size;
    }
}

char32_t get(string const &str, size_t index) {
    const char *s = str.Data;
    for (size_t i = 0; i < index; i++) {
        s += get_size_of_code_point(s);
    }
    return decode_code_point(s);
}

void set(string &str, size_t index, char32_t codePoint) {
    assert(index < str.Length);

    char *target = get_pointer_to_index(str, index);

    size_t sizeAtTarget = get_size_of_code_point(target);
    assert(sizeAtTarget);

    s32 difference = (s32) sizeAtTarget - (s32) get_size_of_code_point(codePoint);
    if (difference < 0) {
        // If we get here, the size of the codepoint we want to encode
        // is larger than the code point already there, so we need to move
        // the data to make enough space.
        difference = -difference;

        reserve(str, str.BytesUsed + difference);
        // We need to recalculate target, because the reserve call above
        // might have moved the Data to a new memory location.
        target = get_pointer_to_index(str, index);

        MoveMemory(target + sizeAtTarget + difference, target + sizeAtTarget, difference);
        str.BytesUsed += difference;
    } else {
        MoveMemory(target + sizeAtTarget - difference, target + sizeAtTarget, difference);
        str.BytesUsed -= difference;
    }

    encode_code_point(target, codePoint);
}

char *get_pointer_to_index(string &str, size_t index) {
    char *s = str.Data;
    for (size_t i = 0; i < index; i++) s += get_size_of_code_point(s);
    return s;
}

b32 compare(string const &str, string const &other) {
    if (str.Length < other.Length) {
        return -1;
    } else if (str.Length > other.Length) {
        return 1;
    }

    for (size_t i = 0; i < str.Length; i++) {
        if (str[i] < other[i]) {
            return -1;
        } else if (str[i] > other[i]) {
            return 1;
        }
    }
    // Both strings are equal
    return 0;
}

void append(string &str, string const &other) {
    size_t neededCapacity = str.BytesUsed + other.BytesUsed;
    reserve(str, neededCapacity);

    CopyMemory(str.Data + str.BytesUsed, other.Data, other.BytesUsed);

    str.BytesUsed += other.BytesUsed;
    str.Length += other.Length;
}

void append(string &str, char32_t codePoint) {
    size_t codePointSize = get_size_of_code_point(codePoint);
    size_t neededCapacity = str.BytesUsed + codePointSize;
    reserve(str, neededCapacity);

    char *s = str.Data + str.BytesUsed;

    encode_code_point(s, codePoint);

    str.BytesUsed += codePointSize;
    str.Length++;
}

void append_cstring(string &str, const char *other) { append_pointer_and_size(str, other, cstyle_strlen(other)); }

void append_pointer_and_size(string &str, const char *data, size_t size) {
    size_t neededCapacity = str.BytesUsed + size;
    reserve(str, neededCapacity);

    CopyMemory(str.Data + str.BytesUsed, data, size);

    str.BytesUsed = neededCapacity;
    const char *end = data + size;
    while (data < end) {
        data += get_size_of_code_point(data);
        str.Length++;
    }
}

size_t get_size_of_code_point(const char *str) {
    if (!str) return 0;

    if (0xf0 == (0xf8 & str[0])) {
        return 4;
    } else if (0xe0 == (0xf0 & str[0])) {
        return 3;
    } else if (0xc0 == (0xe0 & str[0])) {
        return 2;
    } else {
        return 1;
    }
}

size_t get_size_of_code_point(char32_t codePoint) {
    if (((s32) 0xffffff80 & codePoint) == 0) {
        return 1;
    } else if (((s32) 0xfffff800 & codePoint) == 0) {
        return 2;
    } else if (((s32) 0xffff0000 & codePoint) == 0) {
        return 3;
    } else {
        return 4;
    }
}

void encode_code_point(char *str, char32_t codePoint) {
    assert(str);

    size_t size = get_size_of_code_point(codePoint);
    if (size == 1) {
        // 1-byte/7-bit ascii
        // (0b0xxxxxxx)
        str[0] = (char) codePoint;
    } else if (size == 2) {
        // 2-byte/11-bit utf-8 code point
        // (0b110xxxxx 0b10xxxxxx)
        str[0] = 0xc0 | (char) (codePoint >> 6);
        str[1] = 0x80 | (char) (codePoint & 0x3f);
    } else if (size == 3) {
        // 3-byte/16-bit utf-8 code point
        // (0b1110xxxx 0b10xxxxxx 0b10xxxxxx)
        str[0] = 0xe0 | (char) (codePoint >> 12);
        str[1] = 0x80 | (char) ((codePoint >> 6) & 0x3f);
        str[2] = 0x80 | (char) (codePoint & 0x3f);
    } else {
        // 4-byte/21-bit utf-8 code point
        // (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx)
        str[0] = 0xf0 | (char) (codePoint >> 18);
        str[1] = 0x80 | (char) ((codePoint >> 12) & 0x3f);
        str[2] = 0x80 | (char) ((codePoint >> 6) & 0x3f);
        str[3] = 0x80 | (char) (codePoint & 0x3f);
    }
}

char32_t decode_code_point(const char *str) {
    assert(str);

    if (0xf0 == (0xf8 & str[0])) {
        // 4 byte utf-8 code point
        return ((0x07 & str[0]) << 18) | ((0x3f & str[1]) << 12) | ((0x3f & str[2]) << 6) | (0x3f & str[3]);
    } else if (0xe0 == (0xf0 & str[0])) {
        // 3 byte utf-8 code point
        return ((0x0f & str[0]) << 12) | ((0x3f & str[1]) << 6) | (0x3f & str[2]);
    } else if (0xc0 == (0xe0 & str[0])) {
        // 2 byte utf-8 code point
        return ((0x1f & str[0]) << 6) | (0x3f & str[1]);
    } else {
        // 1 byte utf-8 code point
        return str[0];
    }
}

GU_END_NAMESPACE
