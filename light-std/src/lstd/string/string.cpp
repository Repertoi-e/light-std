#include "string.hpp"

LSTD_BEGIN_NAMESPACE

string::code_point &string::code_point::operator=(char32_t other) {
    Parent.set((s64) Index, other);
    return *this;
}

string::code_point::operator char32_t() const { return ((const string &) Parent).get((s64) Index); }

string::string(const memory_view &memory) {
    ByteLength = memory.ByteLength;
    if (ByteLength > SMALL_STRING_BUFFER_SIZE) {
        Data = new (&Allocator, ensure_allocator) byte[ByteLength];
        Reserved = ByteLength;
    }
    if (memory.Data && ByteLength) {
        copy_memory(Data, memory.Data, ByteLength);
        Length = utf8_strlen(Data, ByteLength);
    }
}

string::string(size_t size) { reserve(size); }

string::string(const string &other) {
    ByteLength = other.ByteLength;
    Length = other.Length;
    Allocator = other.Allocator;

    if (ByteLength > SMALL_STRING_BUFFER_SIZE) {
        Data = new (&Allocator, ensure_allocator) byte[ByteLength];
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

string string::removed_all(char32_t ch) const {
    if (Length == 0) return "";

    byte data[4] = {0};
    encode_code_point(data, ch);
    return removed_all(string_view(data, get_size_of_code_point(ch)));
}

string string::removed_all(const string_view &view) const {
    assert(view.Length);

    if (Length == 0) return "";

    string result;
    result.reserve(ByteLength);

    size_t p = 0, pos;
    while ((pos = find(view, p)) != npos) {
        ptr_t offset = get_pointer_to_code_point_at(Data, Length, p) - Data;
        result.append_pointer_and_size(Data + offset, pos - offset);
        p = pos + view.ByteLength;
        if (p == Length) break;
    }
    result.append_pointer_and_size(Data + p, ByteLength - p);

    return result;
}

string string::replaced_all(char32_t oldCh, char32_t newCh) const {
    if (Length == 0) return "";

    byte data1[4] = {0};
    encode_code_point(data1, oldCh);

    byte data2[4] = {0};
    encode_code_point(data2, newCh);

    return replaced_all(string_view(data1, get_size_of_code_point(oldCh)),
                        string_view(data2, get_size_of_code_point(newCh)));
}

string string::replaced_all(const string_view &oldView, const string_view &newView) const {
    assert(oldView.Length != 0);

    if (Length == 0) return "";

    string result;
    result.reserve(ByteLength);

    size_t p = 0;
    size_t pos;
    while ((pos = find(oldView, p)) != npos) {
        result.append_pointer_and_size(Data + p, pos - p);
        result.append(newView);
        p = pos + oldView.ByteLength;
        if (p == Length) break;
    }
    result.append_pointer_and_size(Data + p, ByteLength - p);

    return result;
}

wchar_t *string::to_utf16() const {
    auto *result = new (&Allocator, ensure_allocator) wchar_t[Length + 1];
    get_view().to_utf16(result);
    return result;
}

void string::from_utf16(const wchar_t *str, bool overwrite) {
    if (overwrite) *this = "";

    reserve(2 * cstring_strlen(str));
    for (; *str; ++str) {
        append((char32_t) *str);
    }
}

char32_t *string::to_utf32() const {
    auto *result = new (&Allocator, ensure_allocator) char32_t[Length + 1];
    get_view().to_utf32(result);
    return result;
}

void string::from_utf32(const char32_t *str, bool overwrite) {
    if (overwrite) *this = "";

    reserve(4 * cstring_strlen(str));
    for (; *str; ++str) {
        append(*str);
    }
}

string &string::operator=(const memory_view &memory) {
    release();

    string(memory).swap(*this);
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
        delete[] Data;
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
        if (size <= SMALL_STRING_BUFFER_SIZE) return;

        // If we are small but we need more size, it's time to convert
        // to a dynamically allocated memory.
        Data = new (&Allocator, ensure_allocator) byte[size];
        copy_memory(Data, StackData, ByteLength);
        Reserved = size;
    } else {
        // Return if there is enough space
        if (size <= Reserved) return;

        Data = resize(Data, size);
        Reserved = size;
    }
}

void string::set(s64 index, char32_t codePoint) {
    size_t cpSize = get_size_of_code_point(codePoint);
    auto *target = get_pointer_to_code_point_at(Data, Length, index);
    size_t cpSizeTarget = get_size_of_code_point(target);
    s64 diff = (s64) cpSize - (s64) cpSizeTarget;

    uptr_t offset = (uptr_t)(target - Data);
    reserve((size_t)((s64) ByteLength + diff));

    // We may have moved Data while reserving space!
    target = Data + offset;
    move_memory(Data + offset + cpSize, target + cpSizeTarget, ByteLength - (target - Data) - cpSizeTarget);
    encode_code_point(Data + offset, codePoint);

    ByteLength += diff;
}

void string::insert(s64 index, char32_t codePoint) {
    size_t cpSize = get_size_of_code_point(codePoint);
    reserve(ByteLength + cpSize);

    size_t translated = (size_t) index;
    if (index < 0) {
        translated = Length + index;
    }
    if (translated >= Length) {
        if (translated == Length) {
            append(codePoint);
            return;
        }
        assert(false && "Cannot add code point at specified index (out of range)");
    }

    auto *target = get_pointer_to_code_point_at(Data, Length, translated);
    uptr_t offset = (uptr_t)(target - Data);
    move_memory(Data + offset + cpSize, target, ByteLength - (target - Data));

    encode_code_point(Data + offset, codePoint);

    ByteLength += cpSize;
    ++Length;
}

void string::insert(s64 index, const memory_view &memory) {
    insert_pointer_and_size(index, memory.Data, memory.ByteLength);
}

void string::insert_pointer_and_size(s64 index, const byte *str, size_t size) {
    reserve(ByteLength + size);

    size_t translated = (size_t) index;
    if (index < 0) {
        translated = Length + index;
    }
    if (translated >= Length) {
        if (translated == Length) {
            append_pointer_and_size(str, size);
            return;
        }
        assert(false && "Cannot add code point at specified index (out of range)");
    }

    auto *target = get_pointer_to_code_point_at(Data, Length, translated);
    uptr_t offset = (uptr_t)(target - Data);
    move_memory(Data + offset + size, target, ByteLength - (target - Data));

    copy_memory(Data + offset, str, size);

    ByteLength += size;
    Length += utf8_strlen(str, size);
}

void string::remove(s64 index) {
    auto *target = get_pointer_to_code_point_at(Data, Length, index);

    size_t cpSize = get_size_of_code_point(target);
    --Length;

    uptr_t offset = (uptr_t)(target - Data);
    move_memory(Data + offset, target + cpSize, ByteLength - offset - cpSize);

    ByteLength -= cpSize;
}

void string::remove(s64 begin, s64 end) {
    auto *targetBegin = get_pointer_to_code_point_at(Data, Length, begin);
    const byte *targetEnd;
    if ((size_t) end >= Length) {
        targetEnd = Data + ByteLength;
    } else {
        targetEnd = get_pointer_to_code_point_at(Data, Length, end);
    }
    assert(targetEnd > targetBegin);

    size_t bytes = targetEnd - targetBegin;
    Length -= utf8_strlen(targetBegin, targetEnd - targetBegin);

    uptr_t offset = (uptr_t)(targetBegin - Data);
    move_memory(Data + offset, targetEnd, ByteLength - offset - bytes);

    ByteLength -= bytes;
}

void string::append(char32_t codePoint) {
    size_t cpSize = get_size_of_code_point(codePoint);
    reserve(ByteLength + cpSize);

    encode_code_point(Data + ByteLength, codePoint);

    ByteLength += cpSize;
    ++Length;
}

void string::append(const memory_view &memory) { append_pointer_and_size(memory.Data, memory.ByteLength); }

void string::append_pointer_and_size(const byte *data, size_t size) {
    size_t neededCapacity = ByteLength + size;
    reserve(neededCapacity);

    copy_memory(Data + ByteLength, data, size);

    ByteLength = neededCapacity;

    Length += utf8_strlen(data, size);
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

LSTD_END_NAMESPACE
