#pragma once

#include "types.h"

//
// numeric_info is useful when writing template functions and you don't know the specific integral type, so you cannot just use S32_MAX for example.
// These template definitions are == to std::numeric_limits.
//

LSTD_BEGIN_NAMESPACE

enum class float_round_style {
    Indeterminate = -1,
    To_Zero,
    To_Nearest,
    To_Infinity,
    To_Negative_Infinity
};

enum class float_denorm_style {
    Indeterminate = -1,  // It cannot be determined whether or not the type allows denormalized values.
    Absent,              // The type does not allow denormalized values.
    Present              // The type allows denormalized values.
};

struct numeric_info_base {
    static constexpr float_denorm_style has_denorm = float_denorm_style::Absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr bool is_bounded = false;
    static constexpr bool is_exact = false;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_integral = false;
    static constexpr bool is_modulo = false;
    static constexpr bool is_signed = false;
    static constexpr bool is_specialized = false;
    static constexpr bool tinyness_before = false;
    static constexpr bool traps = false;
    static constexpr float_round_style round_style = float_round_style::To_Zero;
    static constexpr s32 digits = 0;
    static constexpr s32 digits10 = 0;
    static constexpr s32 max_digits10 = 0;
    static constexpr s32 max_exponent = 0;
    static constexpr s32 max_exponent10 = 0;
    static constexpr s32 min_exponent = 0;
    static constexpr s32 min_exponent10 = 0;
    static constexpr s32 radix = 0;
};

template <typename T>
struct numeric_info : public numeric_info_base {
    using value_t = T;

    static value_t min() { return value_t(); }
    static value_t max() { return value_t(); }
    static value_t lowest() { return value_t(); }
    static value_t epsilon() { return value_t(); }
    static value_t round_error() { return value_t(); }
    static value_t denorm_min() { return value_t(); }
    static value_t infinity() { return value_t(); }
    static value_t quiet_NaN() { return value_t(); }
    static value_t signaling_NaN() { return value_t(); }
};

// Const/volatile variations of numeric_info.
template <typename T>
struct numeric_info<const T> : public numeric_info<T> {};

template <typename T>
struct numeric_info<volatile T> : public numeric_info<T> {};

template <typename T>
struct numeric_info<const volatile T> : public numeric_info<T> {};

// Base for integer types
struct numeric_info_int_base : numeric_info_base {
    static constexpr bool is_bounded = true;
    static constexpr bool is_exact = true;
    static constexpr bool is_integral = true;
    static constexpr bool is_specialized = true;
    static constexpr s32 radix = 2;
};

// Base for floating-point types
struct numeric_info_float_base : numeric_info_base {
    static constexpr float_denorm_style has_denorm = float_denorm_style::Present;
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = true;
    static constexpr bool is_bounded = true;
    static constexpr bool is_iec559 = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_specialized = true;
    static constexpr float_round_style round_style = float_round_style::To_Nearest;
    static constexpr s32 radix = F32_RADIX;
};

template <>
struct numeric_info<char> : public numeric_info_int_base {
    static constexpr char min() { return S8_MIN; }
    static constexpr char max() { return S8_MAX; }
    static constexpr char lowest() { return min(); }
    static constexpr char epsilon() { return 0; }
    static constexpr char round_error() { return 0; }
    static constexpr char denorm_min() { return 0; }
    static constexpr char infinity() { return 0; }
    static constexpr char quiet_NaN() { return 0; }
    static constexpr char signaling_NaN() { return 0; }

    static constexpr bool is_signed = S8_MIN != 0;
    static constexpr bool is_modulo = S8_MIN == 0;
    static constexpr s32 digits = 8 - (S8_MIN != 0);
    static constexpr s32 digits10 = 2;
};

