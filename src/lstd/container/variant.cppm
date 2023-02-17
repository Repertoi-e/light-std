module;

#include "../common.h"

//
// variant<> is a type-safe union
//

export module lstd.variant;

export import lstd.type_info;

//
// The base code here is a modified version of Tomilov Anatoliy's 
// https://codereview.stackexchange.com/questions/112218/trivial-full-fledged-variant-for-constexpr-proof-of-concept
// We extended it with visit(), holds() and other useful 
// stuff from the C++17 standard of std::variant.
//

LSTD_BEGIN_NAMESPACE

template <s64 i>
using I = integral_constant<s64, i>;

template<s64 i>
struct index : I <i> {};

template <bool ...b>
struct get_i;
template<>
struct get_i<> {};
template <bool ...Rest>
struct get_i <true, Rest...> : index<sizeof...(Rest)> {};
template <bool ...Rest>
struct get_i <false, Rest...> : get_i<Rest...> {};
template <typename ...Types>
using get_di = get_i<is_constructible<Types>...>;

template <typename T, typename ...Types>
struct i_at {};
template <typename T, typename ...Rest>
struct i_at<T, T, Rest...> : index <sizeof...(Rest)> {};
template <typename T, typename First, typename ...Rest>
struct i_at<T, First, Rest...> : i_at<T, Rest...> {};
template <typename T, typename ...Types>
using i_at_t = typename i_at<T, Types...>::type;

template <bool td, typename ...Types>
struct C;

template <bool td>
struct C <td> {
	void destruct(s64) { ; }
};

template <typename First, typename ...Rest>
struct C <true, First, Rest...> {
	using head = First;
	using tail = C <true, Rest...>;

	union {
		head Head;
		tail Tail;
	};

	C() = default;

	template <typename ...A>
	C(I <(1 + sizeof...(Rest))>, A &&... a) : Head(((A&&)(a))...) {}

	template <typename ...A>
	C(A &&... a) : Tail(((A&&)(a))...) {}

	operator First no_copy() const { return Head; }
	operator First ref() { return Head; }
	template <typename T> operator T no_copy() const { return Tail; }
	template <typename T> operator T ref() { return Tail; }
};

template <typename First, typename ...Rest>
struct C <false, First, Rest...> {
	using head = First;
	using tail = C <false, Rest...>;

	union {
		head Head;
		tail Tail;
	};

	C() = default;

	C(C no_copy) = default;
	C(C ref) = default;
	C(C&&) = default;
	C ref operator = (C no_copy) = default;
	C ref operator = (C ref) = default;
	C ref operator = (C&&) = default;

	~C() { /*Tail.~tail();*/ }

	void destruct(s64 w) { (w == sizeof...(Rest) + 1) ? Head.~head() : Tail.destruct(w); }

	template <typename ...A>
	C(I <(1 + sizeof...(Rest))>, A &&... a)
		: Head(((A&&)(a))...) {}

	template <typename ...A>
	C(A &&... a)
		: Tail(((A&&)(a))...) {}

	operator First no_copy() const { return Head; }
	operator First ref() { return Head; }

	template <typename T>
	operator T no_copy() const { return Tail; }
	template <typename T>
	operator T ref() { return Tail; }
};

template <bool td, bool tdc, typename ...Types>
struct D;

template <typename ...Types>
struct D <true, true, Types...> {
	using storage = C <true, Types...>;

	s64 w;
	storage c;

	s64 which() const { return (w == 0) ? sizeof...(Types) : w; }

	D() = default;

	template <typename i, typename ...A>
	D(i, A &&... a) : w(i::value), c(i{}, ((A&&)(a))...) {}

	template <typename T>
	operator T no_copy() const { return c; }
	template <typename T>
	operator T ref() { return c; }
};

template <typename ...Types>
struct D <false, true, Types...> {
	using storage = C <false, Types...>;

	s64 w;
	storage c;

	s64 which() const { return (w == 0) ? sizeof...(Types) : w; }

	D() = default;

	D(D no_copy) = default;
	D(D ref) = default;
	D(D&&) = default;
	D ref operator = (D no_copy) = default;
	D ref operator = (D ref) = default;
	D ref operator = (D&&) = default;

	~D() { if (w != 0) c.destruct(w); }

	template <typename i, typename ...A>
	D(i, A &&... a) : w(i::value), c(i{}, ((A&&)(a))...) {}

	template <typename T> operator T no_copy() const { return c; }
	template <typename T> operator T& () { return c; }
};

template <typename ...Types>
struct D <true, false, Types...> {
	using storage = C <true, Types...>;

	s64 w;
	storage c;

	s64 which() const { return w; }

	using di = get_di <Types..., void>;

	D() : D(typename di::type{}) { ; }

	template <typename i, typename ...A>
	D(i, A &&... a) : w(i::value), c(i{}, ((A&&)(a))...) {}

	template <typename T> operator T no_copy() const { return c; }
	template <typename T> operator T ref() { return c; }
};

template <typename ...Types>
struct D <false, false, Types...> {
	using storage = C <false, Types...>;

	s64 w;
	storage c;

	s64 which() const { return w; }

	using di = get_di <Types..., void>;

	D() : D(typename di::T{}) { ; }

	D(D no_copy) = default;
	D(D ref) = default;
	D(D&&) = default;
	D ref operator = (D no_copy) = default;
	D ref operator = (D ref) = default;
	D ref operator = (D&&) = default;

	~D() { c.destruct(w); }

	template <typename i, typename ...A>
	D(i, A &&... a) : w(i::value), c(i{}, ((A&&)(a))...) {}

	template <typename T> operator T no_copy() const { return c; }
	template <typename T> operator T ref() { return c; }
};

template <typename ...Types>
using D_t = D <(__is_trivially_destructible(Types) && ...), (__is_trivially_constructible(remove_cvref_t<Types>) && ...), Types...>;

template <bool dc>
struct DC;

template<>
struct DC <true> {
	DC() = default;
	DC(void*) { ; }
};

template<>
struct DC <false> {
	DC() = delete;
	DC(void*) { ; }
};

template <typename ...Types>
using DC_t = DC <(is_constructible<Types> || ...)>;

export {
	template <typename ...Types>
	struct variant : DC_t <Types...> {
		using dc = DC_t <Types...>;
		using storage = D_t <Types...>;

		storage Storage;

		template <typename T>
		using i_at_t = i_at_t <T, Types..., void>;

		s64 which() const { return Storage.which(); }

		variant() = default;

		template <typename T, typename i = i_at_t<T>>
		variant(T&& x) : dc({}), Storage(i{}, (T&&)x) { ; }

		template <typename T, typename = i_at_t<T>>
		explicit operator T no_copy() const { return Storage; }
		template <typename T, typename = i_at_t<T>>
		explicit operator T ref() { return Storage; }

		template <typename T, typename = i_at_t<T>>
		variant ref operator=(T&& x) { return (*this = variant((T&&)x)); }

		template <s64 i, typename ...A>
		variant(index <i>, A &&... a) : dc({}), Storage(I <i>{}, ((A&&)(a))...) {  }
	};
}

LSTD_END_NAMESPACE
