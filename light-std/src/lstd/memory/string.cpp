#include "string.h"

#include "../internal/context.h"

LSTD_BEGIN_NAMESPACE

string::code_point &string::code_point::operator=(char32_t other) {
    Parent->set(Index, other);
    return *this;
}

string::code_point::operator char32_t() const { return ((const string *) Parent)->get(Index); }

string::string(char32_t codePoint, s64 repeat) : string(get_size_of_cp(codePoint) * repeat) {
    s64 cpSize = get_size_of_cp(codePoint);

    auto *data = const_cast<char *>(Data);
    For(range(repeat)) {
        encode_cp(data, codePoint);
        data += cpSize;
        ++Length;
    }

    ByteLength = Length * cpSize;
}

string::string(const wchar_t *str) {
    reserve(2 * c_string_length(str));
    for (; *str; ++str) {
        append((char32_t) *str);
    }
}

string::string(const char32_t *str) {
    reserve(4 * c_string_length(str));
    for (; *str; ++str) {
        append(*str);
    }
}

string::string(s64 size) { reserve(size); }

void string::reserve(s64 target) {
    if (ByteLength + target < Reserved) return;

    target = max<s64>(ceil_pow_of_2(target + ByteLength + 1), 8);

    if (is_owner()) {
        Data = (const char *) allocator::reallocate(const_cast<char *>(Data), target);
    } else {
        auto *oldData = Data;
        Data = (const char *) Context.Alloc.allocate(target);
        encode_owner(Data, this);
        if (ByteLength) copy_memory(const_cast<char *>(Data), oldData, ByteLength);
    }
    Reserved = target;
}

void string::release() {
    if (is_owner()) {
        delete Data;
    }
    Data = null;
    Length = ByteLength = Reserved = 0;
}

string *string::set(s64 index, char32_t codePoint) {
    s64 cpSize = get_size_of_cp(codePoint);

    auto *target = get_cp_at_index(Data, Length, index);
    s64 cpSizeTarget = get_size_of_cp(target);

    s64 diff = cpSize - cpSizeTarget;

    // Calculate the offset and then reserve (which may move the memory!)
    u64 offset = (u64)(target - Data);
    reserve(abs(diff));

    // We may have moved Data while reserving space!
    target = Data + offset;
    copy_memory((char *) Data + offset + cpSize, target + cpSizeTarget, ByteLength - (target - Data) - cpSizeTarget);
    encode_cp((char *) Data + offset, codePoint);

    ByteLength += diff;

    return this;
}

string *string::insert(s64 index, char32_t codePoint) {
    s64 cpSize = get_size_of_cp(codePoint);
    reserve(cpSize);

    auto *target = get_cp_at_index(Data, Length, index, true);
    u64 offset = (u64)(target - Data);
    copy_memory((char *) Data + offset + cpSize, target, ByteLength - (target - Data));

    encode_cp((char *) Data + offset, codePoint);

    ByteLength += cpSize;
    ++Length;

    return this;
}

string *string::insert(s64 index, string str) { return insert_pointer_and_size(index, str.Data, str.ByteLength); }

string *string::insert_pointer_and_size(s64 index, const char *str, s64 size) {
    // assert(str);
    if (!str) return this;

    reserve(size);
    if (size == 0) return this;

    auto *target = get_cp_at_index(Data, Length, index, true);
    u64 offset = (u64)(target - Data);
    copy_memory((char *) Data + offset + size, target, ByteLength - (target - Data));

    copy_memory((char *) Data + offset, str, size);

    ByteLength += size;
    Length += utf8_length(str, size);

    return this;
}

string *string::remove(s64 index) {
    // If this is a view, we don't want to modify the original string!
    if (!is_owner()) reserve(0);

    auto *target = get_cp_at_index(Data, Length, index);

    s64 cpSize = get_size_of_cp(target);
    --Length;

    u64 offset = (u64)(target - Data);
    copy_memory((char *) Data + offset, target + cpSize, ByteLength - offset - cpSize);

    ByteLength -= cpSize;

    return this;
}

string *string::remove(s64 begin, s64 end) {
    // If this is a view, we don't want to modify the original string!
    if (!is_owner()) reserve(0);

    auto *targetBegin = get_cp_at_index(Data, Length, begin);
    auto *targetEnd = get_cp_at_index(Data, Length, end, true);

    assert(targetEnd > targetBegin);

    s64 bytes = targetEnd - targetBegin;
    Length -= utf8_length(targetBegin, targetEnd - targetBegin);

    u64 offset = (u64)(targetBegin - Data);
    copy_memory((char *) Data + offset, targetEnd, ByteLength - offset - bytes);

    ByteLength -= bytes;

    return this;
}

string *string::repeat(s64 n) {
    string contents = *this;
    reserve(n * contents.ByteLength);
    For(range(1, n)) { append(contents); }
    return this;
}

string *string::to_lower() {
    For(range(Length)) set(it, ::to_lower(get(it)));
    return this;
}

string *string::to_upper() {
    For(range(Length)) set(it, ::to_upper(get(it)));
    return this;
}

string *string::remove_all(char32_t cp) {
    if (Length == 0) return this;

    s64 offset = 0;
    For(range(0, Length)) {
        if (get(it - offset) == cp) {
            remove(it - offset++);
        }
    }
    return this;
}

string *string::remove_all(string str) {
    assert(str.Length);

    if (Length == 0) return this;

    s64 offset = 0;
    For(range(0, Length)) {
        auto realIt = it - offset;

        auto progress = str.begin();
        for (auto search = begin() + realIt; progress != str.end(); ++search, ++progress) {
            if (*search != *progress) break;
        }
        if (progress == str.end()) {
            remove(realIt, realIt + str.Length);
            offset += str.Length;
        }
    }
    return this;
}

string *string::replace_all(char32_t oldCp, char32_t newCp) {
    if (Length == 0) return this;
    For(range(0, Length)) {
        if (get(it) == oldCp) set(it, newCp);
    }
    return this;
}

string *string::replace_all(string oldStr, string newStr) {
    assert(oldStr.Length != 0);

    if (Length == 0) return this;

    s64 diff = newStr.Length - oldStr.Length;
    for (s64 it = 0; it < Length; ++it) {
        auto progress = oldStr.begin();
        for (auto search = begin() + it; search != end() && progress != oldStr.end(); ++search, ++progress) {
            if (*search != *progress) break;
        }
        if (progress == oldStr.end()) {
            remove(it, it + oldStr.Length);
            insert(it, newStr);
            it += diff;
        }
    }
    return this;
}

string *string::replace_all(char32_t oldCp, string newStr) {
    char encoded[4];
    encode_cp(encoded, oldCp);
    return replace_all(string(encoded, get_size_of_cp(encoded)), newStr);
}

string *string::replace_all(string oldStr, char32_t newCp) {
    char encoded[4];
    encode_cp(encoded, newCp);
    return replace_all(oldStr, string(encoded, get_size_of_cp(encoded)));
}

string *clone(string *dest, string src) {
    dest->release();
    *dest = {};
    dest->append(src);
    return dest;
}

string *move(string *dest, string *src) {
    dest->release();
    *dest = *src;

    if (!src->is_owner()) return dest;

    // Transfer ownership
    encode_owner(src->Data, dest);
    encode_owner(dest->Data, dest);
    return dest;
}

LSTD_END_NAMESPACE
