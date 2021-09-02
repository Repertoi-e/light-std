#include "string.h"

#include "../common/context.h"

LSTD_BEGIN_NAMESPACE

string::code_point_ref &string::code_point_ref::operator=(utf32 other) {
    string_set(*Parent, Index, other);
    return *this;
}

string::code_point_ref::operator utf32() const { return (*(const string *) Parent)[Index]; }

string::string(utf32 codePoint, s64 repeat) {
    string_reserve(*this, get_size_of_cp(codePoint) * repeat);

    s64 cpSize = get_size_of_cp(codePoint);

    auto *data = Data;
    For(range(repeat)) {
        encode_cp(data, codePoint);
        data += cpSize;
        ++Length;
    }

    Count = Length * cpSize;
}

// Allocates a buffer, copies the string's contents and also appends a zero terminator.
// The caller is responsible for freeing.
[[nodiscard("Leak")]] utf8 *string_to_c_string(const string &s, allocator alloc) {
    utf8 *result = allocate_array<utf8>(s.Count + 1, {.Alloc = alloc});
    copy_memory(result, s.Data, s.Count);
    result[s.Count] = '\0';
    return result;
}

// Allocates a buffer, copies the string's contents and also appends a zero terminator.
// Uses the temporary allocator.
utf8 *string_to_c_string_temp(const string &s) { return string_to_c_string(s, Context.TempAlloc); }

// Sets the _index_'th code point in the string.
void string_set(string &s, s64 index, utf32 codePoint) {
    utf8 *targetOctet = (utf8 *) get_cp_at_index(s.Data, translate_index(index, s.Length));

    s64 cpSize       = get_size_of_cp(codePoint);
    s64 cpTargetSize = get_size_of_cp(targetOctet);

    s64 diff = cpSize - cpTargetSize;

    // Calculate the offset, because string_reserve might move the memory
    u64 offset = (u64) (targetOctet - s.Data);

    // We need to reserve in any case (even if the diff is 0 or negative),
    // because we need to make sure we can modify the string (it's not a view).
    string_reserve(s, abs(diff));

    targetOctet = s.Data + offset;

    // Make space for the new cp and encode it
    if (diff) {
        copy_memory(targetOctet + cpSize, targetOctet + cpTargetSize, s.Count - offset - cpTargetSize);
    }
    encode_cp(s.Data + offset, codePoint);

    // String length hasn't changed, but byte count might have.
    s.Count += diff;
}

void string_insert_at(string &s, s64 index, utf32 codePoint) {
    utf8 data[4];
    encode_cp(data, codePoint);

    s64 offset = get_cp_at_index(s.Data, translate_index(index, s.Length, true)) - s.Data;
    array_insert_at(s, offset, data, get_size_of_cp(data));

    ++s.Length;
}

void string_insert_at(string &s, s64 index, const utf8 *str, s64 size) {
    s64 offset = get_cp_at_index(s.Data, translate_index(index, s.Length, true)) - s.Data;

    array_insert_at(s, offset, str, size);
    s.Length += utf8_length(str, size);
}

void string_remove_at(string &s, s64 index) {
    auto *target = get_cp_at_index(s.Data, translate_index(index, s.Length, true));
    s64 offset   = target - s.Data;

    array_remove_range(s, offset, offset + get_size_of_cp(target));

    --s.Length;
}

void string_remove_range(string &s, s64 begin, s64 end) {
    s64 tbegin = translate_index(begin, s.Length);
    s64 tend   = translate_index(end, s.Length, true);

    auto *t1 = get_cp_at_index(s.Data, tbegin), *t2 = get_cp_at_index(s.Data, tend);

    s64 bi = t1 - s.Data, ei = t2 - s.Data;
    array_remove_range(s, bi, ei);

    s.Length -= tend - tbegin;
}

string *clone(string *dest, const string &src) {
    string_reset(*dest);
    string_append(*dest, src);
    return dest;
}

LSTD_END_NAMESPACE
