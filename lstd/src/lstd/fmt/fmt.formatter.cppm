module;

#include "../common.h"

export module lstd.fmt.formatter;

LSTD_BEGIN_NAMESPACE

export {
    //
    // Specialize this for custom types.
    //
    // e.g.
    //
    // template <>
    // struct formatter<my_vector> {
    //     void format(fmt_context *f, my_vector *value) {
    //         fmt_to_writer(f, "x: {}, y: {}", value->x, value->y);
    // 		   ...
    // 	   }
    // };
    template <typename T>
    struct formatter {
        formatter() = delete;
    };

    // Can T be formatted with a custom formatter
    template <typename T>
    concept formattable = requires(T) { formatter<T>{}; };
}

LSTD_END_NAMESPACE
