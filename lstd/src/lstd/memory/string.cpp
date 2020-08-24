#include "string.h"

#include "../internal/context.h"

LSTD_BEGIN_NAMESPACE

string::code_point_ref &string::code_point_ref::operator=(utf32 other) {
    Parent->set(Index, other);
    return *this;
}

string::code_point_ref::operator utf32() const { return ((const string *) Parent)->get(Index); }

string::string(utf32 codePoint, s64 repeat) {
    reserve(get_size_of_cp(codePoint) * repeat);

    s64 cpSize = get_size_of_cp(codePoint);

    auto *data = Data;
    For(range(repeat)) {
        encode_cp(data, codePoint);
        data += cpSize;
        ++Length;
    }

    Count = Length * cpSize;
}

string::string(const utf16 *str) {
    reserve(2 * c_string_length(str));
    for (; *str; ++str) {
        append((utf32) *str);
    }
}

string::string(const utf32 *str) {
    reserve(4 * c_string_length(str));
    for (; *str; ++str) {
        append(*str);
    }
}

void string::reserve(s64 target) {
    if (Count + target < Allocated) return;

    target = max<s64>(ceil_pow_of_2(target + Count + 1), 8);

    if (Allocated) {
        Data = reallocate_array(Data, target);
    } else {
        auto *oldData = Data;
        Data = allocate_array(utf8, target);
        // We removed the ownership system.
        // encode_owner(Data, this);
        if (Count) copy_memory(Data, oldData, Count);
    }
    Allocated = target;
}

void string::reset() {
    Length = Count = 0;
}

void string::release() {
    if (Allocated) free(Data);
    Data = null;
    Length = Count = Allocated = 0;
}

string *string::set(s64 index, utf32 codePoint) {
    s64 cpSize = get_size_of_cp(codePoint);

    auto *target = get_cp_at_index(Data, Length, index);
    s64 cpSizeTarget = get_size_of_cp(target);

    s64 diff = cpSize - cpSizeTarget;

    // Calculate the offset and then reserve (which may move the memory!)
    u64 offset = (u64)(target - Data);
    reserve(abs(diff));

    // We may have moved Data while reserving space!
    target = Data + offset;
    copy_memory(Data + offset + cpSize, target + cpSizeTarget, Count - (target - Data) - cpSizeTarget);
    encode_cp(Data + offset, codePoint);

    Count += diff;

    return this;
}

string *string::insert(s64 index, utf32 codePoint) {
    s64 cpSize = get_size_of_cp(codePoint);
    reserve(cpSize);

    auto *target = get_cp_at_index(Data, Length, index, true);
    u64 offset = (u64)(target - Data);
    copy_memory(Data + offset + cpSize, target, Count - (target - Data));

    encode_cp(Data + offset, codePoint);

    Count += cpSize;
    ++Length;

    return this;
}

string *string::insert_pointer_and_size(s64 index, const utf8 *str, s64 size) {
    reserve(size);

    // assert(str);
    if (!str) return this;
    if (size == 0) return this;

    auto *target = get_cp_at_index(Data, Length, index, true);
    u64 offset = (u64)(target - Data);
    copy_memory(Data + offset + size, target, Count - (target - Data));

    copy_memory(Data + offset, str, size);

    Count += size;
    Length += utf8_length(str, size);

    return this;
}

string *string::remove(s64 index) {
    if (!Allocated) reserve(0);

    auto *target = get_cp_at_index(Data, Length, index);

    s64 cpSize = get_size_of_cp(target);
    --Length;

    u64 offset = (u64)(target - Data);
    copy_memory(Data + offset, target + cpSize, Count - offset - cpSize);

    Count -= cpSize;

    return this;
}

string *string::remove_range(s64 begin, s64 end) {
    if (!Allocated) reserve(0);

    auto *targetBegin = get_cp_at_index(Data, Length, begin);
    auto *targetEnd = get_cp_at_index(Data, Length, end, true);

    assert(targetEnd > targetBegin);

    s64 bytes = targetEnd - targetBegin;
    Length -= utf8_length(targetBegin, targetEnd - targetBegin);

    u64 offset = (u64)(targetBegin - Data);
    copy_memory(Data + offset, targetEnd, Count - offset - bytes);

    Count -= bytes;

    return this;
}

string *string::repeat(s64 n) {
    string contents = *this;
    reserve(n * contents.Count);
    For(range(1, n)) { append_string(contents); }
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

string *string::remove_all(utf32 cp) {
    if (Length == 0) return this;

    s64 offset = 0;
    For(range(0, Length)) {
        if (get(it - offset) == cp) {
            remove(it - offset++);
        }
    }
    return this;
}

string *string::remove_all(const string &str) {
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
            remove_range(realIt, realIt + str.Length);
            offset += str.Length;
        }
    }
    return this;
}

string *string::replace_all(utf32 oldCp, utf32 newCp) {
    if (Length == 0) return this;
    For(range(0, Length)) {
        if (get(it) == oldCp) set(it, newCp);
    }
    return this;
}

string *string::replace_all(const string &oldStr, const string &newStr) {
    assert(oldStr.Length != 0);

    if (Length == 0) return this;

    s64 diff = newStr.Length - oldStr.Length;
    for (s64 it = 0; it < Length; ++it) {
        auto progress = oldStr.begin();
        for (auto search = begin() + it; search != end() && progress != oldStr.end(); ++search, ++progress) {
            if (*search != *progress) break;
        }
        if (progress == oldStr.end()) {
            remove_range(it, it + oldStr.Length);
            insert_string(it, newStr);
            it += diff;
        }
    }
    return this;
}

string *string::replace_all(utf32 oldCp, const string &newStr) {
    utf8 encoded[4];
    encode_cp(encoded, oldCp);
    return replace_all(string(encoded, get_size_of_cp(encoded)), newStr);
}

string *string::replace_all(const string &oldStr, utf32 newCp) {
    utf8 encoded[4];
    encode_cp(encoded, newCp);
    return replace_all(oldStr, string(encoded, get_size_of_cp(encoded)));
}

string *clone(string *dest, const string &src) {
    dest->reset();
    dest->append_string(src);
    return dest;
}

LSTD_END_NAMESPACE
