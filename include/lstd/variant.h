#pragma once

#include "common.h"
#include "math.h"

//
// variant<> is a type-safe union.
//
// optional<> is a type that either holds T or doesn't
// (implementation-wise it's just a variant between T and a null type).
//

//
// Here's some examples:
//
// optional<int> flip_a_coin() {
//     if (os_get_time() % 2 == 0) {
//         return int(2);
//     } else {
//         return {};
//     }
// }
//
// ...
//
// auto result = flip_a_coin();
// if (result) {
//     int a = result.strict_get<int>();   // This crashes if there's no _int_ in result. 
//     print("Number is {}\n", a);
// }
// else {
//     print("No number\n");
// }
//
//        or more-conveniently, you can pattern match:
//
// 	result.visit(match {
// 	    [](int a) { print("Number is {}\n", a); },
// 	    [](auto) { print("No number\n"); }
//  });
//
//
//
// variant<int, float> flip_a_coin() {
//     if (os_get_time() % 2 == 0) {
//         return int(2);
//     }
//     else {
//         return float(2);
//     }
// }
//
// ...
//
// auto result = flip_a_coin();
// result.visit(match {
//     [](int a) { print("Number is int {}\n", a); },
//     [](float a) { print("Number is float {}\n", a); },
//     [](auto) { print("No number\n"); }               // If the default
//     statement is not handled in  match, there will be a compiler error
// });
//
//

#include "type_info.h"

//
// The base code here is a modified version of cjxgm's:
// https://gist.github.com/cjxgm/83b08092411ede89ecb3
//

LSTD_BEGIN_NAMESPACE

#define FORWARD(x) ((decltype(x) &&)(x))
#define MOVE(x) ((remove_ref_t<decltype(x)> &&)(x))

namespace internal {

//------ const if
//
//	const_if<U, T>				=> T
//	const_if<U const*, T>		=> const T
//
template <class U, class T>
struct const_if_impl {
  using type = T;
};
template <class U, class T>
struct const_if_impl<U const *, T> {
  using type = const T;
};

template <class U, class T>
using const_if_t = typename const_if_impl<U, T>::type;

//
template <u64 I, class U, class... TS>
constexpr auto index_of_impl = I;

template <u64 I, class U, class T, class... TS>
constexpr auto index_of_impl<I, U, T, TS...> = index_of_impl<I + 1, U, TS...>;

template <u64 I, class U, class... TS>
constexpr auto index_of_impl<I, U, U, TS...> = I;

template <u64 I, class... TS>
constexpr auto check() {
  static_assert(I < sizeof...(TS), "Not one of the specified types");
  return I;
};

template <class U, class... TS>
constexpr auto checked = check<index_of_impl<0, U, TS...>, TS...>();

template <class U, class... TS>
constexpr auto index_of = checked<decay_t<U>, decay_t<TS>...>;
}  // namespace internal

template <typename... MEMBERS>
struct aligned_union {
  static constexpr auto ALIGNMENT = max(alignof(decay_t<MEMBERS>)...);
  static constexpr auto SIZE = max(sizeof(decay_t<MEMBERS>)...);

  alignas(ALIGNMENT) char Data[SIZE];

  template <typename T>
  requires(!__is_base_of(decay_t<T>, decay_t<aligned_union>))
      aligned_union(T &&x) {
    construct<T>(FORWARD(x));
  }
  aligned_union() : Data{} {}

  template <class T, class... TS>
  void construct(TS &&...xs) {
    using type = decay_t<T>;
    new (Data) type(FORWARD(xs)...);
  }

  template <class T>
  void destruct() {
    Data.~T();
  }

  template <class T>
  auto ref as() {
    using type = internal::const_if_t<decltype(this), decay_t<T>>;
    return *reinterpret_cast<type *>(Data);
  }

  template <class T>
  auto no_copy as() const {
    using type = internal::const_if_t<decltype(this), decay_t<T>>;
    return *reinterpret_cast<type *>(Data);
  }
};

template <class... MEMBERS>
struct variant {
  struct nil {};

  u64 ti;
  aligned_union<nil, MEMBERS...> au;

  template <class T>
  static constexpr auto get_index_of_t = internal::index_of<T, nil, MEMBERS...>;

