#include "string.h"

#include <algorithm>

GU_BEGIN_NAMESPACE

size_t cstyle_strlen(const char *str) {
    size_t length = 0;
    while (*str++) length++;
    return length;
}

string::string(const char *str, size_t size) {
    BytesUsed = size;
    if (BytesUsed > SMALL_STRING_BUFFER_SIZE) {
        Data = New_And_Set_Allocator<char>(BytesUsed, Allocator);
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

string::string(const char *str) : string(str, str ? cstyle_strlen(str) : 0) {}

string::string(const string &other) {
    BytesUsed = other.BytesUsed;
    Length = other.Length;
    Allocator = other.Allocator;

    if (BytesUsed > SMALL_STRING_BUFFER_SIZE) {
        Data = New_And_Set_Allocator<char>(BytesUsed, Allocator);
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

string::String_Iterator_Mut string::begin() { return String_Iterator_Mut(*this, 0); }
string::String_Iterator_Mut string::end() { return String_Iterator_Mut(*this, string::NPOS); }
string::String_Iterator_Const string::begin() const { return String_Iterator_Const(*this, 0); }
string::String_Iterator_Const string::end() const { return String_Iterator_Const(*this, string::NPOS); }

Code_Point_Ref &Code_Point_Ref::operator=(char32_t other) {
    Parent.set(Index, other);
    CodePoint = other;
    return *this;
}

Code_Point_Ref string::operator[](s64 index) { return Code_Point_Ref(*this, get(index), index); }
const char32_t string::operator[](s64 index) const { return get(index); }

string string::operator()(s64 begin, s64 end) const { return substring(begin, end); }

void string::reserve(size_t size) {
    if (Data == _StackData) {
        // Return if there is enough space
        if (size <= string::SMALL_STRING_BUFFER_SIZE) return;

        // If we are small but we need more size, it's time to convert
        // to a dynamically allocated memory.
        Data = New_And_Set_Allocator<char>(size, Allocator);
        CopyMemory(Data, _StackData, BytesUsed);
        _Reserved = size;
    } else {
        // Return if there is enough space
        if (size <= _Reserved) return;

        Data = Resize_And_Set_Allocator(Data, _Reserved, size, Allocator);
        _Reserved = size;
    }
}

char32_t string::get(s64 index) const { return decode_code_point(_get_pointer_to_index(index)); }

void string::set(s64 index, char32_t codePoint) {
    assert(index < (s64) Length);

    char *target = _get_pointer_to_index(index);

    size_t sizeAtTarget = get_size_of_code_point(target);
    assert(sizeAtTarget);

    size_t codePointSize = get_size_of_code_point(codePoint);

    s32 difference = (s32) sizeAtTarget - (s32) codePointSize;
    if (difference < 0) {
        // If we get here, the size of the codepoint we want to encode
        // is larger than the code point already there, so we need to move
        // the data to make enough space.
        difference = -difference;

        size_t oldBytes = BytesUsed;

        reserve(BytesUsed + difference);
        // We need to recalculate target, because the reserve call above
        // might have moved the Data to a new memory location.
        target = _get_pointer_to_index(index);

        MoveMemory(target + codePointSize, target + sizeAtTarget, (Data + oldBytes) - target - (sizeAtTarget - 1));
        BytesUsed += difference;
    } else if (difference > 0) {
        MoveMemory(target + codePointSize, target + sizeAtTarget, (Data + BytesUsed) - target - (sizeAtTarget - 1));
        BytesUsed -= difference;
    }

    encode_code_point(target, codePoint);
}

// If negative, converts index to a positive number
// depending on the _Length_ of the string.
size_t string::_translate_index(s64 index) const {
    if (index < 0) {
        s64 actual = Length + index;
        assert(actual >= 0);
        return (size_t) actual;
    }
    return (size_t) index;
}

const string string::substring(s64 begin, s64 end) const {
    size_t beginIndex = _translate_index(begin);
    size_t endIndex = _translate_index(end);

    assert(beginIndex < Length);
    assert(endIndex <= Length);

    assert(endIndex > beginIndex);

    char *beginPtr = Data;
    for (size_t i = 0; i < beginIndex; i++) beginPtr += get_size_of_code_point(beginPtr);
    char *endPtr = beginPtr;
    for (size_t i = beginIndex; i < endIndex; i++) endPtr += get_size_of_code_point(endPtr);

    string result;
    result.Data = beginPtr;
    result.BytesUsed = (uptr_t)(endPtr - beginPtr);
    result.Length = endIndex - beginIndex;
    return result;
}

char *string::_get_pointer_to_index(s64 index) const {
    size_t actualIndex = _translate_index(index);
    assert(actualIndex < Length);

    char *s = Data;
    for (size_t i = 0; i < actualIndex; i++) s += get_size_of_code_point(s);
    return s;
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

void string::append_cstring(const char *other) { append_pointer_and_size(other, cstyle_strlen(other)); }

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

b32 string::compare(const string &other) const {
    if (Length < other.Length) {
        return -1;
    } else if (Length > other.Length) {
        return 1;
    }

    for (size_t i = 0; i < Length; i++) {
        if (get(i) < other[i]) {
            return -1;
        } else if (get(i) > other[i]) {
            return 1;
        }
    }
    // Both strings are equal
    return 0;
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

b32 string::begins_with(const string &other) const { return CompareMemory(Data, other.Data, other.BytesUsed) == 0; }

b32 string::ends_with(const string &other) const {
    return CompareMemory(Data + BytesUsed - other.BytesUsed, other.Data, other.BytesUsed) == 0;
}

const string string::trim() const { return trim_start().trim_end(); }

const string string::trim_start() const {
    string result;
    result.Data = Data;
    result.BytesUsed = BytesUsed;
    result.Length = Length;

    for (size_t i = 0; i < Length; i++) {
        char32_t ch = get(i);
        if (!is_space(ch)) break;

        size_t codePointSize = get_size_of_code_point(ch);
        result.Data += codePointSize;
        result.BytesUsed -= codePointSize;
        result.Length--;
    }
    return result;
}

const string string::trim_end() const {
    string result;
    result.Data = Data;
    result.BytesUsed = BytesUsed;
    result.Length = Length;

    for (size_t i = 1; i <= Length; i++) {
        char32_t ch = get(-(s64)(i));
        if (!is_space(ch)) break;

        size_t codePointSize = get_size_of_code_point(ch);
        result.BytesUsed -= codePointSize;
        result.Length--;
    }
    return result;
}

char32_t to_upper(char32_t cp) {
    if (((0x0061 <= cp) && (0x007a >= cp)) || ((0x00e0 <= cp) && (0x00f6 >= cp)) ||
        ((0x00f8 <= cp) && (0x00fe >= cp)) || ((0x03b1 <= cp) && (0x03c1 >= cp)) ||
        ((0x03c3 <= cp) && (0x03cb >= cp))) {
        return cp - 32;
    }
    if (((0x0100 <= cp) && (0x012f >= cp)) || ((0x0132 <= cp) && (0x0137 >= cp)) ||
        ((0x014a <= cp) && (0x0177 >= cp)) || ((0x0182 <= cp) && (0x0185 >= cp)) ||
        ((0x01a0 <= cp) && (0x01a5 >= cp)) || ((0x01de <= cp) && (0x01ef >= cp)) ||
        ((0x01f8 <= cp) && (0x021f >= cp)) || ((0x0222 <= cp) && (0x0233 >= cp)) ||
        ((0x0246 <= cp) && (0x024f >= cp)) || ((0x03d8 <= cp) && (0x03ef >= cp))) {
        return cp & ~0x1;
    }
    if (((0x0139 <= cp) && (0x0148 >= cp)) || ((0x0179 <= cp) && (0x017e >= cp)) ||
        ((0x01af <= cp) && (0x01b0 >= cp)) || ((0x01b3 <= cp) && (0x01b6 >= cp)) ||
        ((0x01cd <= cp) && (0x01dc >= cp))) {
        return (cp - 1) | 0x1;
    }
    if (cp == 0x00ff) return 0x0178;
    if (cp == 0x0180) return 0x0243;
    if (cp == 0x01dd) return 0x018e;
    if (cp == 0x019a) return 0x023d;
    if (cp == 0x019e) return 0x0220;
    if (cp == 0x0292) return 0x01b7;
    if (cp == 0x01c6) return 0x01c4;
    if (cp == 0x01c9) return 0x01c7;
    if (cp == 0x01cc) return 0x01ca;
    if (cp == 0x01f3) return 0x01f1;
    if (cp == 0x01bf) return 0x01f7;
    if (cp == 0x0188) return 0x0187;
    if (cp == 0x018c) return 0x018b;
    if (cp == 0x0192) return 0x0191;
    if (cp == 0x0199) return 0x0198;
    if (cp == 0x01a8) return 0x01a7;
    if (cp == 0x01ad) return 0x01ac;
    if (cp == 0x01b0) return 0x01af;
    if (cp == 0x01b9) return 0x01b8;
    if (cp == 0x01bd) return 0x01bc;
    if (cp == 0x01f5) return 0x01f4;
    if (cp == 0x023c) return 0x023b;
    if (cp == 0x0242) return 0x0241;
    if (cp == 0x037b) return 0x03fd;
    if (cp == 0x037c) return 0x03fe;
    if (cp == 0x037d) return 0x03ff;
    if (cp == 0x03f3) return 0x037f;
    if (cp == 0x03ac) return 0x0386;
    if (cp == 0x03ad) return 0x0388;
    if (cp == 0x03ae) return 0x0389;
    if (cp == 0x03af) return 0x038a;
    if (cp == 0x03cc) return 0x038c;
    if (cp == 0x03cd) return 0x038e;
    if (cp == 0x03ce) return 0x038f;
    if (cp == 0x0371) return 0x0370;
    if (cp == 0x0373) return 0x0372;
    if (cp == 0x0377) return 0x0376;
    if (cp == 0x03d1) return 0x03f4;
    if (cp == 0x03d7) return 0x03cf;
    if (cp == 0x03f2) return 0x03f9;
    if (cp == 0x03f8) return 0x03f7;
    if (cp == 0x03fb) return 0x03fa;
    // No upper case!
    return cp;
}

char32_t to_lower(char32_t cp) {
    if (((0x0041 <= cp) && (0x005a >= cp)) || ((0x00c0 <= cp) && (0x00d6 >= cp)) ||
        ((0x00d8 <= cp) && (0x00de >= cp)) || ((0x0391 <= cp) && (0x03a1 >= cp)) ||
        ((0x03a3 <= cp) && (0x03ab >= cp))) {
        return cp + 32;
    }
    if (((0x0100 <= cp) && (0x012f >= cp)) || ((0x0132 <= cp) && (0x0137 >= cp)) ||
        ((0x014a <= cp) && (0x0177 >= cp)) || ((0x0182 <= cp) && (0x0185 >= cp)) ||
        ((0x01a0 <= cp) && (0x01a5 >= cp)) || ((0x01de <= cp) && (0x01ef >= cp)) ||
        ((0x01f8 <= cp) && (0x021f >= cp)) || ((0x0222 <= cp) && (0x0233 >= cp)) ||
        ((0x0246 <= cp) && (0x024f >= cp)) || ((0x03d8 <= cp) && (0x03ef >= cp))) {
        return cp | 0x1;
    }
    if (((0x0139 <= cp) && (0x0148 >= cp)) || ((0x0179 <= cp) && (0x017e >= cp)) ||
        ((0x01af <= cp) && (0x01b0 >= cp)) || ((0x01b3 <= cp) && (0x01b6 >= cp)) ||
        ((0x01cd <= cp) && (0x01dc >= cp))) {
        return (cp + 1) & ~0x1;
    }
    if (cp == 0x0178) return 0x00ff;
    if (cp == 0x0178) return 0x00ff;
    if (cp == 0x0243) return 0x0180;
    if (cp == 0x018e) return 0x01dd;
    if (cp == 0x023d) return 0x019a;
    if (cp == 0x0220) return 0x019e;
    if (cp == 0x01b7) return 0x0292;
    if (cp == 0x01c4) return 0x01c6;
    if (cp == 0x01c7) return 0x01c9;
    if (cp == 0x01ca) return 0x01cc;
    if (cp == 0x01f1) return 0x01f3;
    if (cp == 0x01f7) return 0x01bf;
    if (cp == 0x0187) return 0x0188;
    if (cp == 0x018b) return 0x018c;
    if (cp == 0x0191) return 0x0192;
    if (cp == 0x0198) return 0x0199;
    if (cp == 0x01a7) return 0x01a8;
    if (cp == 0x01ac) return 0x01ad;
    if (cp == 0x01af) return 0x01b0;
    if (cp == 0x01b8) return 0x01b9;
    if (cp == 0x01bc) return 0x01bd;
    if (cp == 0x01f4) return 0x01f5;
    if (cp == 0x023b) return 0x023c;
    if (cp == 0x0241) return 0x0242;
    if (cp == 0x03fd) return 0x037b;
    if (cp == 0x03fe) return 0x037c;
    if (cp == 0x03ff) return 0x037d;
    if (cp == 0x037f) return 0x03f3;
    if (cp == 0x0386) return 0x03ac;
    if (cp == 0x0388) return 0x03ad;
    if (cp == 0x0389) return 0x03ae;
    if (cp == 0x038a) return 0x03af;
    if (cp == 0x038c) return 0x03cc;
    if (cp == 0x038e) return 0x03cd;
    if (cp == 0x038f) return 0x03ce;
    if (cp == 0x0370) return 0x0371;
    if (cp == 0x0372) return 0x0373;
    if (cp == 0x0376) return 0x0377;
    if (cp == 0x03f4) return 0x03d1;
    if (cp == 0x03cf) return 0x03d7;
    if (cp == 0x03f9) return 0x03f2;
    if (cp == 0x03f7) return 0x03f8;
    if (cp == 0x03fa) return 0x03fb;
    // No lower case!
    return cp;
}

size_t get_size_of_code_point(const char *str) {
    if (!str) return 0;
    if ((*str & 0xc0) == 0x80) return 0;

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
