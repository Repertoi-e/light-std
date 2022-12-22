#pragma once

LSTD_BEGIN_NAMESPACE

//
// And this one is inspired from Python's enumerate().
// For each loop which also gives the index of the element.
// 
// Example usage:
//
//    For_enumerate(data) {
//        other_data[it_index] = it + 1;
//    }
//
// .. which is the same as:
//
//    For(range(data.Count)) {
//        other_data[it] = data[it] + 1;
//    }
//
// Might not look much shorter but you don't a separate
// variable if you use data[it] more than once.
// It's just a convenience.
//
// You can change the names of the internal
// variables by using _For_enumerate_as_.
//
#define For_enumerate_as(it_index, it, in) for (auto [it_index, it] : LSTD_NAMESPACE::enumerate_impl(in))
#define For_enumerate(in) For_enumerate_as(it_index, it, in)

template <typename T, typename TIter = decltype(T().begin()), typename = decltype(T().end()) >
auto enumerate_impl(const T &in) {
    struct iterator {
        s64 I;
        TIter Iter;

        bool operator!=(const iterator &other) const { return Iter != other.Iter; }
        void operator++() { ++I, ++Iter; }

        struct dereference_result {
            s64 Index;
            decltype(*(TIter())) Value;
        };

        auto operator*() const {
            return dereference_result{I, *Iter};
        }
    };

    struct iterable_wrapper {
        T Iterable;

        auto begin() { return iterator{0, Iterable.begin()}; }
        auto end() { return iterator{0, Iterable.end()}; }
    };

    return iterable_wrapper{(T &&) in};
}

LSTD_END_NAMESPACE
