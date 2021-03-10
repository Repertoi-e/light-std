#include "string.h"

#include "../internal/context.h"

LSTD_BEGIN_NAMESPACE

string::code_point_ref &string::code_point_ref::operator=(utf32 other) {
    set(*Parent, Index, other);
    return *this;
}

string::code_point_ref::operator utf32() const { return (*((const string *) Parent))[Index]; }

string::string(utf32 codePoint, s64 repeat) {
    reserve(*this, get_size_of_cp(codePoint) * repeat);

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
    reserve(*this, 2 * c_string_length(str));
    for (; *str; ++str) {
        append_cp(*this, (utf32) *str);
    }
}

string::string(const utf32 *str) {
    reserve(*this, 4 * c_string_length(str));
    for (; *str; ++str) {
        append_cp(*this, *str);
    }
}

void reserve(string &s, s64 target) {
    if (s.Count + target < s.Allocated) return;

    target = max(ceil_pow_of_2(target + s.Count + 1), 8);

    if (s.Allocated) {
        s.Data = reallocate_array(s.Data, target);
    } else {
        auto *oldData = s.Data;
        s.Data = allocate_array<utf8>(target);
        // We removed the ownership system.
        // encode_owner(Data, this);
        if (s.Count) copy_memory(s.Data, oldData, s.Count);
    }
    s.Allocated = target;
}

void reset(string &s) {
    s.Length = s.Count = 0;
}

void free(string &s) {
    if (s.Allocated) free(s.Data);
    s.Data = null;
    s.Length = s.Count = s.Allocated = 0;
}

// Allocates a buffer, copies the string's contents and also appends a zero terminator.
// The caller is responsible for freeing.
[[nodiscard("Leak")]] utf8 *to_c_string(const string &s, allocator alloc) {
    utf8 *result = allocate_array<utf8>(s.Count + 1, {.Alloc = alloc});
    copy_memory(result, s.Data, s.Count);
    result[s.Count] = '\0';
    return result;
}

void set(string &s, s64 index, utf32 codePoint) {
    s64 cpSize = get_size_of_cp(codePoint);

    auto *target = get_cp_at_index(s.Data, s.Length, index);
    s64 cpSizeTarget = get_size_of_cp(target);

    s64 diff = cpSize - cpSizeTarget;

    // Calculate the offset and then reserve (which may move the memory!)
    u64 offset = (u64)(target - s.Data);
    reserve(s, abs(diff));

    // We may have moved Data while reserving space!
    target = s.Data + offset;
    copy_memory(s.Data + offset + cpSize, target + cpSizeTarget, s.Count - (target - s.Data) - cpSizeTarget);
    encode_cp(s.Data + offset, codePoint);

    s.Count += diff;
}

void insert(string &s, s64 index, utf32 codePoint) {
    s64 cpSize = get_size_of_cp(codePoint);
    reserve(s, cpSize);

    auto *target = get_cp_at_index(s.Data, s.Length, index, true);
    u64 offset = (u64)(target - s.Data);
    copy_memory(s.Data + offset + cpSize, target, s.Count - (target - s.Data));

    encode_cp(s.Data + offset, codePoint);

    s.Count += cpSize;
    ++s.Length;
}

void insert_pointer_and_size(string &s, s64 index, const utf8 *str, s64 size) {
    reserve(s, size);

    if (!str) return;
    if (size == 0) return;

    auto *target = get_cp_at_index(s.Data, s.Length, index, true);
    u64 offset = (u64)(target - s.Data);
    copy_memory(s.Data + offset + size, target, s.Count - (target - s.Data));

    copy_memory(s.Data + offset, str, size);

    s.Count += size;
    s.Length += utf8_length(str, size);
}

void remove_at_index(string &s, s64 index) {
    if (!s.Allocated) reserve(s, 0);

    auto *target = get_cp_at_index(s.Data, s.Length, index);

    s64 cpSize = get_size_of_cp(target);
    --s.Length;

    u64 offset = (u64)(target - s.Data);
    copy_memory(s.Data + offset, target + cpSize, s.Count - offset - cpSize);

    s.Count -= cpSize;
}

void remove_range(string &s, s64 begin, s64 end) {
    if (!s.Allocated) reserve(s, 0);

    auto *targetBegin = get_cp_at_index(s.Data, s.Length, begin);
    auto *targetEnd = get_cp_at_index(s.Data, s.Length, end, true);

    assert(targetEnd > targetBegin);

    s64 bytes = targetEnd - targetBegin;
    s.Length -= utf8_length(targetBegin, targetEnd - targetBegin);

    u64 offset = (u64)(targetBegin - s.Data);
    copy_memory(s.Data + offset, targetEnd, s.Count - offset - bytes);

    s.Count -= bytes;
}

void repeat(string &s, s64 n) {
    string contents;
    clone(&contents, s);
    reserve(s, (n - 1) * contents.Count);
    For(range(1, n)) { append_string(s, contents); }
}

void remove_all(string &s, utf32 cp) {
    if (!s) return;

    s64 offset = 0;
    For(range(0, s.Length)) {
        if (s[it - offset] == cp) remove_at_index(s, it - offset++);
    }
}

void remove_all(string &s, const string &str) {
    if (!s) return;

    assert(str.Length);

    s64 offset = 0;
    For(range(0, s.Length)) {
        auto realIt = it - offset;

        auto progress = str.begin();
        for (auto search = s.begin() + realIt; progress != str.end(); ++search, ++progress) {
            if (*search != *progress) break;
        }
        if (progress == str.end()) {
            remove_range(s, realIt, realIt + str.Length);
            offset += str.Length;
        }
    }
}

void replace_all(string &s, utf32 oldCp, utf32 newCp) {
    if (!s) return;
    For(s) {
        if (it == oldCp) it = newCp;
    }
}

void replace_all(string &s, const string &oldStr, const string &newStr) {
    if (!s) return;

    assert(oldStr.Length != 0);

    s64 diff = newStr.Length - oldStr.Length;
    for (s64 it = 0; it < s.Length; ++it) {
        auto progress = oldStr.begin();
        for (auto search = s.begin() + it; search != s.end() && progress != oldStr.end(); ++search, ++progress) {
            if (*search != *progress) break;
        }
        if (progress == oldStr.end()) {
            remove_range(s, it, it + oldStr.Length);
            insert_string(s, it, newStr);
            it += diff;
        }
    }
}

void replace_all(string &s, utf32 oldCp, const string &newStr) {
    utf8 encoded[4];
    encode_cp(encoded, oldCp);
    return replace_all(s, string(encoded, get_size_of_cp(encoded)), newStr);
}

void replace_all(string &s, const string &oldStr, utf32 newCp) {
    utf8 encoded[4];
    encode_cp(encoded, newCp);
    return replace_all(s, oldStr, string(encoded, get_size_of_cp(encoded)));
}

string *clone(string *dest, const string &src) {
    reset(*dest);
    append_string(*dest, src);
    return dest;
}

LSTD_END_NAMESPACE
