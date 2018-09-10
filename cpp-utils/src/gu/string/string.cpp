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
    CountBytes = size;
    if (CountBytes > SMALL_STRING_BUFFER_SIZE) {
        // This ugly cast is worth it because it provides safety to the user !!!
        // I will probably change my mind on that...
        Data = New<char>(CountBytes, Allocator);

        if (!str) ZeroMemory(Data, CountBytes);
        if (str && CountBytes) CopyMemory(Data, str, CountBytes);

        _Reserved = CountBytes;
    } else {
        // If we are here, _Data_ points to _StackData_!
        if (str && CountBytes) CopyMemory(Data, str, CountBytes);
    }
}

string::string(string const &other) {
    Allocator = other.Allocator;
    CountBytes = other.CountBytes;

    if (other.Data == other._StackData) {
        // We are dealing with a short string.
        CopyMemory(_StackData, other._StackData, CountBytes);

        Data = _StackData;
        _Reserved = 0;
    } else {
        if (other.Data && CountBytes) {
            Data = New<char>(CountBytes, Allocator);
            CopyMemory(Data, other.Data, CountBytes);
        }
        _Reserved = CountBytes;
    }
}

string::string(string &&other) { *this = std::move(other); }

string &string::operator=(string const &other) {
    release(*this);
    Allocator = other.Allocator;
    CountBytes = other.CountBytes;

    if (other.Data == other._StackData) {
        // We are dealing with a short string.
        CopyMemory(_StackData, other._StackData, CountBytes);

        Data = _StackData;
        _Reserved = 0;
    } else {
        if (other.Data && CountBytes) {
            Data = New<char>(CountBytes, Allocator);
            CopyMemory(Data, other.Data, CountBytes);
        }
        _Reserved = CountBytes;
    }
}

string &string::operator=(string &&other) {
    if (this != &other) {
        release(*this);

        Allocator = other.Allocator;
        CountBytes = other.CountBytes;

        if (other.Data == other._StackData) {
            CopyMemory(_StackData, other._StackData, CountBytes);
            Data = _StackData;

            ZeroMemory(other._StackData, CountBytes);
        } else {
            if (other.Data && other.CountBytes) {
                Data = New<char>(CountBytes, Allocator);
                CopyMemory(Data, other.Data, CountBytes);
            }
            _Reserved = CountBytes;
        }

        other.Data = null;
        other.CountBytes = 0;
    }
    return *this;
}

string::~string() { release(*this); }

void release(string &str) {
    if (str.Data != str._StackData && str.Data) {
        Delete(str.Data, str._Reserved, str.Allocator);
        str.Data = null;

        str._Reserved = 0;
        str.CountBytes = 0;
    }
}

const char32_t string::operator[](size_t index) const {
	char *s = Data;

	for (size_t i = 0; i < index; i++) {
		s32 eatCodePoint;
		s = (char *) utf8codepoint(s, &eatCodePoint);
	}
	s32 codePoint;
	utf8codepoint(s, &codePoint);
	return (char32_t) codePoint;
}

size_t length(string const &str) {
    size_t result = 0;

    const byte *s = (const byte *) str.Data;
    for (size_t i = 0; i < str.CountBytes; i++, result++) {
        if (0xf0 == (0xf8 & *s)) {
            s += 4;
        } else if (0xe0 == (0xf0 & *s)) {
            s += 3;
        } else if (0xc0 == (0xe0 & *s)) {
            s += 2;
        } else {
            s += 1;
        }
    }
    return result;
}

void reserve(string &str, size_t size) {
    if (str.Data == str._StackData && size > string::SMALL_STRING_BUFFER_SIZE) {
        // If we are small but we need more size, it's time to convert
        // to a dynamically allocated memory.
        str.Data = New<char>(size, str.Allocator);
        str._Reserved = size;
    } else {
        // Return if there is enough space
        if (size <= str._Reserved) return;

        void *newData =
            str.Allocator.Function(Allocator_Mode::RESIZE, str.Allocator.Data, size, str.Data, str._Reserved, 0);
        str.Data = (char *const) newData;
        str._Reserved = size;
    }
}

b32 equal(string const &str, string const &other) {
    if (str.CountBytes != other.CountBytes) return false;

    byte *s1 = (byte *) str.Data;
    byte *s2 = (byte *) other.Data;

    for (size_t i = 0; i < str.CountBytes; i++, s1++, s2++) {
        if (*s1 != *s2) return false;
    }
    return true;
}

void append(string &str, string const &other) {
    size_t neededCapacity = str.CountBytes + other.CountBytes;
    reserve(str, neededCapacity);

    CopyMemory(str.Data + str.CountBytes, other.Data, other.CountBytes);

    str.CountBytes = neededCapacity;
}

void append_cstring(string &str, const char *other) { append_pointer_and_size(str, other, cstyle_strlen(other)); }

void append_pointer_and_size(string &str, const char *data, size_t size) {
    size_t neededCapacity = str.CountBytes + size;
    reserve(str, neededCapacity);

    CopyMemory(str.Data + str.CountBytes, data, size);

    str.CountBytes = neededCapacity;
}

GU_END_NAMESPACE
