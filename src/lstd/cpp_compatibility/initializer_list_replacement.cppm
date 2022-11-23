module;

#include "../common/namespace.h"

export module lstd.initializer_list_replacement;

// Don't import this if you are including the STL.

export namespace std {
template <typename T>
struct initializer_list {
    const T *First = nullptr;
    const T *Last  = nullptr;

    using value_type      = T;
    using reference       = const T &;
    using const_reference = const T &;
    using size_type       = size_t;

    constexpr initializer_list() noexcept {
    }

    constexpr initializer_list(const T *first, const T *last) noexcept
        : First(first),
          Last(last) {
    }

    using iterator       = const T *;
    using const_iterator = const T *;

    constexpr const T *begin() const noexcept { return First; }
    constexpr const T *end() const noexcept { return Last; }

    constexpr size_t size() const noexcept { return static_cast<size_t>(Last - First); }
};
}  // namespace std

LSTD_BEGIN_NAMESPACE
export template <typename T>
using initializer_list = std::initializer_list<T>;
LSTD_END_NAMESPACE