template <>
struct numeric_info<wchar_t> : public numeric_info_int_base {
    static constexpr wchar_t min() { return WCHAR_MIN; }
    static constexpr wchar_t max() { return WCHAR_MAX; }
    static constexpr wchar_t lowest() { return min(); }
    static constexpr wchar_t epsilon() { return 0; }
    static constexpr wchar_t round_error() { return 0; }
    static constexpr wchar_t denorm_min() { return 0; }
    static constexpr wchar_t infinity() { return 0; }
    static constexpr wchar_t quiet_NaN() { return 0; }
    static constexpr wchar_t signaling_NaN() { return 0; }

    static constexpr bool is_modulo = true;
    static constexpr s32 digits = 16;
    static constexpr s32 digits10 = 4;
};

template <>
struct numeric_info<bool> : public numeric_info_int_base {  // limits for type bool
    static constexpr bool min() { return false; }
    static constexpr bool max() { return true; }
    static constexpr bool lowest() { return min(); }
    static constexpr bool epsilon() { return 0; }
    static constexpr bool round_error() { return 0; }
    static constexpr bool denorm_min() { return 0; }
    static constexpr bool infinity() { return 0; }
    static constexpr bool quiet_NaN() { return 0; }
    static constexpr bool signaling_NaN() { return 0; }

    static constexpr s32 digits = 1;
};

template <>
struct numeric_info<u8> : public numeric_info_int_base {
    static constexpr u8 min() { return 0; }
    static constexpr u8 max() { return U8_MAX; }
    static constexpr u8 lowest() { return min(); }
    static constexpr u8 epsilon() { return 0; }
    static constexpr u8 round_error() { return 0; }
    static constexpr u8 denorm_min() { return 0; }
    static constexpr u8 infinity() { return 0; }
    static constexpr u8 quiet_NaN() { return 0; }
    static constexpr u8 signaling_NaN() { return 0; }

    static constexpr bool is_modulo = true;
    static constexpr s32 digits = 8;
    static constexpr s32 digits10 = 2;
};

template <>
struct numeric_info<s16> : public numeric_info_int_base {
    static constexpr s16 min() { return S16_MIN; }
    static constexpr s16 max() { return S16_MAX; }
    static constexpr s16 lowest() { return min(); }
    static constexpr s16 epsilon() { return 0; }
    static constexpr s16 round_error() { return 0; }
    static constexpr s16 denorm_min() { return 0; }
    static constexpr s16 infinity() { return 0; }
    static constexpr s16 quiet_NaN() { return 0; }
    static constexpr s16 signaling_NaN() { return 0; }

    static constexpr bool is_signed = true;
    static constexpr s32 digits = 15;
    static constexpr s32 digits10 = 4;
};

#ifdef _NATIVE_WCHAR_T_DEFINED
template <>
struct numeric_info<u16> : public numeric_info_int_base {
    static constexpr u16 min() { return 0; }
    static constexpr u16 max() { return U16_MAX; }
    static constexpr u16 lowest() { return min(); }
    static constexpr u16 epsilon() { return 0; }
    static constexpr u16 round_error() { return 0; }
    static constexpr u16 denorm_min() { return 0; }
    static constexpr u16 infinity() { return 0; }
    static constexpr u16 quiet_NaN() { return 0; }
    static constexpr u16 signaling_NaN() { return 0; }

    static constexpr bool is_modulo = true;
    static constexpr s32 digits = 16;
    static constexpr s32 digits10 = 4;
};
#endif

template <>
struct numeric_info<char8_t> : public numeric_info_int_base {
    static constexpr char8_t min() { return 0; }
    static constexpr char8_t max() { return U8_MAX; }
    static constexpr char8_t lowest() { return min(); }
    static constexpr char8_t epsilon() { return 0; }
    static constexpr char8_t round_error() { return 0; }
    static constexpr char8_t denorm_min() { return 0; }
    static constexpr char8_t infinity() { return 0; }
    static constexpr char8_t quiet_NaN() { return 0; }
    static constexpr char8_t signaling_NaN() { return 0; }

    static constexpr bool is_modulo = true;
    static constexpr s32 digits = 8;
    static constexpr s32 digits10 = 2;
};

