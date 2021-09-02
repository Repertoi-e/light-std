#include "../types.h"

using namespace lstd;

constexpr bool a = types::is_integral<s128>;
static_assert(types::is_signed_integral<s128>);
