#pragma once

#include "common.h"

LSTD_BEGIN_NAMESPACE

//
// Dense Unicode tables with optional full-range toggle.
// Default: BMP only (0x0000..0xFFFF). Define LSTD_UNICODE_FULL_RANGE to use full range (0x0000..0x10FFFF).
// This affects executable size and memory usage of the tables significantly.
//
#if defined(LSTD_UNICODE_FULL_RANGE)
static constexpr u32 UNICODE_TABLE_SIZE = 0x110000u; // 1,114,112 code points
#else
static constexpr u32 UNICODE_TABLE_SIZE = 0x10000u; // BMP only
#endif

// General_Category as per Unicode.
// @Volatile Keep numeric values stable; generator uses the same order.
enum class unicode_general_category
{
    Lu,
    Ll,
    Lt,
    Lm,
    Lo, // ... Letter
    Mn,
    Mc,
    Me, // ... Mark
    Nd,
    Nl,
    No, // ... Number
    Pc,
    Pd,
    Ps,
    Pe,
    Pi,
    Pf,
    Po, // ... Punctuation
    Sm,
    Sc,
    Sk,
    So, // ... Symbol
    Zs,
    Zl,
    Zp, // ... Separator
    Cc,
    Cf,
    Cs,
    Co,
    Cn, // ... Other (Cn = Unassigned)
    Count
};

// Core property bit flags from DerivedCoreProperties.txt (subset).
enum unicode_core_property : u32
{
    UCP_Alphabetic = 1u << 0,
    UCP_White_Space = 1u << 1,
    UCP_Uppercase = 1u << 2,
    UCP_Lowercase = 1u << 3,
    UCP_Cased = 1u << 4,
    UCP_Case_Ignorable = 1u << 5,
    UCP_Default_Ignorable_Code_Point = 1u << 6,
};

enum class unicode_script
{
  Unknown = -1,
  Common = 0,
  Latin,
  Greek,
  Cyrillic,
  Armenian,
  Hebrew,
  Arabic,
  Syriac,
  Thaana,
  Devanagari,
  Bengali,
  Gurmukhi,
  Gujarati,
  Oriya,
  Tamil,
  Telugu,
  Kannada,
  Malayalam,
  Sinhala,
  Thai,
  Lao,
  Tibetan,
  Myanmar,
  Georgian,
  Hangul,
  Ethiopic,
  Cherokee,
  Canadian_Aboriginal,
  Ogham,
  Runic,
  Khmer,
  Mongolian,
  Hiragana,
  Katakana,
  Bopomofo,
  Han,
  Yi,
  Old_Italic,
  Gothic,
  Deseret,
  Inherited,
  Tagalog,
  Hanunoo,
  Buhid,
  Tagbanwa,
  Limbu,
  Tai_Le,
  Linear_B,
  Ugaritic,
  Shavian,
  Osmanya,
  Cypriot,
  Braille,
  Buginese,
  Coptic,
  New_Tai_Lue,
  Glagolitic,
  Tifinagh,
  Syloti_Nagri,
  Old_Persian,
  Kharoshthi,
  Balinese,
  Cuneiform,
  Phoenician,
  Phags_Pa,
  Nko,
  Sundanese,
  Lepcha,
  Ol_Chiki,
  Vai,
  Saurashtra,
  Kayah_Li,
  Rejang,
  Lycian,
  Carian,
  Lydian,
  Cham,
  Tai_Tham,
  Tai_Viet,
  Avestan,
  Egyptian_Hieroglyphs,
  Samaritan,
  Lisu,
  Bamum,
  Javanese,
  Meetei_Mayek,
  Imperial_Aramaic,
  Old_South_Arabian,
  Inscriptional_Parthian,
  Inscriptional_Pahlavi,
  Old_Turkic,
  Kaithi,
  Batak,
  Brahmi,
  Mandaic,
  Chakma,
  Meroitic_Cursive,
  Meroitic_Hieroglyphs,
  Miao,
  Sharada,
  Sora_Sompeng,
  Takri,
  Caucasian_Albanian,
  Bassa_Vah,
  Duployan,
  Elbasan,
  Grantha,
  Pahawh_Hmong,
  Khojki,
  Linear_A,
  Mahajani,
  Manichaean,
  Mende_Kikakui,
  Modi,
  Mro,
  Old_North_Arabian,
  Nabataean,
  Palmyrene,
  Pau_Cin_Hau,
  Old_Permic,
  Psalter_Pahlavi,
  Siddham,
  Khudawadi,
  Tirhuta,
  Warang_Citi,
  Ahom,
  Anatolian_Hieroglyphs,
  Hatran,
  Multani,
  Old_Hungarian,
  SignWriting,
  Adlam,
  Bhaiksuki,
  Marchen,
  Newa,
  Osage,
  Tangut,
  Masaram_Gondi,
  Nushu,
  Soyombo,
  Zanabazar_Square,
  Dogra,
  Gunjala_Gondi,
  Makasar,
  Medefaidrin,
  Hanifi_Rohingya,
  Sogdian,
  Old_Sogdian,
  Elymaic,
  Nandinagari,
  Nyiakeng_Puachue_Hmong,
  Wancho,
  Chorasmian,
  Dives_Akuru,
  Khitan_Small_Script,
  Yezidi,
  Cypro_Minoan,
  Old_Uyghur,
  Tangsa,
  Toto,
  Vithkuqi,
  Kawi,
  Nag_Mundari,
  Garay,
  Gurung_Khema,
  Kirat_Rai,
  Ol_Onal,
  Sunuwar,
  Todhri,
  Tulu_Tigalari,
  Count
};

