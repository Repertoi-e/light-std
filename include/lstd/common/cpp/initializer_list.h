#pragma once

#include "../namespace.h"

#if defined LSTD_NO_CRT

namespace std {
template <typename T>
struct initializer_list {
  using value_type = T;
  using reference = const T &;
  using const_reference = const T &;
  using size_type = size_t;

  initializer_list() noexcept {}

  initializer_list(const T *first, const T *last) noexcept
      : First(first), Last(last) {}

  using iterator = const T *;
  using const_iterator = const T *;

  const T *begin() const noexcept { return First; }
  const T *end() const noexcept { return Last; }

  size_t size() const noexcept { return static_cast<size_t>(Last - First); }

  // private in order to be compatible with std::
 private:
  const T *First = nullptr;
  const T *Last = nullptr;
};
}  // namespace std
#else
#include <initializer_list>
#endif

LSTD_BEGIN_NAMESPACE
template <typename T>
using initializer_list = std::initializer_list<T>;
LSTD_END_NAMESPACE