template <>
struct numeric_info<char16_t> : public numeric_info_int_base {
    static constexpr char16_t min() { return 0; }
    static constexpr char16_t max() { return U16_MAX; }
    static constexpr char16_t lowest() { return min(); }
    static constexpr char16_t epsilon() { return 0; }
    static constexpr char16_t round_error() { return 0; }
    static constexpr char16_t denorm_min() { return 0; }
    static constexpr char16_t infinity() { return 0; }
    static constexpr char16_t quiet_NaN() { return 0; }
    static constexpr char16_t signaling_NaN() { return 0; }

    static constexpr bool is_modulo = true;
    static constexpr s32 digits = 16;
    static constexpr s32 digits10 = 4;
};

template <>
struct numeric_info<s32> : public numeric_info_int_base {
    static constexpr s32 min() { return S32_MIN; }
    static constexpr s32 max() { return S32_MAX; }
    static constexpr s32 lowest() { return min(); }
    static constexpr s32 epsilon() { return 0; }
    static constexpr s32 round_error() { return 0; }
    static constexpr s32 denorm_min() { return 0; }
    static constexpr s32 infinity() { return 0; }
    static constexpr s32 quiet_NaN() { return 0; }
    static constexpr s32 signaling_NaN() { return 0; }

    static constexpr bool is_signed = true;
    static constexpr s32 digits = 31;
    static constexpr s32 digits10 = 9;
};

template <>
struct numeric_info<u32> : public numeric_info_int_base {
    static constexpr u32 min() { return 0; }
    static constexpr u32 max() { return U32_MAX; }
    static constexpr u32 lowest() { return min(); }
    static constexpr u32 epsilon() { return 0; }
    static constexpr u32 round_error() { return 0; }
    static constexpr u32 denorm_min() { return 0; }
    static constexpr u32 infinity() { return 0; }
    static constexpr u32 quiet_NaN() { return 0; }
    static constexpr u32 signaling_NaN() { return 0; }

    static constexpr bool is_modulo = true;
    static constexpr s32 digits = 32;
    static constexpr s32 digits10 = 9;
};

template <>
struct numeric_info<long> : public numeric_info_int_base {
    static_assert(sizeof(s32) == sizeof(long));
    static constexpr long min() { return S32_MIN; }
    static constexpr long max() { return S32_MAX; }
    static constexpr long lowest() { return min(); }
    static constexpr long epsilon() { return 0; }
    static constexpr long round_error() { return 0; }
    static constexpr long denorm_min() { return 0; }
    static constexpr long infinity() { return 0; }
    static constexpr long quiet_NaN() { return 0; }
    static constexpr long signaling_NaN() { return 0; }

    static constexpr bool is_signed = true;
    static constexpr s32 digits = 31;
    static constexpr s32 digits10 = 9;
};

template <>
struct numeric_info<unsigned long> : public numeric_info_int_base {
    static_assert(sizeof(u32) == sizeof(unsigned long));
    static constexpr unsigned long min() { return 0; }
    static constexpr unsigned long max() { return U32_MAX; }
    static constexpr unsigned long lowest() { return min(); }
    static constexpr unsigned long epsilon() { return 0; }
    static constexpr unsigned long round_error() { return 0; }
    static constexpr unsigned long denorm_min() { return 0; }
    static constexpr unsigned long infinity() { return 0; }
    static constexpr unsigned long quiet_NaN() { return 0; }
    static constexpr unsigned long signaling_NaN() { return 0; }

    static constexpr bool is_modulo = true;
    static constexpr s32 digits = 32;
    static constexpr s32 digits10 = 9;
};

template <>
struct numeric_info<char32_t> : public numeric_info_int_base {
   public:
    static constexpr char32_t min() { return 0; }
    static constexpr char32_t max() { return U32_MAX; }
    static constexpr char32_t lowest() { return min(); }
    static constexpr char32_t epsilon() { return 0; }
    static constexpr char32_t round_error() { return 0; }
    static constexpr char32_t denorm_min() { return 0; }
    static constexpr char32_t infinity() { return 0; }
    static constexpr char32_t quiet_NaN() { return 0; }
    static constexpr char32_t signaling_NaN() { return 0; }

