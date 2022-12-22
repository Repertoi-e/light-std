module;

#include "../common.h"

export module lstd.fmt.fmt_type;

LSTD_BEGIN_NAMESPACE

export {
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

    bool fmt_is_type_integral(fmt_type type) {
        return type > fmt_type::NONE && type <= fmt_type::LAST_INTEGRAL;
    }

    bool fmt_is_type_arithmetic(fmt_type type) {
        return type > fmt_type::NONE && type <= fmt_type::LAST_ARITHMETIC;
    }
}

LSTD_END_NAMESPACE
