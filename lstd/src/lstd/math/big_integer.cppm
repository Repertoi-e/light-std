module;

#include "../memory/string_builder.h"

export module lstd.big_integer;

//
// Provides arbitrary precision integer math.
// Supports integers on the stack (with compile-time capacity)
// and a dynamic integer which grows as it needs more range.
//

LSTD_BEGIN_NAMESPACE

export {
    //
    // We use u32s for storing single digits, each digit has range from -2^30 to 2^30.
    // When implementing operations we use grade school math,
    // but treat each digit of the integer as in base 2^30 instead of base 10.
    //
    // Since u32 digits are big digits, we call them bigits.
    //
    namespace big_integer_implementation {
    using digit  = u32;
    using sdigit = s32;

    using double_digit  = u64;
    using sdouble_digit = s64;

    constexpr u32 SHIFT  = 30;
    constexpr digit BASE = (digit) 1 << SHIFT;
    constexpr digit MASK = BASE - 1;
    }  // namespace big_integer_implementation

    struct big_integer_nan {
        static constexpr s64 Capacity = 0;
        static constexpr s64 Size     = S64_MAX;

        stack_array<big_integer_implementation::digit, 1> Bigits{};

        constexpr bool ensure_bigits(s64 n) { return false; }
    };

    // This is a special value which is returned when certain errors in arithmetic occur: overflows/underflows, division by zero, etc.
    constexpr big_integer_nan NAN = {};

    // These are the requirements for an object to be considered a big integer and on which operations are compiled.
    // We do this in order to reuse code. That way we can have a big integer on the stack and a big integer which grows dynamically,
    // operations are implemented transparently despite the underlying storage. Operations also work between stack and dynamic big integers for example.
    // This is like compile-time inheritance but it doesn't have runtime overhead.
    // Kudos for C++20 concepts.
    //
    // We also use a similar trick in "array_like.h". :CodeReusability:
    template <typename T>
    concept is_big_integer = requires(T t) {
        // Storage for digits in base 2^30.
        // Index 0 being the least significant one.
        {t.Bigits};
        {t.Bigits[0]};
        {types::is_same<decltype(t.Bigits[0]), big_integer_implementation::digit> == true};

        // A *signed* integer which stores how many digits from _t.Bigits_ are used by the object.
        // When the integer is negative, t.Size < 0, so negating is as simple as setting t.Size to -t.Size.
        // This is very similar to CPython's longobject.
        {t.Size};

        // Should ensure that there are at least 'n' bigits available in the storage.
        // Should trip and return false on failure.
        {t.ensure_bigits(0)};
    };

    template <is_big_integer T>
    struct div_result {
        T Quot, Rem;
    };

    namespace big_integer_implementation {
    // Assign an integral value (s8, s16, s32, s64, u8, u16, u32, u64, s128, u128, other big integers,
    // as well as user integral types - declared with DECLARE_INTEGRAL or DECLARE_INTEGRAL_PAIR) to a big integer _b_.
    constexpr bool assign(is_big_integer auto &b, const types::is_integral auto &v);
    }  // namespace big_integer_implementation

    template <s64 Capacity_>
    struct big_integer_on_the_stack {
        static constexpr s64 Capacity = Capacity_;
        static_assert(Capacity > 4, "For smaller values use s8, s16, s32, s64, s128. They are faster.");

        stack_array<big_integer_implementation::digit, Capacity> Bigits{};
        s64 Size = 0;

        constexpr big_integer_on_the_stack() {}
        constexpr big_integer_on_the_stack(big_integer_nan) : Size(S64_MAX) {}

        constexpr big_integer_on_the_stack(const types::is_integral auto &value) { big_integer_implementation::assign(*this, value); }

        auto &operator=(const types::is_integral auto &value) {
            big_integer_implementation::assign(*this, value);
            return *this;
        }

        // big_integer_on_the_stack doesn't allocate dynamic memory,
        // check out big_integer_dynamic for a type which grows automatically.
        constexpr bool ensure_bigits(s64 n) {
            if (n > Capacity) return false;
            return true;
        }

        //
        // We don't provide conversion operators to smaller integer values.
        // People who use this object can truncate the value themselves by looking at 'Bigits'.
        //
    };

    namespace types {
    template <s64 Capacity>
    struct is_integral_helper<big_integer_on_the_stack<Capacity>> : true_t {};
    }  // namespace types

    // @Speed Refactor code and make this "maybe_cast_to_big_integer"
    // to avoid casting between two different big integer types.
    // This might be a rare case but we can avoid it.

    template <is_big_integer Fallback>
    constexpr auto cast_to_same_big_integer(const types::is_integral auto x) {
        if constexpr (is_big_integer<decltype(x)>) {
            if constexpr (types::is_same<Fallback, types::remove_cvref_t<decltype(x)>>) {
                return x;
            } else {
                Fallback result;
                assign(result, x);
                return result;
            }
        } else {
            return Fallback(x);
        }
    }

    namespace big_integer_implementation {

    // x[0:m] and y[0:n] are digit vectors, LSD first, m >= n required.  x[0:n]
    // is modified in place, by adding y to it.  Carries are propagated as far as
    // x[m-1], and the remaining carry (0 or 1) is returned.
    constexpr digit v_iadd(digit *x, s64 m, digit *y, s64 n) {
        assert(m >= n);

        digit carry = 0;

        s64 i = 0;
        while (i < n) {
            carry += x[i] + y[i];
            x[i] = carry & MASK;
            carry >>= SHIFT;

            assert((carry & 1) == carry);
            ++i;
        }

        while (carry && i < m) {
            carry += x[i];
            x[i] = carry & MASK;
            carry >>= SHIFT;

            assert((carry & 1) == carry);
            ++i;
        }
        return carry;
    }

    // x[0:m] and y[0:n] are digit vectors, LSD first, m >= n required.  x[0:n]
    // is modified in place, by subtracting y from it.  Borrows are propagated as
    // far as x[m-1], and the remaining borrow (0 or 1) is returned.
    constexpr digit v_isub(digit *x, s64 m, digit *y, s64 n) {
        assert(m >= n);

        digit borrow = 0;

        s64 i = 0;
        while (i < n) {
            borrow = x[i] - y[i] - borrow;
            x[i]   = borrow & MASK;
            borrow >>= SHIFT;
            borrow &= 1;  // Keep only 1 sign bit

            ++i;
        }

        while (borrow && i < m) {
            borrow = x[i] - borrow;
            x[i]   = borrow & MASK;
            borrow >>= SHIFT;
            borrow &= 1;

            ++i;
        }
        return borrow;
    }

    // Multiply digit vector[0:m] by _d_ and return the carry (if any).
    // Store the result in z[0:m]. If z has enough space the caller can then store the carry in it.
    constexpr digit v_product(digit *z, const digit *a, s64 m, digit d) {
        double_digit carry = 0;

        For(range(m)) {
            carry += (double_digit) a[it] * d;

            z[it] = (digit) (carry & MASK);
            carry >>= SHIFT;
            assert(carry <= MASK);
        }

        return (digit) carry;
    }

    // Normalize (remove leading zeros from) from a big integer.
    constexpr void normalize(is_big_integer auto &v) {
        s64 j = abs(v.Size);
        s64 i = j;

        while (i > 0 && v.Bigits[i - 1] == 0) --i;
        if (i != j) v.Size = v.Size < 0 ? -i : i;
    }

    constexpr bool assign(is_big_integer auto &b, const types::is_integral auto &value) {
        b.Size = 0;

        if constexpr (is_big_integer<decltype(value)>) {
            if (value.Size == S64_MAX) {
                b = NAN;
            } else {
                s64 absSize = abs(value.Size);

                // Assign from another big integer
                if (!b.ensure_bigits(absSize)) {
                    b = NAN;
                    return false;
                }

                copy_elements(b.Bigits.Data, value.Bigits.Data, absSize);
                b.Size = value.Size;
            }
        } else {
            // Note: Big integers should have minimum storage of 5 u32s (bigger than u128).
            // We static_assert in the type itself for that.
            // For smaller integers you'd better be using the smaller intrinsic types..
            b.ensure_bigits(5);

            s64 size = 0;

            auto v = value;

            s32 sign = 1;
            if constexpr (types::is_signed_integral<decltype(v)>) {
                if (v < 0) {
                    v    = -v;
                    sign = -1;
                }
            }

            do {
                if constexpr (sizeof(v) == sizeof(u128)) {
                    // Handle 128 bit integers
                    b.Bigits[size++] = v.lo & MASK;
                } else {
                    // Handle 8, 16, 32, 64 bit integers
                    b.Bigits[size++] = v & MASK;
                }
                v >>= SHIFT;
            } while (v);

            b.Size = size * sign;
        }
        normalize(b);

        return true;
    }

    // Add the absolute values of two integers. Returns NAN on error.
    template <is_big_integer T>
    constexpr T x_add(const T &lhs, const T &rhs) {
        auto *a = &lhs, *b = &rhs;
        s64 sizea = abs(a->Size), sizeb = abs(b->Size);

        if (sizea < sizeb) {
            swap(a, b);
            swap(sizea, sizeb);
        }

        T result;
        if (!result.ensure_bigits(sizea + 1)) return (T) NAN;

        digit carry = 0;

        s64 i = 0;
        while (i < sizeb) {
            carry += a->Bigits[i] + b->Bigits[i];
            result.Bigits[i] = carry & MASK;
            carry >>= SHIFT;

            ++i;
        }

        while (i < sizea) {
            carry += a->Bigits[i];
            result.Bigits[i] = carry & MASK;
            carry >>= SHIFT;

            ++i;
        }
        result.Bigits[i] = carry;

        result.Size = sizea + 1;
        normalize(result);
        return result;
    }

    // Subtract the absolute values of two integers. Returns NAN on error.
    template <is_big_integer T>
    constexpr T x_sub(const T &lhs, const T &rhs) {
        auto *a = &lhs, *b = &rhs;
        s64 sizea = abs(a->Size), sizeb = abs(b->Size);

        s32 sign = 1;

        // Ensure a is the larger of the two
        if (sizea < sizeb) {
            sign = -1;
            swap(a, b);
            swap(sizea, sizeb);
        } else if (sizea == sizeb) {
            // Find highest digit where a and b differ
            s64 i = sizea;
            while (--i >= 0 && a->Bigits[i] == b->Bigits[i])
                ;

            if (i < 0) return T(0);

            if (a->Bigits[i] < b->Bigits[i]) {
                sign = -1;
                swap(a, b);
            }
            sizea = sizeb = i + 1;
        }

        T result;
        if (!result.ensure_bigits(sizea)) return (T) NAN;

        digit borrow = 0;

        s64 i = 0;

        while (i < sizeb) {
            borrow = a->Bigits[i] - b->Bigits[i] - borrow;

            result.Bigits[i] = borrow & MASK;
            borrow >>= SHIFT;
            borrow &= 1;  // Keep only one sign bit

            ++i;
        }

        while (i < sizea) {
            borrow = a->Bigits[i] - borrow;

            result.Bigits[i] = borrow & MASK;
            borrow >>= SHIFT;
            borrow &= 1;  // Keep only one sign bit

            ++i;
        }

        assert(borrow == 0);

        result.Size = sign * sizea;
        normalize(result);
        return result;
    }

    // Multiply the absolute values of two integers, Returns NAN on error.
    // Grade-school algorithm.
    template <is_big_integer T>
    constexpr T x_mul(const T &lhs, const T &rhs) {
        s64 sizea = abs(lhs.Size), sizeb = abs(rhs.Size);

        T result;
        if (!result.ensure_bigits(sizea + sizeb)) return (T) NAN;
        result.Size = sizea + sizeb;

        if (lhs == rhs) {
            // Efficient squaring per HAC, Algorithm 14.16:
            // http://www.cacr.math.uwaterloo.ca/hac/about/chap14.pdf
            // Gives slightly less than a 2x speedup when a == b,
            // via exploiting that each entry in the multiplication
            // pyramid appears twice (except for the sizea squares).

            For(range(sizea)) {
                double_digit f = lhs.Bigits[it];

                digit *pz    = const_cast<digit *>(result.Bigits.Data) + (it << 1);
                digit *pa    = const_cast<digit *>(lhs.Bigits.Data) + it + 1;
                digit *paend = const_cast<digit *>(lhs.Bigits.Data) + sizea;

                double_digit carry = *pz + f * f;

                *pz++ = (digit) (carry & MASK);

                carry >>= SHIFT;
                assert(carry <= MASK);

                // Now f is added in twice in each column of the
                // pyramid it appears. Same as adding f<<1 once.
                f <<= 1;
                while (pa < paend) {
                    carry += *pz + *pa++ * f;
                    *pz++ = (digit) (carry & MASK);
                    carry >>= SHIFT;
                    assert(carry <= (MASK << 1));
                }

                if (carry) {
                    carry += *pz;
                    *pz++ = (digit) (carry & MASK);
                    carry >>= SHIFT;
                }

                if (carry) *pz += (digit) (carry & MASK);

                assert((carry >> SHIFT) == 0);
            }
        } else {
            // lhs != rhs

            For(range(sizea)) {
                double_digit f = lhs.Bigits[it];

                digit *pz    = const_cast<digit *>(result.Bigits.Data) + it;
                digit *pb    = const_cast<digit *>(rhs.Bigits.Data);
                digit *pbend = const_cast<digit *>(rhs.Bigits.Data) + sizeb;

                double_digit carry = 0;

                while (pb < pbend) {
                    carry += *pz + *pb++ * f;
                    *pz++ = (digit) (carry & MASK);
                    carry >>= SHIFT;
                    assert(carry <= MASK);
                }

                if (carry) *pz += (digit) (carry & MASK);

                assert((carry >> SHIFT) == 0);
            }
        }

        normalize(result);
        return result;
    }

    // For int multiplication, use the O(N**2) school algorithm unless
    // both operands contain more than KARATSUBA_CUTOFF digits.
    constexpr s64 KARATSUBA_CUTOFF        = 70;
    constexpr s64 KARATSUBA_SQUARE_CUTOFF = 2 * KARATSUBA_CUTOFF;

    // A helper for Karatsuba multiplication (k_mul).
    // Takes an int "n" and an integer "size" representing the place to
    // split, and sets low and high such that abs(n) == (high << size) + low,
    // viewing the shift as being by digits.  The sign bit is ignored, and
    // the return values are >= 0.
    // Returns 0 on success, -1 on failure.
    template <is_big_integer T>
    constexpr bool kmul_split(const T &n, s64 size, T *high, T *low) {
        s64 sizen = abs(n.Size);

        s64 sizelo = min(sizen, size);
        s64 sizehi = sizen - sizelo;

        if (!high->ensure_bigits(sizehi)) return false;
        if (!low->ensure_bigits(sizelo)) return false;

        copy_elements(low->Bigits.Data, n.Bigits.Data, sizelo);
        copy_elements(high->Bigits.Data, n.Bigits.Data + sizelo, sizehi);
        high->Size = sizehi;
        low->Size  = sizelo;

        normalize(*high);
        normalize(*low);

        return true;
    }

    template <is_big_integer T>
    constexpr T k_mul(const T &lhs, const T &rhs);

    // b has at least twice the digits of a, and a is big enough that Karatsuba
    // would pay off *if* the inputs had balanced sizes.  View b as a sequence
    // of slices, each with a->ob_size digits, and multiply the slices by a,
    // one at a time.  This gives k_mul balanced inputs to work with, and is
    // also cache-friendly (we compute one double-width slice of the result
    // at a time, then move on, never backtracking except for the helpful
    // single-width slice overlap between successive partial sums).
    template <is_big_integer T>
    constexpr T k_lopsided_mul(const T *a, const T *b) {
        s64 sizea = abs(a->Size), sizeb = abs(b->Size);

        assert(sizea > KARATSUBA_CUTOFF);
        assert(2 * sizea <= sizeb);

        T result;
        if (!result.ensure_bigits(sizea + sizeb)) return (T) NAN;
        result.Size = sizea + sizeb;

        // Successive slices of b are copied into bslice
        T bslice;
        if (!bslice.ensure_bigits(sizea)) return (T) NAN;
        bslice.Size = sizea;

        s64 nbdone = 0;  // # of b digits already multiplied
        while (sizeb > 0) {
            s64 nbtouse = min(sizeb, sizea);

            // Multiply the next slice of b by a

            copy_elements(bslice.Bigits.Data, b->Bigits.Data + nbdone, nbtouse);
            bslice.Size = nbtouse;

            T product = k_mul(*a, bslice);
            if (product == NAN) return (T) NAN;

            // Add into result
            v_iadd(result.Bigits.Data + nbdone, result.Size - nbdone, product.Bigits.Data, product.Size);

            sizeb -= nbtouse;
            nbdone += nbtouse;
        }

        normalize(result);
        return result;
    }

    // Multiply the absolute values of two integers, Returns NAN on error.
    // Karatsuba multiplication. See Knuth Vol. 2 Chapter 4.3.3 (Pp. 294-295).
    template <is_big_integer T>
    constexpr T k_mul(const T &lhs, const T &rhs) {
        auto *a = &lhs, *b = &rhs;
        s64 sizea = abs(a->Size), sizeb = abs(b->Size);

        // (ah*X+al)(bh*X+bl) = ah*bh*X*X + (ah*bl + al*bh)*X + al*bl
        // Let k = (ah+al)*(bh+bl) = ah*bl + al*bh  + ah*bh + al*bl
        // Then the original product is
        //     ah*bh*X*X + (k - ah*bh - al*bl)*X + al*bl
        // By picking X to be a power of 2, "*X" is just shifting, and it's
        // been reduced to 3 multiplies on numbers half the size.

        // We want to split based on the larger number; fiddle so that bis largest.
        if (sizea > sizeb) {
            swap(a, b);
            swap(sizea, sizeb);
        }

        // @Speed We treat big integers as immutable. That has serious performance implications.
        // Also the (a == b) comparison here..

        s64 i = (*a == *b) ? KARATSUBA_SQUARE_CUTOFF : KARATSUBA_CUTOFF;
        if (sizea <= i) {
            if (sizea == 0) {
                return T(0);
            } else {
                return x_mul(*a, *b);
            }
        }

        // If a is small compared to b, splitting on b gives a degenerate
        // case with ah==0, and Karatsuba may be (even much) less efficient
        // than "grade school" then.  However, we can still win, by viewing
        // b as a string of "big digits", each of width a->Size.  That
        // leads to a sequence of balanced calls to k_mul.

        if (2 * sizea <= sizeb) return k_lopsided_mul(a, b);

        T ah_, al_;

        // Split a & b into hi & lo pieces
        s64 shift = sizeb >> 1;
        if (!kmul_split(*a, shift, &ah_, &al_)) return (T) NAN;

        assert(ah_.Size > 0);  // The split isn't degenerate

        T bh_, bl_;

        T *ah = &ah_, *al = &al_;
        T *bh, *bl;

        if (*a == *b) {
            bh = ah;
            bl = al;
        } else {
            if (!kmul_split(*b, shift, &bh_, &bl_)) return (T) NAN;

            bh = &bh_;
            bl = &bl_;
        }

        // The plan:
        // 1. Allocate result space (sizea + sizeb digits: that's always enough).
        // 2. Compute ah*bh, and copy into result at 2*shift.
        // 3. Compute al*bl, and copy into result at 0. Note that this can't overlap with #2.
        // 4. Subtract al*bl from the result, starting at shift. This may
        //    underflow (borrow out of the high digit), but we don't care:
        //    we're effectively doing unsigned arithmetic mod
        //    BASE**(sizea + sizeb), and so long as the *final* result fits,
        //    borrows and carries out of the high digit can be ignored.
        // 5. Subtract ah*bh from the result, starting at shift.
        // 6. Compute (ah+al)*(bh+bl), and add it into the result starting at shift.

        // 1.
        T result;
        if (!result.ensure_bigits(sizea + sizeb)) return (T) NAN;
        result.Size = sizea + sizeb;

#ifdef Py_DEBUG
        // Fill with trash, to catch reference to uninitialized digits.
        // fill_memory(result.Bigits.Data, 0xDF, result.Size * sizeof(digit));
#endif
        T t1;

        // 2. t1 <- ah*bh, and copy into high digits of result
        if ((t1 = k_mul(*ah, *bh)) == NAN) return (T) NAN;

        assert(t1.Size >= 0);
        assert(2 * shift + t1.Size <= result.Size);
        copy_elements(result.Bigits.Data + 2 * shift, t1.Bigits.Data, t1.Size);

        // Zero-out the digits higher than the ah*bh copy
        i = result.Size - 2 * shift - t1.Size;
        if (i) fill_memory(result.Bigits.Data + 2 * shift + t1.Size, 0, i * sizeof(digit));

        T t2;

        // 3. t2 <- al*bl, and copy into the low digits
        if ((t2 = k_mul(*al, *bl)) == NAN) return (T) NAN;

        assert(t2.Size >= 0);
        assert(t2.Size <= 2 * shift); /* no overlap with high digits */
        copy_elements(result.Bigits.Data, t2.Bigits.Data, t2.Size);

        // Zero out remaining digits
        i = 2 * shift - t2.Size;  // number of uninitialized digits
        if (i) zero_memory(result.Bigits.Data + t2.Size, i * sizeof(digit));

        // 4 & 5. Subtract ah*bh (t1) and al*bl (t2).
        // We do al*bl first because it's fresher in cache.
        i = result.Size - shift;  // # digits after shift
        v_isub(result.Bigits.Data + shift, i, t2.Bigits.Data, t2.Size);

        v_isub(result.Bigits.Data + shift, i, t1.Bigits.Data, t1.Size);

        // 6. t3 <- (ah+al)(bh+bl), and add into result
        if ((t1 = x_add(*ah, *al)) == NAN) return (T) NAN;

        ah = al = null;

        if (*a == *b) {
            t2 = t1;
        } else if ((t2 = x_add(*bh, *bl)) == NAN) {
            return (T) NAN;
        }
        bh = bl = null;

        T t3 = k_mul(t1, t2);
        if (t3 == NAN) return (T) NAN;

        assert(t3.Size >= 0);

        // Add t3. It's not obvious why we can't run out of room here.
        // Let f(x) mean the floor of x and c(x) mean the ceiling of x.  Some facts
        // to start with:
        // 1. For any integer i, i = c(i/2) + f(i/2). In particular, sizeb = c(sizeb/2) + f(sizeb/2).
        // 2. shift = f(sizeb/2)
        // 3. sizea <= sizeb
        // 4. Since we call k_lopsided_mul if sizea*2 <= sizeb, sizea*2 > sizeb in this
        //    routine, so sizea > sizeb/2 >= f(sizeb/2) in this routine.
        //
        // We allocated sizea + sizeb result digits, and add t3 into them at an offset
        // of shift.  This leaves sizea+sizeb-shift allocated digit positions for t3
        // to fit into, = (by #1 and #2) sizea + f(sizeb/2) + c(sizeb/2) - f(sizeb/2) =
        // sizea + c(sizeb/2) available digit positions.
        //
        // bh has c(sizeb/2) digits, and bl at most f(size/2) digits.  So bh+hl has
        // at most c(sizeb/2) digits + 1 bit.
        //
        // If sizea == sizeb, ah has c(sizeb/2) digits, else ah has at most f(sizeb/2)
        // digits, and al has at most f(sizeb/2) digits in any case.  So ah+al has at
        // most (sizea == sizeb ? c(sizeb/2) : f(sizeb/2)) digits + 1 bit.
        //
        // The product (ah+al)*(bh+bl) therefore has at most
        //     c(sizeb/2) + (sizea == sizeb ? c(sizeb/2) : f(sizeb/2)) digits + 2 bits
        // and we have sizea + c(sizeb/2) available digit positions.  We need to show
        // this is always enough.  An instance of c(sizeb/2) cancels out in both, so
        // the question reduces to whether sizea digits is enough to hold
        // (sizea == sizeb ? c(sizeb/2) : f(sizeb/2)) digits + 2 bits.
        //
        // If sizea < sizeb, then we're asking whether sizea digits >= f(sizeb/2) digits + 2 bits.
        // By #4, sizea is at least f(sizeb/2)+1 digits, so this in turn reduces to whether 1
        // digit is enough to hold 2 bits.  This is so since SHIFT=15 >= 2.
        //
        // If sizea == sizeb, then we're asking whether sizeb digits is enough to hold
        // c(sizeb/2) digits + 2 bits, or equivalently (by #1) whether f(sizeb/2) digits
        // is enough to hold 2 bits.  This is so if sizeb >= 2, which holds because
        // sizeb >= KARATSUBA_CUTOFF >= 2.
        //
        // Note that since there's always enough room for (ah+al)*(bh+bl), and that's
        // clearly >= each of ah*bh and al*bl, there's always enough room to subtract
        // ah*bh and al*bl too.
        v_iadd(result.Bigits.Data + shift, i, t3.Bigits.Data, t3.Size);

        normalize(result);
        return result;
    }

    // Divide long pin, w/ size digits, by non-zero digit n, storing quotient
    // in pout, and returning the remainder.  pin and pout point at the LSD.
    // It's OK for pin == pout on entry.
    constexpr digit inplace_divrem1(digit *pout, const digit *pin, s64 size, digit n) {
        double_digit rem = 0;

        assert(n > 0 && n <= MASK);
        pin += size;
        pout += size;
        while (--size >= 0) {
            digit hi;
            rem = (rem << SHIFT) | *--pin;

            *--pout = hi = (digit) (rem / n);
            rem -= (double_digit) hi * n;
        }
        return (digit) rem;
    }

    template <is_big_integer T>
    struct div1_result {
        T Quot;
        digit Rem;
    };

    // Divide an integer by a digit, returning both the quotient
    // and the remainder. The sign of _a_ is ignored.
    // Returns NAN on error.
    template <is_big_integer T>
    constexpr auto divrem1(const T &a, digit n) {
        if (a == NAN) return div1_result<T>{a, 0};

        assert(n > 0 && n <= MASK);

        s64 size = abs(a.Size);

        T result;
        if (!result.ensure_bigits(size)) return div1_result<T>{NAN, 0};
        result.Size = size;

        digit rem = inplace_divrem1(result.Bigits.Data, a.Bigits.Data, size, n);
        normalize(result);

        return div1_result<T>{result, rem};
    }

    template <is_big_integer T>
    constexpr auto x_divrem(const T &a, const T &b) {
        // We follow Knuth [The Art of Computer Programming, Vol. 2 (3rd edn.), section 4.3.1, Algorithm D].
        // This divides an n-word dividend by an m-word divisor
        // and gives an n-m+1-word quotient and m-word remainder.

        //
        // https://skanthak.homepage.t-online.de/division.html
        // https://surface.syr.edu/cgi/viewcontent.cgi?article=1162&context=eecs_techreports
        //

        s64 sizea = abs(a.Size), sizeb = abs(b.Size);

        T na, nb;  // Normalized form of _a_ and _b_; _na_ also stores the normalized remainder in the end.

        // Normalize by shifting _b_ left just enough so that its high-order
        // bit is on, and shift _a_ left the same amount. We may have to append a
        // high-order digit on the dividend; we do that unconditionally.

        // This counts the number of leading zeros
        auto s = SHIFT - (msb(b.Bigits[sizeb - 1]) + 1);

        assert(s >= 0 && s <= 31);

        na = lshift(a, s);
        nb = lshift(b, s);

        // No need to check ub, it can't overflow by definition of _s_
        if (na == NAN) {
            assert(false && "Overflow");
            return div_result<T>{NAN, NAN};
        }

        // If _a_ overflows..
        sizea = abs(na.Size);

        if (na.Bigits[sizea - 1] >= nb.Bigits[sizeb - 1]) {
            bool ensured = na.ensure_bigits(sizea + 1);
            assert(ensured && "Overflow");

            na.Bigits[sizea] = 0;
            sizea += 1;
        }

        s64 j = sizea - sizeb;

        T q;  // _q_ holds the quotient
        q.ensure_bigits(j);
        q.Size = j;

        while (j--) {
            digit atop = na.Bigits[j + sizeb];
            assert(atop <= nb.Bigits[sizeb - 1]);

            // Compute estimate qhat; may overestimate by 1 (rare).
            double_digit vv = ((double_digit) atop << SHIFT) | na.Bigits[j + sizeb - 1];

            digit qhat = (digit) (vv / nb.Bigits[sizeb - 1]);
            digit rhat = (digit) (vv - qhat * (double_digit) nb.Bigits[sizeb - 1]);

            while ((double_digit) nb.Bigits[sizeb - 2] * qhat > (((double_digit) rhat << SHIFT) | na.Bigits[j + sizeb - 2])) {
                --qhat;
                rhat += nb.Bigits[sizeb - 1];
                if (rhat >= BASE) break;
            }
            assert(qhat <= BASE);

            // Multiply and subtract (qhat*nb[0:sizeb] from na[j:j+sizeb+1])
            sdouble_digit z;
            sdigit zhi = 0;

            For(range(sizeb)) {
                // Invariants: -BASE <= -qhat <= zhi <= 0;
                //             -BASE * qhat <= z < BASE

                z = (sdigit) na.Bigits[it + j] + zhi -
                    (sdouble_digit) qhat * (sdouble_digit) nb.Bigits[it];

                na.Bigits[it + j] = z & MASK;
                zhi               = (sdigit) (z >> SHIFT);
            }

            assert((sdigit) atop + zhi == -1 || (sdigit) atop + zhi == 0);

            // If we subtracted too much, add back.
            if ((sdigit) atop + zhi < 0) [[unlikely]] {
                digit carry = 0;
                For(range(sizeb)) {
                    carry += na.Bigits[it + j] + nb.Bigits[it];
                    na.Bigits[it + j] = carry & MASK;
                    carry >>= SHIFT;
                }
                --qhat;
            }

            // Store quotient digit
            assert(qhat < BASE);
            q.Bigits[j] = qhat;
        }

        // Unnormalize the remainder, which was stored in _na_ during the computation.
        na.Size = sizeb;
        T r     = rshift(na, s);

        normalize(q);
        normalize(r);

        return div_result<T>{q, r};
    }

    // divmod is NOT the same as divrem.
    // divrem gives the remainder after division of |a| by |b|, with the sign of a.
    //
    // a mod b == a - b * floor(a/b)
    // a rem b == a - b * trunc(a/b) (if trunc truncates toward zero).
    //
    // Some examples:
    //   a           b      a mod b    a rem b
    //   13          10      3          3
    //  -13          10      7         -3
    //   13         -10     -7          3
    //  -13         -10     -3         -3
    //
    // divmod is defined in terms of divrem, but we adjust the remainder.
    // Operator % is defined in terms of divmod.
    //
    // Both functions return NAN on error (e.g. division by zero).
    template <is_big_integer T>
    constexpr auto divrem(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);

        if (lhs == NAN) return div_result<T>{NAN, NAN};
        if (rhs == NAN) return div_result<T>{NAN, NAN};

        s64 sizea = abs(lhs.Size), sizeb = abs(rhs.Size);

        div_result<T> result;  // _lhs_ determines the resulting types

        if (!sizeb) {
            assert(false && "Division by zero");
            return div_result<T>{NAN, NAN};
        }

        if (sizea < sizeb || (sizea == sizeb && (lhs.Bigits[sizea - 1] < rhs.Bigits[sizeb - 1]))) {
            // |lhs| < |rhs|
            result.Quot = 0;
            result.Rem  = lhs;
            return result;
        }

        //
        // |lhs| >= |rhs|
        //

        if (sizeb == 1) {
            auto [div, rem] = big_integer_implementation::divrem1(lhs, rhs.Bigits[0]);
            result.Quot     = div;
            result.Rem      = rem;
        } else {
            result = x_divrem(lhs, rhs);
        }

        if (result.Quot == NAN) return result;

        // Set the signs.
        // The quotient z has the sign of lhs * rhs;
        // the remainder r has the sign of lhs, so lhs = rhs * z + r.
        if ((lhs.Size < 0) != (rhs.Size < 0)) {
            result.Quot.Size = -result.Quot.Size;
        }
        if (lhs.Size < 0 && result.Rem.Size != 0) {
            result.Rem.Size = -result.Rem.Size;
        }
        return result;
    }

    template <is_big_integer T>
    constexpr auto lshift(const T &lhs, s64 n) {
        if (lhs == NAN) return (T) NAN;

        if (n == 0) return lhs;
        if (lhs == T(0)) return T(0);

        s64 wordshift = n / SHIFT;
        u32 remshift  = n % SHIFT;

        // This version due to Tim Peters

        s64 oldSize = abs(lhs.Size);
        s64 newSize = oldSize + wordshift;

        if (remshift) ++newSize;

        T result;
        if (!result.ensure_bigits(newSize)) return (T) NAN;

        if (lhs.Size < 0) {
            result.Size = -newSize;
        } else {
            result.Size = newSize;
        }

        For(range(wordshift)) {
            result.Bigits[it] = 0;
        }

        s64 accum = 0;

        s64 i = wordshift;
        For_as(j, range(oldSize)) {
            accum |= (double_digit) lhs.Bigits[j] << remshift;
            result.Bigits[i] = (digit) (accum & MASK);
            accum >>= SHIFT;

            ++i;
        }

        if (remshift) {
            result.Bigits[newSize - 1] = (digit) accum;
        } else {
            assert(!accum);
        }

        normalize(result);
        return result;
    }

    template <is_big_integer T>
    constexpr T invert(const T &lhs) {
        if (lhs == NAN) return (T) NAN;

        T result    = lhs + 1;
        result.Size = -result.Size;
        return result;
    }

    template <is_big_integer T>
    constexpr T rshift(const T &lhs, s64 n) {
        if (lhs == NAN) return (T) NAN;

        if (n == 0) return lhs;
        if (lhs == 0) return T(0);

        s64 wordshift = n / SHIFT;
        u32 remshift  = n % SHIFT;

        if (lhs.Size < 0) {
            // Right shifting negative numbers is harder
            return invert(rshift(invert(lhs), n));
        } else {
            s64 newSize = lhs.Size - wordshift;
            if (newSize <= 0) return T(0);

            s64 hishift  = SHIFT - remshift;
            digit lomask = (1ul << hishift) - 1;
            digit himask = MASK ^ lomask;

            T result;
            if (!result.ensure_bigits(newSize)) return (T) NAN;
            result.Size = newSize;

            s64 i = 0, j = wordshift;
            while (i < newSize) {
                result.Bigits[i] = (lhs.Bigits[j] >> remshift) & lomask;
                if (i + 1 < newSize) {
                    result.Bigits[i] |= (lhs.Bigits[j + 1] << hishift) & himask;
                }
                ++i, ++j;
            }
            normalize(result);
            return result;
        }
    }

    // Compute two's complement of digit vector a[0:m], writing result to
    // z[0:m]. The digit vector a need not be normalized, but should not
    // be entirely zero. a and z may point to the same digit vector. */
    constexpr void v_complement(digit *z, digit *a, s64 m) {
        digit carry = 1;
        For(range(m)) {
            carry += a[it] ^ MASK;
            z[it] = carry & MASK;
            carry >>= SHIFT;
        }
        assert(carry == 0);
    }

    // _op_ is one of the following: '&', '|', '^'
    template <is_big_integer T>
    T bitwise(const T &lhs, byte op, const T &rhs) {
        // Bitwise operations for negative numbers operate as though
        // on a two's complement representation. So convert arguments
        // from sign-magnitude to two's complement, and convert the
        // result back to sign-magnitude at the end.

        s64 sizea = abs(lhs.Size), sizeb = abs(rhs.Size);
        bool negLhs = lhs.Size < 0, negRhs = rhs.Size < 0;

        T aComp, bComp;

        T *a = &lhs, *b = &rhs;

        if (negLhs) {
            bool ensured = aComp.ensure_bigits(sizea);
            assert(ensured);  // Sanity
            aComp.size = sizea;

            v_complement(aComp.Bigits, lhs.Bigits, sizea);
            a = &aComp;
        }

        if (negRhs) {
            bool ensured = bComp.ensure_bigits(sizeb);
            assert(ensured);  // Sanity
            bComp.size = sizeb;

            v_complement(bComp.Bigits, rhs.Bigits, sizeb);
            b = &bComp;
        }

        // Swap a and b if necessary to ensure sizea >= sizeb
        if (sizea < sizeb) {
            swap(a, b);
            swap(sizea, sizeb);
            swap(negLhs, negRhs);
        }

        s64 sizez;
        bool negZ;

        // JRH: The original logic here was to allocate the result value (z)
        // as the longer of the two operands. However, there are some cases
        // where the result is guaranteed to be shorter than that: AND of two
        // positives, OR of two negatives: use the shorter number.  AND with
        // mixed signs: use the positive number.  OR with mixed signs: use the
        // negative number.
        if (op == '&') {
            negZ  = negLhs & negRhs;
            sizez = negRhs ? sizea : sizeb;
        } else if (op == '|') {
            negZ  = negLhs | negRhs;
            sizez = negRhs ? sizeb : sizea;
        } else if (op == '^') {
            negZ  = negLhs ^ negRhs;
            sizez = sizea;
        } else {
            assert(false);
        }

        T result;

        // We allow an extra digit if z is negative, to make sure that
        // the final two's complement of z doesn't overflow.
        s64 rsize = sizea + (negZ ? 1 : 0);
        if (!ensure_bigits(result, rsize)) return (T) NAN;
        result.Size = rsize;

        s64 i = 0;

        // Compute digits for overlap of a and b
        if (op == '&') {
            while (i < sizeb) {
                result.Bigits[i] = a->Bigits[i] & b->Bigits[i];
                ++i;
            }
        } else if (op == '|') {
            while (i < sizeb) {
                result.Bigits[i] = a->Bigits[i] | b->Bigits[i];
                ++i;
            }
        } else if (op == '^') {
            while (i < sizeb) {
                result.Bigits[i] = a->Bigits[i] ^ b->Bigits[i];
                ++i;
            }
        } else {
            assert(false);
        }

        // Copy any remaining digits of a, inverting if necessary
        if (op == '^' && negRhs) {
            while (i < sizez) {
                result.Bigits[i] = a->Bigits[i] ^ MASK;
                ++i;
            }
        } else if (i < sizez) {
            copy_elements(&result.Bigits[i], &a->Bigits[i], sizez - i);
        }

        // Complement result if negative
        if (negZ) {
            result.Size          = -result.Size;
            result.Bigits[sizez] = MASK;
            v_complement(result.Bigits, result.Bigits, sizez + 1);
        }

        normalize(result);
        return result;
    }

    }  // namespace big_integer_implementation

    using big_integer_implementation::divrem;
    using big_integer_implementation::invert;

    // divmod is NOT the same as divrem.
    // divrem gives the remainder after division of |a| by |b|, with the sign of a.
    //
    // a mod b == a - b * floor(a/b)
    // a rem b == a - b * trunc(a/b) (if trunc truncates toward zero).
    //
    // Some examples:
    //   a           b      a mod b    a rem b
    //   13          10      3          3
    //  -13          10      7         -3
    //   13         -10     -7          3
    //  -13         -10     -3         -3
    //
    // divmod is defined in terms of divrem, but we adjust the remainder.
    // Operator % is defined in terms of divmod.
    //
    // Both functions return NAN on error (e.g. division by zero).
    template <is_big_integer T>
    constexpr auto divmod(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);

        if (lhs == NAN) return div_result<T>{NAN, NAN};
        if (rhs == NAN) return div_result<T>{NAN, NAN};

        auto [div, mod] = divrem(lhs, rhs);

        // To get from rem to mod, we have to add rhs if lhs and rhs
        // have different signs. We then subtract one from the 'div'
        // part of the outcome to keep the invariant intact.
        if ((mod.Size < 0 && rhs.Size > 0) || (mod.Size > 0 && rhs.Size < 0)) {
            return div_result<T>{div - 1, rhs + mod};
        } else {
            return div_result<T>{div, mod};
        }
    }

    // if a < b, returns a negative number
    // if a == b, returns 0
    // if a > b, returns a positive number
    template <is_big_integer T>
    constexpr s64 compare(const T &a, const types::is_integral auto &b_) {
        // We need to check for this case because we can't cast to big_integer_nan in cast_to_same_big_integer..
        if constexpr (types::is_same<T, big_integer_nan>) {
            if (b_.Size == S64_MAX) return 0;
            return S64_MAX;  // We treat NAN as bigger than anything imaginable
        } else {
            auto b = cast_to_same_big_integer<T>(b_);

            using big_integer_implementation::sdigit;

            // The type of _a_ was not big_integer_nan, but it has the same value.
            // A bit confusing... @Cleanup: Can we handle NAN better?
            if (a.Size == S64_MAX) {
                if (b.Size == S64_MAX) return 0;
                return S64_MAX;  // We treat NAN as bigger than anything imaginable
            }

            s64 sign = a.Size - b.Size;
            if (sign == 0) {
                s64 i       = abs(a.Size);
                sdigit diff = 0;
                while (--i >= 0) {
                    diff = (sdigit) a.Bigits[i] - (sdigit) b.Bigits[i];
                    if (diff) break;
                }
                sign = a.Size < 0 ? -diff : diff;
            }
            return sign;
        }
    }

    constexpr bool operator==(const is_big_integer auto &lhs, const types::is_integral auto &rhs) { return compare(lhs, rhs) == 0; }
    constexpr bool operator!=(const is_big_integer auto &lhs, const types::is_integral auto &rhs) { return compare(lhs, rhs) != 0; }
    constexpr bool operator<(const is_big_integer auto &lhs, const types::is_integral auto &rhs) { return compare(lhs, rhs) < 0; }
    constexpr bool operator>(const is_big_integer auto &lhs, const types::is_integral auto &rhs) { return compare(lhs, rhs) > 0; }
    constexpr bool operator<=(const is_big_integer auto &lhs, const types::is_integral auto &rhs) { return !(lhs > rhs); }
    constexpr bool operator>=(const is_big_integer auto &lhs, const types::is_integral auto &rhs) { return !(lhs < rhs); }

    template <is_big_integer T>
    constexpr auto operator+(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);

        if (lhs == NAN) return (T) NAN;
        if (rhs == NAN) return (T) NAN;

        if (lhs.Size < 0) {
            if (rhs.Size < 0) {
                auto z = big_integer_implementation::x_add(lhs, rhs);
                z.Size = -z.Size;
                return z;
            } else {
                return big_integer_implementation::x_sub(rhs, lhs);
            }
        } else {
            if (rhs.Size < 0) {
                return big_integer_implementation::x_sub(lhs, rhs);
            } else {
                return big_integer_implementation::x_add(lhs, rhs);
            }
        }
    }

    template <is_big_integer T>
    constexpr auto operator-(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);

        if (lhs == NAN) return (T) NAN;
        if (rhs == NAN) return (T) NAN;

        if (lhs.Size < 0) {
            if (rhs.Size < 0) {
                return big_integer_implementation::x_sub(rhs, lhs);
            } else {
                auto z = big_integer_implementation::x_add(lhs, rhs);
                z.Size = -z.Size;
                return z;
            }
        } else {
            if (rhs.Size < 0) {
                return big_integer_implementation::x_add(lhs, rhs);
            } else {
                return big_integer_implementation::x_sub(lhs, rhs);
            }
        }
    }

    template <is_big_integer T>
    constexpr auto operator*(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);

        if (lhs == NAN) return (T) NAN;
        if (rhs == NAN) return (T) NAN;

        T result = big_integer_implementation::k_mul(lhs, rhs);

        // Negate if exactly one of the inputs is negative
        if ((lhs.Size ^ rhs.Size) < 0 && result.Size) {
            result.Size = -result.Size;
        }

        return result;
    }

    template <is_big_integer T>
    constexpr auto operator/(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);

        if (lhs == NAN) return (T) NAN;
        if (rhs == NAN) return (T) NAN;

        auto [div, rem] = divrem(lhs, rhs);
        return div;
    }

    template <is_big_integer T>
    constexpr auto operator%(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);

        if (lhs == NAN) return (T) NAN;
        if (rhs == NAN) return (T) NAN;

        auto [div, mod] = divmod(lhs, rhs);
        return mod;
    }

    template <is_big_integer T>
    constexpr auto &operator+=(T &one, const types::is_integral auto &other) {
        one = one + other;
        return one;
    }

    template <is_big_integer T>
    constexpr auto &operator-=(T &one, const types::is_integral auto &other) {
        one = one - other;
        return one;
    }

    template <is_big_integer T>
    constexpr auto &operator*=(T &one, const types::is_integral auto &other) {
        one = one * other;
        return one;
    }

    template <is_big_integer T>
    constexpr auto &operator/=(T &one, const types::is_integral auto &other) {
        one = one / other;
        return one;
    }

    template <is_big_integer T>
    constexpr auto &operator%=(T &one, const types::is_integral auto &other) {
        one = one % other;
        return one;
    }

    // We don't support these.
    // constexpr u128 operator++(s32);
    // constexpr u128 operator--(s32);
    // constexpr u128 &operator++();
    // constexpr u128 &operator--();

    template <is_big_integer T>
    constexpr auto operator>>(const T &lhs, s64 n) {
        return big_integer_implementation::rshift(lhs, n);
    }

    template <is_big_integer T>
    constexpr auto operator<<(const T &lhs, s64 n) {
        return big_integer_implementation::lshift(lhs, n);
    }

    template <is_big_integer T>
    constexpr auto &operator>>=(T &one, s64 n) {
        one = one >> n;
        return one;
    }

    template <is_big_integer T>
    constexpr auto &operator<<=(T &one, s64 n) {
        one = one << n;
        return one;
    }

    template <is_big_integer T>
    constexpr auto operator&(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);
        return big_integer_implementation::bitwise(lhs, '&', rhs);
    }

    template <is_big_integer T>
    constexpr auto operator|(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);
        return big_integer_implementation::bitwise(lhs, '|', rhs);
    }

    template <is_big_integer T>
    constexpr auto operator^(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);
        return big_integer_implementation::bitwise(lhs, '^', rhs);
    }

    template <is_big_integer T>
    constexpr auto &operator&=(T &one, const types::is_integral auto &other_) {
        one = one & other_;
        return one;
    }

    template <is_big_integer T>
    constexpr auto &operator|=(T &one, const types::is_integral auto &other_) {
        one = one | other_;
        return one;
    }

    template <is_big_integer T>
    constexpr auto &operator^=(T &one, const types::is_integral auto &other_) {
        one = one ^ other_;
        return one;
    }

    // Note: For when you don't know how large the number would be, we provide big_integer_dynamic which allocates memory as it grows.
    //
    // Here we provide typedefs for signed integers bigger than 128 bits.
    //
    // We don't provide unsigned versions. Usually you would want to use unsigned versions for the extra positive range,
    // but since big integers may have practically unlimited range, you wouldn't care for that extra bit.
    // Providing unsigned versions would mean doubling our code to support it, which is not something I'm fond of.
    //
    // These (contrary to s128/u128) are not guaranteed to be memory aligned to their respective sizes.
    // Operations with these don't overflow but simply fail and assert and return a special value - NAN.
    //
    // These (like normal integers) are immutable. Which means that each operation you do creates a new big integer.
    // This seems more natural than to provide methods which modify the integer in place:
    //  1) You'd rarely want to discard the old value when doing an operation and having to copy is tedious.
    //  2) For when you want to discard it, use assignment operators (e.g. +=, -=, *=, ... ).
    // @TODO @Speed We can provide specialized implementations for case 2 which are way
    //              faster than what we are currently doing (just calling the normal operator).
    using big_int_256  = big_integer_on_the_stack<8>;
    using big_int_512  = big_integer_on_the_stack<16>;
    using big_int_1024 = big_integer_on_the_stack<32>;
    using big_int_2048 = big_integer_on_the_stack<64>;
    using big_int_4096 = big_integer_on_the_stack<128>;
}

LSTD_END_NAMESPACE
