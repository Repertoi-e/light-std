#include "lstd/unicode.h"
#include "lstd/context.h"

LSTD_BEGIN_NAMESPACE

// This translation unit requires generated tables; run tools/gen_unicode.py before building.
#include "unicode_tables.inc"

extern const char *const g_unicode_script_names[]; // from generated file
extern const u8 g_unicode_ccc[UNICODE_TABLE_SIZE];
extern const u32 g_unicode_decomp_offsets[UNICODE_TABLE_SIZE];
extern const u32 g_unicode_decomp_array[];
extern const u64 g_unicode_comp_keys[];
extern const u32 g_unicode_comp_values[];
extern const u32 g_unicode_decomp_array_size;
extern const u32 g_unicode_comp_count;
extern const u64 g_unicode_prop_mask[];
extern const char* const g_unicode_prop_names[];

static inline u32 clamp_cp(code_point cp)
{
    if (cp < 0)
        return 0;
    if ((u32)cp >= UNICODE_TABLE_SIZE)
        return (u32)cp; // out of compiled range; identity
    return (u32)cp;
}

// Locale-aware simple tailorings (1:1 only). Currently supports Turkic.
code_point unicode_to_upper(code_point cp, text_locale loc)
{
    if (loc == text_locale::Unspecified)
        loc = Context.Locale;
    u32 c = clamp_cp(cp);
    if (c >= UNICODE_TABLE_SIZE)
        return cp; // identity beyond table
    // Turkic: i (0069) -> İ (0130)
    if (loc == text_locale::Turkic && c == 0x0069)
        return 0x0130;
    return (code_point)g_unicode_to_upper[c];
}

code_point unicode_to_lower(code_point cp, text_locale loc)
{
    if (loc == text_locale::Unspecified)
        loc = Context.Locale;
    u32 c = clamp_cp(cp);
    if (c >= UNICODE_TABLE_SIZE)
        return cp;
    // Turkic: I (0049) -> ı (0131); note: İ maps to i in simple mapping tables already
    if (loc == text_locale::Turkic && c == 0x0049)
        return 0x0131;
    return (code_point)g_unicode_to_lower[c];
}

unicode_general_category unicode_get_general_category(code_point cp)
{
    u32 c = clamp_cp(cp);
    if (c >= UNICODE_TABLE_SIZE)
        return unicode_general_category::Cn; // Unassigned/unknown
    return (unicode_general_category)g_unicode_general_category[c];
}

unicode_script unicode_get_script(code_point cp)
{
    u32 c = clamp_cp(cp);
    if (c >= UNICODE_TABLE_SIZE)
        return unicode_script::Unknown; // Unknown
    return (unicode_script)g_unicode_script[c];
}

bool unicode_has_property(code_point cp, unicode_property prop)
{
    u32 c = clamp_cp(cp);
    if (c >= UNICODE_TABLE_SIZE) return false;
    u32 pid = (u32)prop;
    if (pid >= (u32)unicode_property::Count) return false;
    u64 mask = g_unicode_prop_mask[c];
    return (mask >> pid) & 1ull;
}

const char* unicode_script_name(unicode_script id)
{
    if (id == unicode_script::Unknown) return "Unknown";
    return g_unicode_script_names[(s32)id];
}

u8 unicode_combining_class(code_point cp)
{
    u32 c = clamp_cp(cp);
    if (c >= UNICODE_TABLE_SIZE)
        return 0;
    return g_unicode_ccc[c];
}

s32 unicode_canonical_decompose(code_point cp, code_point *out, s32 cap)
{
    u32 c = clamp_cp(cp);
    if (c >= UNICODE_TABLE_SIZE)
    {
        if (cap > 0)
            out[0] = cp;
        return 1;
    }
    u32 off = g_unicode_decomp_offsets[c];
    if (off == 0)
    {
        if (cap > 0)
            out[0] = cp;
        return 1;
    }
    // Packed as: len, cp...
    u32 idx = off - 1;
    u32 len = g_unicode_decomp_array[idx];
    s32 n = (s32)len;
    for (s32 i = 0; i < n && i < cap; ++i)
    {
        out[i] = (code_point)g_unicode_decomp_array[idx + 1 + (u32)i];
    }
    return n;
}

static inline s32 comp_key_compare(const void *a, const void *b)
{
    u64 ka = *(const u64 *)a;
    u64 kb = *(const u64 *)b;
    return (ka < kb) ? -1 : (ka > kb) ? 1
                                      : 0;
}

code_point unicode_compose_pair(code_point a, code_point b)
{
    u64 key = ((u64)a << 21) | (u64)b;
    // binary search in g_unicode_comp_keys
    s64 lo = 0, hi = (s64)g_unicode_comp_count - 1;
    while (lo <= hi)
    {
        s64 mid = (lo + hi) >> 1;
        u64 k = g_unicode_comp_keys[mid];
        if (k < key)
            lo = mid + 1;
        else if (k > key)
            hi = mid - 1;
        else
            return (code_point)g_unicode_comp_values[mid];
    }
    return 0;
}

LSTD_END_NAMESPACE