enum class text_locale
{
    Unspecified = -1,
    Default = 0,
    Turkic = 1
};

// Properties and metadata
unicode_general_category unicode_get_general_category(code_point cp);        
unicode_script unicode_get_script(code_point cp);  
const char* unicode_script_to_string(unicode_script id);
bool unicode_has_core_prop(code_point cp, u32 mask);  // any of mask set?

// Canonical combining class (CCC) and normalization helpers
u8 unicode_combining_class(code_point cp);
// Returns number of code points written to out (canonical decomposition, 1 if none)
s32 unicode_canonical_decompose(code_point cp, code_point *out, s32 cap);
// Returns composed code point for (a,b) if such canonical composition exists, else 0
code_point unicode_compose_pair(code_point a, code_point b);

// If locale is Unspecified, it gets the locale from the Context.
code_point unicode_to_upper(code_point cp, text_locale loc = text_locale::Unspecified);

// If locale is Unspecified, it gets the locale from the Context.
code_point unicode_to_lower(code_point cp, text_locale loc = text_locale::Unspecified);

inline bool unicode_is_upper(code_point cp) { return unicode_has_core_prop(cp, UCP_Uppercase); }
inline bool unicode_is_lower(code_point cp) { return unicode_has_core_prop(cp, UCP_Lowercase); }
inline bool unicode_is_alpha(code_point cp) { return unicode_has_core_prop(cp, UCP_Alphabetic); }
inline bool unicode_is_whitespace(code_point cp) { return unicode_has_core_prop(cp, UCP_White_Space); }

inline bool unicode_is_letter(unicode_general_category gc)
{
    return gc <= unicode_general_category::Lo;
}

inline bool unicode_is_mark(unicode_general_category gc)
{
    return gc >= unicode_general_category::Mn && gc <= unicode_general_category::Me;
}

inline bool unicode_is_number(unicode_general_category gc)
{
    return gc >= unicode_general_category::Nd && gc <= unicode_general_category::No;
}

inline bool unicode_is_punctuation(unicode_general_category gc)
{
    return gc >= unicode_general_category::Pc && gc <= unicode_general_category::Po;
}

inline bool unicode_is_symbol(unicode_general_category gc)
{
    return gc >= unicode_general_category::Sm && gc <= unicode_general_category::So;
}

inline bool unicode_is_separator(unicode_general_category gc)
{
    return gc >= unicode_general_category::Zs && gc <= unicode_general_category::Zp;
}

inline bool unicode_is_other(unicode_general_category gc)
{
    return gc >= unicode_general_category::Cc;
}

inline bool unicode_is_letter(code_point cp)
{
    return unicode_is_letter(unicode_get_general_category(cp));
}

inline bool unicode_is_mark(code_point cp)
{
    return unicode_is_mark(unicode_get_general_category(cp));
}

inline bool unicode_is_number(code_point cp)
{
    return unicode_is_number(unicode_get_general_category(cp));
}

inline bool unicode_is_punctuation(code_point cp)
{
    return unicode_is_punctuation(unicode_get_general_category(cp));
}

inline bool unicode_is_symbol(code_point cp)
{
    return unicode_is_symbol(unicode_get_general_category(cp));
}

inline bool unicode_is_separator(code_point cp)
{
    return unicode_is_separator(unicode_get_general_category(cp));
}

inline bool unicode_is_other(code_point cp)
{
    return unicode_is_other(unicode_get_general_category(cp));
}

LSTD_END_NAMESPACE