    static constexpr bool is_modulo = true;
    static constexpr s32 digits = 32;
    static constexpr s32 digits10 = 9;
};

template <>
struct numeric_info<s64> : public numeric_info_int_base {
    static constexpr s64 min() { return S64_MIN; }
    static constexpr s64 max() { return S64_MAX; }
    static constexpr s64 lowest() { return min(); }
    static constexpr s64 epsilon() { return 0; }
    static constexpr s64 round_error() { return 0; }
    static constexpr s64 denorm_min() { return 0; }
    static constexpr s64 infinity() { return 0; }
    static constexpr s64 quiet_NaN() { return 0; }
    static constexpr s64 signaling_NaN() { return 0; }

    static constexpr bool is_signed = true;
    static constexpr s32 digits = 63;
    static constexpr s32 digits10 = 18;
};

template <>
struct numeric_info<u64> : public numeric_info_int_base {
   public:
    static constexpr u64 min() { return 0; }
    static constexpr u64 max() { return U64_MAX; }
    static constexpr u64 lowest() { return min(); }
    static constexpr u64 epsilon() { return 0; }
    static constexpr u64 round_error() { return 0; }
    static constexpr u64 denorm_min() { return 0; }
    static constexpr u64 infinity() { return 0; }
    static constexpr u64 quiet_NaN() { return 0; }
    static constexpr u64 signaling_NaN() { return 0; }

    static constexpr bool is_modulo = true;
    static constexpr s32 digits = 64;
    static constexpr s32 digits10 = 19;
};

template <>
struct numeric_info<f32> : public numeric_info_float_base {
   public:
    static constexpr f32 min() { return F32_MIN; }
    static constexpr f32 max() { return F32_MAX; }
    static constexpr f32 lowest() { return -max(); }
    static constexpr f32 epsilon() { return F32_EPSILON; }
    static constexpr f32 round_error() { return 0.5F; }
    static constexpr f32 denorm_min() { return F32_TRUE_MIN; }
    static constexpr f32 infinity() { return __builtin_huge_valf(); }
    static constexpr f32 quiet_NaN() { return __builtin_nanf("0"); }
    static constexpr f32 signaling_NaN() { return __builtin_nansf("1"); }

    static constexpr s32 digits = F32_MANT_DIG;
    static constexpr s32 digits10 = F32_DIG;
    static constexpr s32 max_digits10 = 9;
    static constexpr s32 max_exponent = F32_MAX_EXP;
    static constexpr s32 max_exponent10 = F32_MAX_10_EXP;
    static constexpr s32 min_exponent = F32_MIN_EXP;
    static constexpr s32 min_exponent10 = F32_MIN_10_EXP;
};

template <>
struct numeric_info<f64> : public numeric_info_float_base {
   public:
    static constexpr f64 min() { return F64_MIN; }
    static constexpr f64 max() { return F64_MAX; }
    static constexpr f64 lowest() { return -max(); }
    static constexpr f64 epsilon() { return F64_EPSILON; }
    static constexpr f64 round_error() { return 0.5; }
    static constexpr f64 denorm_min() { return F64_TRUE_MIN; }
    static constexpr f64 infinity() { return __builtin_huge_val(); }
    static constexpr f64 quiet_NaN() { return __builtin_nan("0"); }
    static constexpr f64 signaling_NaN() { return __builtin_nans("1"); }

    static constexpr s32 digits = F64_MANT_DIG;
    static constexpr s32 digits10 = F64_DIG;
    static constexpr s32 max_digits10 = 17;
    static constexpr s32 max_exponent = F64_MAX_EXP;
    static constexpr s32 max_exponent10 = F64_MAX_10_EXP;
    static constexpr s32 min_exponent = F64_MIN_EXP;
    static constexpr s32 min_exponent10 = F64_MIN_10_EXP;
};

LSTD_END_NAMESPACE
