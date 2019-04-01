#include "string.hpp"

#include "../intrinsics/intrin.hpp"

LSTD_BEGIN_NAMESPACE

string::code_point &string::code_point::operator=(char32_t other) {
    Parent.set((s64) Index, other);
    return *this;
}

string::code_point::operator char32_t() const { return ((const string &) Parent).get((s64) Index); }

string::string(const memory_view &memory) {
    ByteLength = memory.ByteLength;
    Data = stack_dynamic_memory<byte, 8>(memory.Data, memory.ByteLength);
    Length = utf8_strlen(Data.get(), ByteLength);
}

string::string(size_t size) { Data.reserve(size); }

void string::swap(string &other) {
    Data.swap(other.Data);
    std::swap(ByteLength, other.ByteLength);
    std::swap(Length, other.Length);
}

void string::remove_all(char32_t ch) {
    if (Length == 0) return;

    ptr_t offset = 0;
    For(range(0, Length)) {
        if (get(it - offset) == ch) {
            remove(it - offset++);
        }
    }
}

void string::remove_all(const string_view &view) {
    assert(view.Length);

    if (Length == 0) return;

    ptr_t offset = 0;
    For(range(0, Length)) {
        auto realIt = it - offset;

        auto progress = view.begin();
        for (auto search = begin() + realIt; progress != view.end(); ++search, ++progress) {
            if (*search != *progress) break;
        }
        if (progress == view.end()) {
            remove(realIt, realIt + view.Length);
            offset += view.Length;
        }
    }
}

void string::replace_all(char32_t oldCh, char32_t newCh) {
    if (Length == 0) return;
    For(range(0, Length)) {
        if (get(it) == oldCh) set(it, newCh);
    }
}

void string::replace_all(const string_view &oldView, const string_view &newView) {
    assert(oldView.Length != 0);

    if (Length == 0) return;

    ptr_t diff = (ptr_t) newView.Length - (ptr_t) oldView.Length;
    for (size_t it = 0; it < Length; ++it) {
        auto progress = oldView.begin();
        for (auto search = begin() + it; search != end() && progress != oldView.end(); ++search, ++progress) {
            if (*search != *progress) break;
        }
        if (progress == oldView.end()) {
            remove(it, it + oldView.Length);
            insert(it, newView);
            it += diff;
        }
    }
}

wchar_t *string::to_utf16() const {
    auto *result = new (&Data.Allocator, ensure_allocator) wchar_t[Length + 1];
    get_view().to_utf16(result);
    return result;
}

void string::from_utf16(const wchar_t *str, bool overwrite) {
    if (overwrite) *this = "";

    Data.reserve(2 * cstring_strlen(str));
    for (; *str; ++str) {
        append((char32_t) *str);
    }
}

char32_t *string::to_utf32() const {
    auto *result = new (&Data.Allocator, ensure_allocator) char32_t[Length + 1];
    get_view().to_utf32(result);
    return result;
}

void string::from_utf32(const char32_t *str, bool overwrite) {
    if (overwrite) *this = "";

    Data.reserve(4 * cstring_strlen(str));
    for (; *str; ++str) {
        append(*str);
    }
}

string &string::operator=(const memory_view &memory) {
    Data.release();

    ByteLength = memory.ByteLength;
    Data = stack_dynamic_memory<byte, 8>(memory.Data, memory.ByteLength);
    Length = utf8_strlen(Data.get(), ByteLength);
    return *this;
}

void string::release() {
    Data.release();
    clear();
}

string::iterator string::begin() { return iterator(*this, 0); }
string::iterator string::end() { return iterator(*this, Length); }
string::const_iterator string::begin() const { return const_iterator(*this, 0); }
string::const_iterator string::end() const { return const_iterator(*this, Length); }

string::code_point string::operator[](s64 index) { return get(index); }
char32_t string::operator[](s64 index) const { return get(index); }

string_view string::operator()(s64 begin, s64 end) const { return substring(begin, end); }

void string::set(s64 index, char32_t codePoint) {
    size_t cpSize = get_size_of_code_point(codePoint);

    auto *target = get_pointer_to_code_point_at(Data.get(), Length, index);
    size_t cpSizeTarget = get_size_of_code_point(target);
    
    s64 diff = (s64) cpSize - (s64) cpSizeTarget;

    uptr_t offset = (uptr_t)(target - Data.get());
    Data.reserve((size_t)((s64) ByteLength + diff));

    // We may have moved Data while reserving space!
    target = Data.get() + offset;
    move_memory(Data.get() + offset + cpSize, target + cpSizeTarget, ByteLength - (target - Data.get()) - cpSizeTarget);
    encode_code_point(Data.get() + offset, codePoint);

    ByteLength += diff;
}

void string::insert(s64 index, char32_t codePoint) {
    size_t cpSize = get_size_of_code_point(codePoint);
    Data.reserve(ByteLength + cpSize);

    auto *target = get_pointer_to_code_point_at(Data.get(), Length, index, true);
    uptr_t offset = (uptr_t)(target - Data.get());
    move_memory(Data.get() + offset + cpSize, target, ByteLength - (target - Data.get()));

    encode_code_point(Data.get() + offset, codePoint);

    ByteLength += cpSize;
    ++Length;
}

void string::insert(s64 index, const memory_view &memory) {
    insert_pointer_and_size(index, memory.Data, memory.ByteLength);
}

void string::insert_pointer_and_size(s64 index, const byte *str, size_t size) {
    Data.reserve(ByteLength + size);

    auto *target = get_pointer_to_code_point_at(Data.get(), Length, index, true);
    uptr_t offset = (uptr_t)(target - Data.get());
    move_memory(Data.get() + offset + size, target, ByteLength - (target - Data.get()));

    copy_memory(Data.get() + offset, str, size);

    ByteLength += size;
    Length += utf8_strlen(str, size);
}

void string::remove(s64 index) {
    auto *target = get_pointer_to_code_point_at(Data.get(), Length, index);

    size_t cpSize = get_size_of_code_point(target);
    --Length;

    uptr_t offset = (uptr_t)(target - Data.get());
    move_memory(Data.get() + offset, target + cpSize, ByteLength - offset - cpSize);

    ByteLength -= cpSize;
}

void string::remove(s64 begin, s64 end) {
    auto *targetBegin = get_pointer_to_code_point_at(Data.get(), Length, begin);
    auto *targetEnd = get_pointer_to_code_point_at(Data.get(), Length, end, true);;
    assert(targetEnd > targetBegin);

    size_t bytes = targetEnd - targetBegin;
    Length -= utf8_strlen(targetBegin, targetEnd - targetBegin);

    uptr_t offset = (uptr_t)(targetBegin - Data.get());
    move_memory(Data.get() + offset, targetEnd, ByteLength - offset - bytes);

    ByteLength -= bytes;
}

void string::append(char32_t codePoint) {
    size_t cpSize = get_size_of_code_point(codePoint);
    Data.reserve(ByteLength + cpSize);

    encode_code_point(Data.get() + ByteLength, codePoint);

    ByteLength += cpSize;
    ++Length;
}

void string::append(const memory_view &memory) { append_pointer_and_size(memory.Data, memory.ByteLength); }

void string::append_pointer_and_size(const byte *data, size_t size) {
    size_t neededCapacity = ByteLength + size;
    Data.reserve(neededCapacity);

    copy_memory(Data.get() + ByteLength, data, size);

    ByteLength = neededCapacity;

    Length += utf8_strlen(data, size);
}

string string::repeated(size_t n) {
    string result = *this;
    result.Data.reserve(n * ByteLength);
    For(range(1, n)) { result.append(*this); }
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