  template <class T>
  requires(!__is_base_of(decay_t<T>, decay_t<variant>)) variant(T &&x)
      : ti{get_index_of_t<T>}, au{FORWARD(x)} {}
  variant() : variant(nil{}) {}
  variant(variant no_copy x) { x.visit_with_nil<copy_constructor>({*this}); }
  variant(variant &&x) { x.visit_with_nil<move_constructor>({*this}); }
  ~variant() { destruct(); }

  template <class T, class... TS>
  void emplace(TS &&...xs) {
    destruct();
    construct<T>(FORWARD(xs)...);
  }

  template <class T>
  auto is() const {
    return (get_index_of_t<T> == ti);
  }

  explicit operator bool() const { return !is<nil>(); }

  template <class F>
  decltype(auto) visit(F no_copy f = {}) {
    if (!ti)
      return visit_with_nil<F>(f);
    else
      return visit<F, MEMBERS...>(f, ti - 1);
  }

  template <class F>
  decltype(auto) visit(F no_copy f = {}) const {
    if (!ti)
      return visit_with_nil<F>(f);
    else
      return visit<F, MEMBERS...>(f, ti - 1);
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type"

  template <class T>
  auto ref strict_get() {
    if (is<T>())
      return as<T>();
    else
      panic();
  }

  template <class T>
  auto ref strict_get() const {
    if (is<T>())
      return as<T>();
    else
      panic();
  }

#pragma clang diagnostic pop

  template <class T>
  requires(!__is_base_of(decay_t<T>, decay_t<variant>)) auto ref operator=(
      T &&x) {
    if (is<T>())
      as<T>() = FORWARD(x);
    else
      emplace<T>(FORWARD(x));
    return *this;
  }

  auto ref operator=(variant x) {
    destruct();
    x.visit<move_constructor>({*this});
    return *this;
  }

 private:
  void panic() {
    // Bad, probably got here by reading an incorrect type from the variant/optional or 
    // by trying to read a value from an empty variant/optional.
    int *d = 0;
    *d = 42;
  }

  template <class T>
  auto ref as() {
    return au.template as<T>();
  }
  template <class T>
  auto no_copy as() const {
    return au.template as<T>();
  }

  template <class F, class T>
  decltype(auto) visit(F no_copy f, u64) {
    return f(as<T>());
  }

  template <class F, class T>
  decltype(auto) visit(F no_copy f, u64) const {
    return f(as<T>());
  }

  template <class F, class U, class T, class... TS>
  decltype(auto) visit(F no_copy f, u64 i) {
    if (i) return visit<F, T, TS...>(f, i - 1);
    return f(as<U>());
  }

  template <class F, class U, class T, class... TS>
  decltype(auto) visit(F no_copy f, u64 i) const {
    if (i) return visit<F, T, TS...>(f, i - 1);
    return f(as<U>());
  }

  template <class F>
  decltype(auto) visit_with_nil(F no_copy f = {}) {
    return visit<F, nil, MEMBERS...>(f, ti);
  }

  template <class F>
  decltype(auto) visit_with_nil(F no_copy f = {}) const {
    return visit<F, nil, MEMBERS...>(f, ti);
  }

  struct destructor {
    template <class T>
    void operator()(T ref x) const {
      x.~T();
    }
  };
  void destruct() { visit_with_nil<destructor>(); }

  template <class T, class... TS>
  void construct(TS &&...xs) {
    au.template construct<T>(FORWARD(xs)...);
    ti = get_index_of_t<T>;
  }

  struct move_constructor {
    variant ref self;
    move_constructor(variant ref self) : self{self} {}

    template <class T>
    void operator()(T &x) const {
      self.construct<T>(MOVE(x));
    }
  };

  struct copy_constructor {
    variant ref self;
    copy_constructor(variant ref self) : self{self} {}

    template <class T>
    void operator()(T no_copy x) const {
      self.construct<T>(x);
    }
  };
};

template <typename T>
using optional = variant<T>;

template <typename... Ts>
struct match : Ts... {
  using Ts::operator()...;
};
template <typename... Ts>
match(Ts...) -> match<Ts...>;

#undef FORWARD
#undef MOVE

LSTD_END_NAMESPACE
