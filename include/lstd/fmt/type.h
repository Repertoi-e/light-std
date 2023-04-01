#pragma once

#include "../common.h"
#include "../string.h"

LSTD_BEGIN_NAMESPACE

enum class fmt_type {
  NONE = 0,

  S64,
  U64,
  BOOL,
  LAST_INTEGRAL = BOOL,

  F32,
  F64,
  LAST_ARITHMETIC = F64,

  STRING,
  POINTER,

  CUSTOM
};

inline bool fmt_is_type_integral(fmt_type type) {
  return type > fmt_type::NONE && type <= fmt_type::LAST_INTEGRAL;
}

inline bool fmt_is_type_arithmetic(fmt_type type) {
  return type > fmt_type::NONE && type <= fmt_type::LAST_ARITHMETIC;
}

template <typename T>
struct fmt_type_constant : integral_constant<fmt_type, fmt_type::CUSTOM> {};

#define TYPE_CONSTANT(Type, constant) \
  template <>                         \
  struct fmt_type_constant<Type> : integral_constant<fmt_type, constant> {}

TYPE_CONSTANT(char, fmt_type::S64);
TYPE_CONSTANT(s32, fmt_type::S64);
TYPE_CONSTANT(s64, fmt_type::S64);
TYPE_CONSTANT(u32, fmt_type::U64);
TYPE_CONSTANT(u64, fmt_type::U64);
TYPE_CONSTANT(bool, fmt_type::BOOL);
TYPE_CONSTANT(f32, fmt_type::F32);
TYPE_CONSTANT(f64, fmt_type::F64);
TYPE_CONSTANT(string, fmt_type::STRING);
TYPE_CONSTANT(void *, fmt_type::POINTER);
#undef TYPE_CONSTANT

template <typename T>
auto fmt_type_constant_v = fmt_type_constant<remove_cvref_t<T>>::value;

LSTD_END_NAMESPACE
