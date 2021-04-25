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

    constexpr u32 SHIFT = 30;
    constexpr u32 BASE  = (digit) 1 << SHIFT;
    constexpr u32 MASK  = BASE - 1;
    }  // namespace big_integer_implementation

    struct big_integer_nan {
        static constexpr s64 Capacity = 0;
        static constexpr s64 Size     = S64_MAX;

        stack_array<big_integer_implementation::digit, 1> Bigits{};

        constexpr bool ensure_bigits(s64 n) { return false; }
    };

    // This is a special value which is returned when certain errors in arithmetic occur: overflows/underflows, division by zero.
    constexpr big_integer_nan NAN = {};

    // This are the requirements for an object to be considered a big integer and on which operations are compiled.
    // We do this in order to reuse code. That way we can have a big integer on the stack and a big integer which grows dynamically,
    // operations are implemented transparently despite the underlying storage. Operations also work between stack and dynamic big integers for example.
    // This is like compile-time inheritance but it doesn't have runtime overhead.
    // Concepts allow us to write both type-safe and clean, readable, reusable code. Kudos for C++20.
    //
    // We also use a similar trick in "array_like.h". :CodeReusability:
    template <typename T>
    concept is_big_integer = requires(T t) {
        // Storage for digits in base 2^30 ("bigits").
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

    namespace big_integer_implementation {
    // Assign an integral value (s8, s16, s32, s64, u8, u16, u32, u64, s128, u128, other big integers,
    // as well as user integral types - declared with DECLARE_INTEGRAL or DECLARE_INTEGRAL_PAIR) to a big integer _b_.
    constexpr bool assign(is_big_integer auto &b, const types::is_integral auto &v);
    }  // namespace big_integer_implementation

    template <s64 Capacity_>
    struct big_integer_on_the_stack {
        static constexpr s64 Capacity = Capacity_;
        static_assert(Capacity > 4, "For smaller values, use s8, s16, s32, s64, s128. They are faster.");

        stack_array<big_integer_implementation::digit, Capacity> Bigits{};
        s64 Size = 0;

        constexpr big_integer_on_the_stack() {}
        constexpr big_integer_on_the_stack(big_integer_nan) : Size(S64_MAX) {}

        constexpr big_integer_on_the_stack(const types::is_integral auto &value) { big_integer_implementation::assign(*this, value); }

        big_integer_on_the_stack &operator=(const types::is_integral auto &value) {
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

    // Shift digit vector a[0:m] d bits left, with 0 <= d < SHIFT.  Put
    // result in z[0:m], and return the d bits shifted out of the top.
    constexpr digit v_lshift(digit *z, const digit *a, s64 m, s32 d) {
        assert(0 <= d && d < SHIFT);

        digit carry = 0;
        For(range(m)) {
            double_digit acc = (double_digit) a[it] << d | carry;
            z[it]            = (digit) acc & MASK;
            carry            = (digit)(acc >> SHIFT);
        }
        return carry;
    }

    // Shift digit vector a[0:m] d bits right, with 0 <= d < SHIFT.  Put
    // result in z[0:m], and return the d bits shifted out of the bottom.
    constexpr digit v_rshift(digit *z, const digit *a, s64 m, s32 d) {
        assert(0 <= d && d < SHIFT);

        digit carry = 0;
        digit mask  = ((digit) 1 << d) - 1U;

        For(range(m - 1, -1, -1)) {
            double_digit acc = (double_digit) carry << SHIFT | a[it];
            carry            = (digit) acc & mask;
            z[it]            = (digit)(acc >> d);
        }
        return carry;
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
            // Note: Big integers should have minimum storage of 5 u32s (bigger than u128). We static_assert that.
            // Otherwise you'd better be using the smaller intrinsic types..
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

                *pz++ = (digit)(carry & MASK);

                carry >>= SHIFT;
                assert(carry <= MASK);

                // Now f is added in twice in each column of the
                // pyramid it appears. Same as adding f<<1 once.
                f <<= 1;
                while (pa < paend) {
                    carry += *pz + *pa++ * f;
                    *pz++ = (digit)(carry & MASK);
                    carry >>= SHIFT;
                    assert(carry <= (MASK << 1));
                }

                if (carry) {
                    carry += *pz;
                    *pz++ = (digit)(carry & MASK);
                    carry >>= SHIFT;
                }

                if (carry) *pz += (digit)(carry & MASK);

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
                    *pz++ = (digit)(carry & MASK);
                    carry >>= SHIFT;
                    assert(carry <= MASK);
                }

                if (carry) *pz += (digit)(carry & MASK);

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

        copy_memory(low->Bigits.Data, n.Bigits.Data, sizelo * sizeof(digit));
        copy_memory(high->Bigits.Data, n.Bigits.Data + sizelo, sizehi * sizeof(digit));

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

            copy_memory(bslice.Bigits.Data, b->Bigits.Data + nbdone, nbtouse * sizeof(digit));
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
        copy_memory(result.Bigits.Data + 2 * shift, t1.Bigits.Data, t1.Size * sizeof(digit));

        // Zero-out the digits higher than the ah*bh copy
        i = result.Size - 2 * shift - t1.Size;
        if (i) fill_memory(result.Bigits.Data + 2 * shift + t1.Size, 0, i * sizeof(digit));

        T t2;

        // 3. t2 <- al*bl, and copy into the low digits
        if ((t2 = k_mul(*al, *bl)) == NAN) return (T) NAN;

        assert(t2.Size >= 0);
        assert(t2.Size <= 2 * shift); /* no overlap with high digits */
        copy_memory(result.Bigits.Data, t2.Bigits.Data, t2.Size * sizeof(digit));

        // Zero out remaining digits
        i = 2 * shift - t2.Size;  // number of uninitialized digits
        if (i) copy_memory(result.Bigits.Data + t2.Size, 0, i * sizeof(digit));

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
            rem     = (rem << SHIFT) | *--pin;
            *--pout = hi = (digit)(rem / n);
            rem -= (double_digit) hi * n;
        }
        return (digit) rem;
    }

    template <is_big_integer T>
    struct divrem1_result {
        T Div;
        digit Rem;
    };

    // Divide an integer by a digit, returning both the quotient
    // and the remainder. The sign of _a_ is ignored.
    // Returns NAN on error.
    template <is_big_integer T>
    constexpr auto divrem1(const T &a, digit n) {
        if (a == NAN) return divrem1_result<T>{a, 0};

        assert(n > 0 && n <= MASK);

        s64 size = abs(a.Size);

        T result;
        if (!result.ensure_bigits(size)) return divrem1_result<T>{NAN, 0};
        result.Size = size;

        digit rem = inplace_divrem1(result.Bigits.Data, a.Bigits.Data, size, n);
        normalize(result);

        return divrem1_result<T>{result, rem};
    }

    // Gives the remainder after division of |a| by |b|, with the sign of a.
    // This is also expressed as a - b * trunc(a/b), if trunc truncates towards zero.
    // Some examples:
    //   a           b      a rem b
    //   13          10      3
    //  -13          10     -3
    //   13         -10      3
    //  -13         -10     -3
    template <is_big_integer T>
    struct divrem_result {
        T Div, Rem;
    };

    // Returns NAN on error.
    template <is_big_integer T>
    constexpr auto divrem(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);

        if (lhs == NAN) return divrem_result<T>{NAN, NAN};
        if (rhs == NAN) return divrem_result<T>{NAN, NAN};

        s64 sizea = abs(lhs.Size), sizeb = abs(rhs.Size);

        divrem_result<T> result;  // _lhs_ determines the resulting types

        if (!sizeb) {
            assert(false && "Division by zero");
            return divrem_result<T>{NAN, NAN};
        }

        if (sizea < sizeb || (sizea == sizeb && (lhs.Bigits[sizea - 1] < rhs.Bigits[sizeb - 1]))) {
            // |lhs| < |rhs|
            assign(result.Div, 0);

            // result.Div = 0;
            result.Rem = lhs;
        } else {
            // |lhs| >= |rhs|
            if (sizeb == 1) {
                auto [div, rem] = big_integer_implementation::divrem1(lhs, rhs.Bigits[0]);
                result.Div      = div;
                assign(result.Rem, rem);
            } else {
                // We follow Knuth [The Art of Computer Programming, Vol. 2 (3rd edn.), section 4.3.1, Algorithm D],
                // except that we don't explicitly handle the special case when the initial estimate q for a quotient
                // digit is >= BASE: the max value for q is BASE + 1, and that won't overflow a digit.

                T v;
                if (!v.ensure_bigits(sizea + 1)) {
                    assert(false && "Overflow");
                    return divrem_result<T>{NAN, NAN};
                }

                auto &w      = result.Rem;  // Short-hand. _w_ holds the remainder.
                bool ensured = w.ensure_bigits(sizeb);
                assert(ensured && "Impossible! sizea >= sizeb!!!");

                // Normalize: shift lhs left so that its top digit is >= BASE / 2.
                // Shift rhs left by the same amount.
                // Results go into w and v.

                digit d, carry;

                d = SHIFT - (count_digits_base_2(rhs.Bigits[sizeb - 1]) + 1);

                carry = v_lshift(w.Bigits.Data, rhs.Bigits.Data, sizeb, d);
                assert(carry == 0);
                carry = v_lshift(v.Bigits.Data, lhs.Bigits.Data, sizea, d);

                if (carry || v.Bigits[sizea - 1] >= w.Bigits[sizeb - 1]) {
                    v.Bigits[sizea] = carry;
                    sizea++;  // Before we ensured _v_ has sizea + 1 digits.
                }

                // Now v.Bigits[sizea - 1] < w.Bigits[sizeb - 1],
                // so the quotient has at most (and usually exactly)
                // k = sizea - sizeb digits.

                s64 k = sizea - sizeb;
                assert(k >= 0);

                auto &a = result.Div;  // Short-hand. _a_ holds the quotient.
                ensured = a.ensure_bigits(k);
                assert(ensured);

                auto *v0  = v.Bigits.Data;
                auto *w0  = w.Bigits.Data;
                digit wm1 = w0[sizeb - 1];
                digit wm2 = w0[sizeb - 2];

                auto *vk = v0 + k;
                auto *ak = a.Bigits.Data + k;

                while (vk-- > v0) {
                    // Inner loop: Divide vk[0:sizeb+1] by w0[0:sizeb],
                    // giving single-digit quotient q, remainder in vk[0:sizeb].

                    // Estimate quotient digit q; may overestimate by 1 (rare)

                    digit vtop = vk[sizeb];
                    assert(vtop <= wm1);

                    double_digit vv = ((double_digit) vtop << SHIFT) | vk[sizeb - 1];

                    digit q = (digit)(vv / wm1);
                    digit r = (digit)(vv - (double_digit) wm1 * q);  // r = vv % wm1

                    while ((double_digit) wm2 * q > (((double_digit) r << SHIFT) | vk[sizeb - 2])) {
                        --q;
                        r += wm1;
                        if (r >= BASE) break;
                    }
                    assert(q <= BASE);

                    // Subtract q * w0[0:sizeb] from vk[0:sizeb+1]
                    sdigit zhi = 0;

                    For(range(sizeb)) {
                        // Invariants: -BASE <= -q <= zhi <= 0;
                        //             -BASE * q <= z < BASE
                        sdigit z = (sdigit) vk[it] + zhi - (sdouble_digit) q * (sdouble_digit) w0[it];
                        vk[it]   = (digit) z & MASK;
                        zhi      = z >> SHIFT;
                    }

                    // Add w back if q was too large
                    assert((sdigit) vtop + zhi == -1 || (sdigit) vtop + zhi == 0);
                    if ((sdigit) vtop + zhi < 0) [[unlikely]] {
                        carry = 0;
                        For(range(sizeb)) {
                            carry += vk[it] + w0[it];
                            vk[it] = carry & MASK;
                            carry >>= SHIFT;
                        }
                        --q;
                    }

                    // Store quotient digit
                    assert(q < BASE);
                    *--ak = q;
                }

                // Unshift remainder; we reuse w to store the result
                carry = v_rshift(w0, v0, sizeb, d);
                assert(carry == 0);
            }
        }

        // Set the signs.
        // The quotient z has the sign of lhs * rhs;
        // the remainder r has the sign of lhs, so lhs = rhs * z + r.
        if ((lhs.Size < 0) != (rhs.Size < 0)) {
            result.Div.Size = -result.Div.Size;
        }
        if (lhs.Size < 0 && result.Rem.Size != 0) {
            result.Rem.Size = -result.Rem.Size;
        }
        return result;
    }
    }  // namespace big_integer_implementation

    using big_integer_implementation::divrem;

    template <is_big_integer T>
    struct divmod_result {
        T Div, Mod;
    };

    // The expression a mod b has the value a - b * floor(a/b).
    // Some examples:
    //   a           b      a mod b
    //   13          10      3
    //  -13          10      7
    //   13         -10     -7
    //  -13         -10     -3
    //
    // The / and % operators are now defined in terms of divmod().
    //
    // Returns NAN on error.
    template <is_big_integer T>
    constexpr auto divmod(const T &lhs, const types::is_integral auto &rhs_) {
        auto rhs = cast_to_same_big_integer<T>(rhs_);

        if (lhs == NAN) return divmod_result<T>{NAN, NAN};
        if (rhs == NAN) return divmod_result<T>{NAN, NAN};

        auto [div, mod] = divrem(lhs, rhs);

        // To get from rem to mod, we have to add rhs if lhs and rhs
        // have different signs.  We then subtract one from the 'div'
        // part of the outcome to keep the invariant intact.

        if ((mod.Size < 0 && rhs.Size > 0) || (mod.Size > 0 && rhs.Size < 0)) {
            mod += rhs;
            div -= 1;
        }

        return divmod_result<T>{div, mod};
    }

    constexpr bool operator==(const is_big_integer auto &lhs, const is_big_integer auto &rhs) {
        if (lhs.Size != rhs.Size) return false;
        For(range(lhs.Size)) {
            if (lhs.Bigits[it] != rhs.Bigits[it]) return false;
        }
        return true;
    }

    constexpr bool operator!=(const is_big_integer auto &lhs, const is_big_integer auto &rhs) { return !(lhs == rhs); }

    constexpr bool operator<(const is_big_integer auto &lhs, const is_big_integer auto &rhs) {
        s64 sizea = abs(lhs.Size), sizeb = abs(rhs.Size);
        if (sizea > sizeb) {
            return false;
        } else if (sizea < sizeb) {
            return true;
        } else {
            // Find highest digit where a and b differ
            s64 i = sizea;
            while (--i >= 0 && lhs.Bigits[i] == rhs.Bigits[i])
                ;

            if (lhs.Bigits[i] < rhs.Bigits[i]) {
                return true;
            } else {
                return false;
            }
        }
    }

    constexpr bool operator>(const is_big_integer auto &lhs, const is_big_integer auto &rhs) { return rhs < lhs; }
    constexpr bool operator<=(const is_big_integer auto &lhs, const is_big_integer auto &rhs) { return !(rhs < lhs); }
    constexpr bool operator>=(const is_big_integer auto &lhs, const is_big_integer auto &rhs) { return !(lhs < rhs); }

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
    constexpr auto &operator+=(T &one, const types::is_integral auto &other_) {
        auto other = cast_to_same_big_integer<T>(other_);

        one = one + other;
        return one;
    }

    template <is_big_integer T>
    constexpr auto &operator-=(T &one, const types::is_integral auto &other_) {
        auto other = cast_to_same_big_integer<T>(other_);

        one = one - other;
        return one;
    }

    // constexpr u128 &operator+=(u128 other);
    // constexpr u128 &operator-=(u128 other);
    // constexpr u128 &operator*=(u128 other);
    // constexpr u128 &operator/=(u128 other);
    // constexpr u128 &operator%=(u128 other);
    // constexpr u128 operator++(s32);
    // constexpr u128 operator--(s32);
    // constexpr u128 &operator<<=(s32);
    // constexpr u128 &operator>>=(s32);
    // constexpr u128 &operator&=(u128 other);
    // constexpr u128 &operator|=(u128 other);
    // constexpr u128 &operator^=(u128 other);
    // constexpr u128 &operator++();
    // constexpr u128 &operator--();

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
    using s256  = big_integer_on_the_stack<8>;
    using s512  = big_integer_on_the_stack<16>;
    using s1024 = big_integer_on_the_stack<32>;
    using s2048 = big_integer_on_the_stack<64>;
    using s4096 = big_integer_on_the_stack<128>;
}

LSTD_END_NAMESPACE
