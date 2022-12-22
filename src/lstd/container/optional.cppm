module;

#include "../common.h"

export module lstd.optional;

export import lstd.variant;

LSTD_BEGIN_NAMESPACE

export {
	template <typename T>
	struct optional : variant<null_t, T> {
		optional() : variant(null) {}

		template <typename i = i_at_t<T>>
		optional(T&& x) : variant((T&&)x) {}

		template <typename = i_at_t<T>>
		optional ref operator=(T&& x) { return (*this = optional((T&&)x)); }

		bool has_value() const { return this->which() != 0; }

		explicit operator bool() const { return has_value(); }

		const T* operator->() const { assert(has_value()); return &((T)this); }
		T* operator->() { assert(has_value()); return &((T)this); }
		T no_copy operator*() const& { assert(has_value()); return ((T)this); }
		T ref operator*()& { assert(has_value()); return ((T)this); }
		const T&& operator*() const&& { assert(has_value()); return (T&&)((T)this); }
		T&& operator*()&& { assert(has_value()); return (T&&)((T)this); }
	};
}

LSTD_END_NAMESPACE
