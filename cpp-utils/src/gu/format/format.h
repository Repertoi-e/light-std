#pragma once

#include "../common.h"

#include <type_traits>

GU_BEGIN_NAMESPACE

namespace fmt {

// A formatter for objects of type T.
template <typename T, typename Char = char, typename Enable = void>
struct Formatter {
	static_assert(false, "There's no Formatter specialization for this type.");

	template <typename ParseContext>
	typename ParseContext::iterator parse(ParseContext &);

	template <typename FormatContext>
	auto format(const T &value, FormatContext &context) -> decltype(context.out());
};

template <typename T, typename Char, typename Enable = void>
struct Is_Convertible_To_Int : std::integral_constant<b32, !std::is_arithmetic_v<T> && std::is_convertible_v<T, s32>> {};


}
GU_END_NAMESPACE
