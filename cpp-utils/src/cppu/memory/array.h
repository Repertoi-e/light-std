#pragma once

#include "../common.h"

#include "range.h"

CPPU_BEGIN_NAMESPACE

template <typename T, size_t Size>
struct Array {
    T Data[Size];
    static constexpr size_t Count = Size;

    constexpr T *begin() { return Data; }
    constexpr T *end() { return Data + Count; }
    constexpr const T *begin() const { return Data; }
    constexpr const T *end() const { return Data + Count; }

    constexpr T &operator[](size_t index) { return Data[index]; }
    constexpr const T &operator[](size_t index) const { return Data[index]; }

    constexpr bool operator==(const Array &other) {
        if (Count != other.Count) return false;
        for (size_t i = 0; i < Count; i++) {
            if (Data[i] != other.Data[i]) {
                return false;
            }
        }
        return true;
    }

    constexpr bool operator!=(const Array &other) { return !(*this == other); }
};

template <typename... T>
constexpr auto to_array(T &&... values)
    -> Array<typename std::decay_t<typename std::common_type_t<T...>>, sizeof...(T)> {
    return Array<typename std::decay_t<typename std::common_type_t<T...>>, sizeof...(T)>{std::forward<T>(values)...};
}

#include "../format/fmt.h"

template <typename T, size_t Size>
struct fmt::Formatter<Array<T, Size>> {
    string_view::Iterator parse(const Parse_Context &parseContext) { return parseContext.It; }

    void format(const Array<T, Size> &value, Format_Context &f) {
        f.Out.append("{ { ");
        if (Size > 0) {
            f.Out.append(fmt::to_string(value[0]));
            for (s32 i : range(1, Size)) {
                f.Out.append(", ");
                f.Out.append(fmt::to_string(value[i]));
            }
        }
        f.Out.append(" }, Size: ");
        f.Out.append(fmt::to_string(Size));
        f.Out.append(" }");
        }
};

CPPU_END_NAMESPACE