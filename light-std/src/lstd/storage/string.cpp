#include "string.h"

#include "../context.h"

LSTD_BEGIN_NAMESPACE

string::code_point &string::code_point::operator=(char32_t other) {
    Parent->set((s64) Index, other);
    return *this;
}

string::code_point::operator char32_t() const { return ((const string *) Parent)->get((s64) Index); }

string::string(char32_t codePoint, size_t repeat, allocator alloc) : string(get_size_of_cp(codePoint) * repeat, alloc) {
    size_t cpSize = get_size_of_cp(codePoint);

    auto *data = const_cast<byte *>(Data);
    For(range(repeat)) {
        encode_cp(data, codePoint);
        data += cpSize;
        ++Length;
    }

    ByteLength = Length * cpSize;
}

string::string(const wchar_t *str) {
    reserve(2 * cstring_strlen(str));
    for (; *str; ++str) {
        append((char32_t) *str);
    }
}

string::string(const char32_t *str) {
    reserve(4 * cstring_strlen(str));
    for (; *str; ++str) {
        append(*str);
    }
}

string::string(size_t size, allocator alloc) { reserve(size, alloc); }

void string::reserve(size_t size, allocator alloc) {
    if (size < Reserved) return;

    if (!Reserved && size < ByteLength) {
        size += ByteLength;
    }

    size_t reserveTarget = 8;
    while (reserveTarget < size) {
        reserveTarget *= 2;
    }

    if (Reserved) {
        assert(is_owner() && "Cannot resize a buffer that isn't owned by this string.");

        auto *actualData = const_cast<byte *>(Data) - POINTER_SIZE;

        if (alloc) {
            auto *header = (allocation_header *) actualData - 1;
            assert(alloc.Function == header->AllocatorFunction && alloc.Context == header->AllocatorContext &&
                   "Calling reserve() on a string that already has reserved a buffer but with a different allocator. "
                   "Call with null allocator to avoid that.");
        }

        Data = (byte *) allocator::reallocate(actualData, reserveTarget + POINTER_SIZE) + POINTER_SIZE;
    } else {
        auto *oldData = Data;
        Data = encode_owner(new (alloc) byte[reserveTarget + POINTER_SIZE], this);
        if (ByteLength) copy_memory(const_cast<byte *>(Data), oldData, ByteLength);
    }
    Reserved = reserveTarget;
}

void string::release() {
    if (is_owner()) {
        delete (Data - POINTER_SIZE);
        Data = null;
        Length = ByteLength = Reserved = 0;
    }
}

void string::set(s64 index, char32_t codePoint) {
    size_t cpSize = get_size_of_cp(codePoint);

    auto *target = get_cp_at_index(Data, Length, index);
    size_t cpSizeTarget = get_size_of_cp(target);

    s64 diff = (s64) cpSize - (s64) cpSizeTarget;

    uptr_t offset = (uptr_t)(target - Data);
    reserve((size_t)((s64) ByteLength + diff));

    // We may have moved Data while reserving space!
    target = Data + offset;
    copy_memory((byte *) Data + offset + cpSize, target + cpSizeTarget, ByteLength - (target - Data) - cpSizeTarget);
    encode_cp((byte *) Data + offset, codePoint);

    ByteLength += diff;
}

void string::insert(s64 index, char32_t codePoint) {
    size_t cpSize = get_size_of_cp(codePoint);
    reserve(ByteLength + cpSize);

    auto *target = get_cp_at_index(Data, Length, index, true);
    uptr_t offset = (uptr_t)(target - Data);
    copy_memory((byte *) Data + offset + cpSize, target, ByteLength - (target - Data));

    encode_cp((byte *) Data + offset, codePoint);

    ByteLength += cpSize;
    ++Length;
}

void string::insert(s64 index, string str) { insert_pointer_and_size(index, str.Data, str.ByteLength); }

void string::insert_pointer_and_size(s64 index, const byte *str, size_t size) {
    reserve(ByteLength + size);

    auto *target = get_cp_at_index(Data, Length, index, true);
    uptr_t offset = (uptr_t)(target - Data);
    copy_memory((byte *) Data + offset + size, target, ByteLength - (target - Data));

    copy_memory((byte *) Data + offset, str, size);

    ByteLength += size;
    Length += utf8_strlen(str, size);
}

void string::remove(s64 index) {
    auto *target = get_cp_at_index(Data, Length, index);

    size_t cpSize = get_size_of_cp(target);
    --Length;

    uptr_t offset = (uptr_t)(target - Data);
    copy_memory((byte *) Data + offset, target + cpSize, ByteLength - offset - cpSize);

    ByteLength -= cpSize;
}

void string::remove(s64 begin, s64 end) {
    auto *targetBegin = get_cp_at_index(Data, Length, begin);
    auto *targetEnd = get_cp_at_index(Data, Length, end, true);

    assert(targetEnd > targetBegin);

    size_t bytes = targetEnd - targetBegin;
    Length -= utf8_strlen(targetBegin, targetEnd - targetBegin);

    uptr_t offset = (uptr_t)(targetBegin - Data);
    copy_memory((byte *) Data + offset, targetEnd, ByteLength - offset - bytes);

    ByteLength -= bytes;
}

string string::repeated(size_t n) const {
    string result = *this;
    result.reserve(n * ByteLength);
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

void string::remove_all(char32_t cp) {
    if (Length == 0) return;

    ptr_t offset = 0;
    For(range(0, Length)) {
        if (get(it - offset) == cp) {
            remove(it - offset++);
        }
    }
}

void string::remove_all(string str) {
    assert(str.Length);

    if (Length == 0) return;

    ptr_t offset = 0;
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
}

void string::replace_all(char32_t oldCp, char32_t newCp) {
    if (Length == 0) return;
    For(range(0, Length)) {
        if (get(it) == oldCp) set(it, newCp);
    }
}

void string::replace_all(string oldStr, string newStr) {
    assert(oldStr.Length != 0);

    if (Length == 0) return;

    ptr_t diff = (ptr_t) newStr.Length - (ptr_t) oldStr.Length;
    for (size_t it = 0; it < Length; ++it) {
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
}

string *clone(string *dest, string src) {
    *dest = {};
    dest->append(src);
    return dest;
}

string *move(string *dest, string src) {
    assert(src.is_owner());

    dest->release();
    *dest = src;

    // Transfer ownership
    change_owner(src.Data, dest);
    change_owner(dest->Data, dest);
    return dest;
}

LSTD_END_NAMESPACE
