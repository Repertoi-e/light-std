module;

#include "../common.h"

export module lstd.big_integer;

import lstd.stack_array;
import lstd.context;

//
// Provides arbitrary precision integer math.
// Supports integers on the stack (with compile-time capacity)
// and a dynamic integer which grows as it needs more range.
//

LSTD_BEGIN_NAMESPACE

//
// We use u32s for storing single digits, each digit has range from -2^30 to 2^30.
// When implementing operations we use grade school math,
// but treat each digit of the integer as in base 2^30 instead of base 10.
//
// Using base 2^30 instead of 2^32 simplifies some code since we have room for some overflow which is then carried.
//
using digit  = u32;
using sdigit = s32;

using double_digit  = u64;
using sdouble_digit = s64;

constexpr u32 SHIFT  = 30;
constexpr digit BASE = (digit) 1 << SHIFT;
constexpr digit MASK = BASE - 1;

export {
    //
    // Big integers grow as they need more memory (similar to a dynamic array)
    // so they have (practically) infinite precision.
    //
    // We don't provide an unsigned version. Usually you would want to use unsigned
    // integers for the extra range, but big integers don't store their
    // sign in a bit that can be used to encode more numbers.
    // (and as we said big integers have practically unlimited range).
    //
    // When doing arithmetic operations we allocate a new integer (we don't do the work in place).
    // This is because: 1. for most operations we need to allocate a seperate buffer to store the result
    // anyway; 2. sometimes you don't want to discard the old result.
    //
    // To free the big_integer use: if (b.Digits) free(b.Digits);
    // Note: We don't expect you to be freeing big integers very often:
    //
    //    Don't do big integer operations with a regular allocator
    //    because that would cause lots of fragmentation. Instead,
    //    create a small isolated arena allocator and do a free_all when you're done.
    //    This makes stuff very light to allocate and can't create
    //    leaks if you forget to free an integer.
    //
    struct big_integer {
        // Negative if the big integer is negative.
        // The absolute value of this says how many digits in _Digits_ are used.
        s64 Size = 0;

        digit *Digits = null;

        union {
            digit SmallDigits[2]{};
            s64 Allocated;
        };
    };

    // For small values (2 digits) we use a small buffer contained in the structure.
    // In that case Digits == null (because members pointing to other members is dangerous).
    constexpr bool is_small(big_integer * b) { return !b->Digits; }

    // Attempts to convert an integer into a small integer and frees any allocated memory.
    constexpr void maybe_small(big_integer * b) {
        if (is_small(b)) return;

        if (abs(b->Size) <= 2) {
            digit d1 = b->Digits[0];
            digit d2 = b->Digits[1];
            free(b->Digits);
            b->Digits         = null;
            b->SmallDigits[0] = d1;
            b->SmallDigits[1] = d2;
        }
    }

    // For small integers, b->Digits is null and the digits
    // are stored in a small buffer inside the struct.
    constexpr digit *get_digits(big_integer * b) {
        if (is_small(b)) return b->SmallDigits;
        return b->Digits;
    }

    constexpr digit get_digit(big_integer * b, s64 index) {
        s64 space = is_small(b) ? 2 : b->Allocated;
        index     = translate_index(index, space);
        return get_digits(b)[index];
    }

    constexpr void set_digit(big_integer * b, s64 index, digit value) {
        s64 space            = is_small(b) ? 2 : b->Allocated;
        index                = translate_index(index, space);
        get_digits(b)[index] = value;
    }

    // Ensure there is space for at least _n_ digits in the big integer.
    constexpr void ensure_digits(big_integer * b, s64 n);

    // Ensure there is space for at least _n_ digits in the big integer
    // but reserve more than needed (next power of two).
    constexpr void grow(big_integer * b, s64 n);

    // Note: Big integers have minimum storage of 5 digits (and are bigger than u128).
    // For smaller values you'd better be using the smaller integer types (8 to 128 bit).
    constexpr big_integer create_big_integer(s64 initialDigits);

    // Assign any integral value (s8, s16, s32, s64, u8, u16, u32, u64, s128, u128,
    // other big integers, etc.) to a big integer.
    constexpr bool assign(big_integer * b, types::is_integral auto v);
    constexpr big_integer cast_big(types::is_integral auto v);

    // Save an unnecessary allocation
    constexpr big_integer cast_big(big_integer b) { return b; }

    // Remove "leading zeros" from a big integer (e.g. 000000000000000000001).
    // Those are actually the trailing zeros in the Digits array since we store chunks in reverse.
    constexpr void normalize(big_integer * b);

    constexpr big_integer invert(big_integer b);
    constexpr big_integer lshift(big_integer lhs, s64 n);
    constexpr big_integer rshift(big_integer lhs, s64 n);

    struct div_result {
        big_integer Q;  // Quotient
        big_integer R;  // Remainder/modulus, note: see divrem vs divmod comparison
    };

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
    constexpr div_result divrem(big_integer lhs, big_integer rhs);

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
    constexpr div_result divmod(big_integer lhs, big_integer rhs);

    // if a < b, returns -1
    // if a == b, returns 0
    // if a > b, returns 1
    constexpr s32 compare(big_integer a, big_integer b);

    // We don't support ++/-- and assignment operators.

    constexpr bool operator==(big_integer lhs, big_integer rhs) { return compare(lhs, rhs) == 0; }
    constexpr bool operator!=(big_integer lhs, big_integer rhs) { return compare(lhs, rhs) != 0; }
    constexpr bool operator<(big_integer lhs, big_integer rhs) { return compare(lhs, rhs) < 0; }
    constexpr bool operator<=(big_integer lhs, big_integer rhs) { return compare(lhs, rhs) <= 0; }
    constexpr bool operator>(big_integer lhs, big_integer rhs) { return compare(lhs, rhs) > 0; }
    constexpr bool operator>=(big_integer lhs, big_integer rhs) { return compare(lhs, rhs) >= 0; }

    constexpr big_integer operator+(big_integer lhs, big_integer rhs);
    constexpr big_integer operator-(big_integer lhs, big_integer rhs);
    constexpr big_integer operator*(big_integer lhs, big_integer rhs);
    constexpr big_integer operator/(big_integer lhs, big_integer rhs);
    constexpr big_integer operator%(big_integer lhs, big_integer rhs);

    constexpr big_integer operator>>(big_integer lhs, s64 n) { return rshift(lhs, n); }
    constexpr big_integer operator<<(big_integer lhs, s64 n) { return lshift(lhs, n); }

    constexpr big_integer operator&(big_integer lhs, big_integer rhs);
    constexpr big_integer operator|(big_integer lhs, big_integer rhs);
    constexpr big_integer operator^(big_integer lhs, big_integer rhs);
}

constexpr void ensure_digits(big_integer *b, s64 n) {
    assert(n > 0);

    if (is_small(b)) {
        if (n <= 2) return;

        digit d1 = b->SmallDigits[0];
        digit d2 = b->SmallDigits[1];

        b->Digits = malloc<digit>({.Count = n});
        zero_memory(b->Digits, n * sizeof(digit));
        b->Allocated = n;

        b->Digits[0] = d1;
        b->Digits[1] = d2;
    } else if (b->Allocated < n) {
        b->Digits = realloc(b->Digits, {.NewCount = n});
        zero_memory(b->Digits + b->Allocated, (n - b->Allocated) * sizeof(digit));
        b->Allocated = n;
    }
}

constexpr void grow(big_integer *b, s64 n) {
    s64 target = max(ceil_pow_of_2(n + 1), 8);
    ensure_digits(b, target);
}

constexpr big_integer create_big_integer(s64 initialDigits) {
    big_integer b;
    ensure_digits(&b, initialDigits);
    return b;
}

constexpr big_integer create_big_integer_and_set_size(s64 initialDigits) {
    big_integer b = create_big_integer(initialDigits);
    b.Size        = initialDigits;
    return b;
}

constexpr void normalize(big_integer *b) {
    s64 j = abs(b->Size);
    s64 i = j;

    while (i > 0 && get_digit(b, i - 1) == 0) --i;

    if (i != j) b->Size = b->Size < 0 ? -i : i;
    maybe_small(b);
}

constexpr big_integer cast_big(types::is_integral auto v) {
    big_integer b;
    assign(&b, v);
    return b;
}

constexpr bool assign(big_integer *b, types::is_integral auto v) {
    if constexpr (types::is_same<big_integer, decltype(v)>) {
        // Assign from another big integer
        ensure_digits(b, abs(v.Size));
        copy_memory(get_digits(b), get_digits(&v), abs(v.Size) * sizeof(digit));
        b->Size = v.Size;
    } else {
        auto binaryDigits = count_digits<1>(abs(v));  // How many bits do we need to store the value in _v_

        auto digits = binaryDigits / BASE + (binaryDigits % BASE != 0);
        ensure_digits(b, digits);

        s64 size = 0, sign = 1;
        if constexpr (types::is_signed_integral<decltype(v)>) {
            if (v < 0) {
                v    = -v;
                sign = -1;
            }
        }

        auto *p = get_digits(b);

        do {
            if constexpr (sizeof(v) == sizeof(u128)) {
                // Handle 128 bit integers
                *p++ = v.lo & MASK;
                ++size;
            } else {
                // Handle 8, 16, 32, 64 bit integers
                *p++ = v & MASK;
                ++size;
            }
            v >>= SHIFT;
        } while (v);

        b->Size = size * sign;
    }
    normalize(b);

    return true;
}

// x[0:m] and y[0:n] are digit vectors, LSD first, m >= n required.  x[0:n]
// is modified in place, by adding y to it. Carries are propagated as far as
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
// is modified in place, by subtracting y from it. Borrows are propagated as
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

constexpr big_integer x_add(big_integer *a, big_integer *b) {
    s64 sizea = abs(a->Size), sizeb = abs(b->Size);

    if (sizea < sizeb) {
        swap(a, b);
        swap(sizea, sizeb);
    }

    big_integer result = create_big_integer(sizea + 1);

    digit carry = 0;

    s64 i = 0;
    while (i < sizeb) {
        carry += get_digit(a, i) + get_digit(b, i);
        set_digit(&result, i, carry & MASK);
        carry >>= SHIFT;

        ++i;
    }

    while (i < sizea) {
        carry += get_digit(a, i);
        set_digit(&result, i, carry & MASK);
        carry >>= SHIFT;

        ++i;
    }
    set_digit(&result, i, carry & MASK);

    result.Size = sizea + 1;
    normalize(&result);
    return result;
}

constexpr big_integer x_sub(big_integer *a, big_integer *b) {
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

        while (--i >= 0 && get_digit(a, i) == get_digit(b, i)) {}

        if (i < 0) return cast_big(0);

        if (get_digit(a, i) < get_digit(b, i)) {
            sign = -1;
            swap(a, b);
        }
        sizea = sizeb = i + 1;
    }

    auto result = create_big_integer(sizea);

    digit borrow = 0;
    s64 i        = 0;
    while (i < sizeb) {
        borrow = get_digit(a, i) - get_digit(b, i) - borrow;

        set_digit(&result, i, borrow & MASK);
        borrow >>= SHIFT;
        borrow &= 1;  // Keep only one sign bit

        ++i;
    }

    while (i < sizea) {
        borrow = get_digit(a, i) - borrow;

        set_digit(&result, i, borrow & MASK);
        borrow >>= SHIFT;
        borrow &= 1;  // Keep only one sign bit

        ++i;
    }

    assert(borrow == 0);

    result.Size = sign * sizea;
    normalize(&result);
    return result;
}

// Grade-school algorithm.
constexpr big_integer x_mul(big_integer lhs, big_integer rhs) {
    s64 sizea = abs(lhs.Size), sizeb = abs(rhs.Size);

    auto result = create_big_integer_and_set_size(sizea + sizeb);
    if (lhs == rhs) {
        // Efficient squaring per HAC, Algorithm 14.16:
        // http://www.cacr.math.uwaterloo.ca/hac/about/chap14.pdf
        // Gives slightly less than a 2x speedup when a == b,
        // via exploiting that each entry in the multiplication
        // pyramid appears twice (except for the sizea squares).

        For(range(sizea)) {
            double_digit f = get_digit(&lhs, it);

            auto *pz    = get_digits(&result) + (it << 1);
            auto *pa    = get_digits(&lhs) + it + 1;
            auto *paend = get_digits(&lhs) + sizea;

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
            double_digit f = get_digit(&lhs, it);

            auto *pz    = get_digits(&result) + it;
            auto *pb    = get_digits(&rhs);
            auto *pbend = get_digits(&rhs) + sizeb;

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

    normalize(&result);
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
constexpr void kmul_split(big_integer n, s64 size, big_integer *high, big_integer *low) {
    s64 sizen = abs(n.Size);

    s64 sizelo = min(sizen, size);
    s64 sizehi = sizen - sizelo;

    ensure_digits(high, sizehi);
    ensure_digits(low, sizelo);

    copy_memory(get_digits(low), get_digits(&n), sizelo * sizeof(digit));
    copy_memory(get_digits(high), get_digits(&n) + sizelo, sizehi * sizeof(digit));
    high->Size = sizehi;
    low->Size  = sizelo;

    normalize(high);
    normalize(low);
}

constexpr big_integer k_mul(big_integer *lhs, big_integer *rhs);

// b has at least twice the digits of a, and a is big enough that Karatsuba
// would pay off *if* the inputs had balanced sizes.  View b as a sequence
// of slices, each with a->ob_size digits, and multiply the slices by a,
// one at a time.  This gives k_mul balanced inputs to work with, and is
// also cache-friendly (we compute one double-width slice of the result
// at a time, then move on, never backtracking except for the helpful
// single-width slice overlap between successive partial sums).
constexpr big_integer k_lopsided_mul(big_integer *a, big_integer *b) {
    s64 sizea = abs(a->Size), sizeb = abs(b->Size);

    assert(sizea > KARATSUBA_CUTOFF);
    assert(2 * sizea <= sizeb);

    big_integer result = create_big_integer_and_set_size(sizea + sizeb);

    // Successive slices of b are copied into bslice
    big_integer bslice = create_big_integer_and_set_size(sizea);

    s64 nbdone = 0;  // # of b digits already multiplied
    while (sizeb > 0) {
        s64 nbtouse = min(sizeb, sizea);

        // Multiply the next slice of b by a

        copy_memory(get_digits(&bslice), get_digits(b) + nbdone, nbtouse * sizeof(digit));
        bslice.Size = nbtouse;

        big_integer product = k_mul(a, &bslice);

        // Add into result
        v_iadd(get_digits(&result) + nbdone, result.Size - nbdone, get_digits(&product), product.Size);

        sizeb -= nbtouse;
        nbdone += nbtouse;
    }

    normalize(&result);
    return result;
}

// Multiply the absolute values of two integers.
// Karatsuba multiplication. See Knuth Vol. 2 Chapter 4.3.3 (Pp. 294-295).
constexpr big_integer k_mul(big_integer *a, big_integer *b) {
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
            return cast_big(0);
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

    big_integer ah_, al_;

    // Split a & b into hi & lo pieces
    s64 shift = sizeb >> 1;
    kmul_split(*a, shift, &ah_, &al_);

    assert(ah_.Size > 0);  // The split isn't degenerate

    big_integer bh_, bl_;

    big_integer *ah = &ah_, *al = &al_;
    big_integer *bh, *bl;

    if (*a == *b) {
        bh = ah;
        bl = al;
    } else {
        kmul_split(*b, shift, &bh_, &bl_);

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

    big_integer result = create_big_integer_and_set_size(sizea + sizeb);

    // 2. t1 <- ah*bh, and copy into high digits of result
    big_integer t1 = k_mul(ah, bh);

    assert(t1.Size >= 0);
    assert(2 * shift + t1.Size <= result.Size);
    copy_memory(get_digits(&result) + 2 * shift, get_digits(&t1), t1.Size * sizeof(digit));

    // Zero-out the digits higher than the ah*bh copy
    i = result.Size - 2 * shift - t1.Size;
    if (i) fill_memory((char *) (get_digits(&result) + 2 * shift + t1.Size), (char) 0, i * sizeof(digit));

    // 3. t2 <- al*bl, and copy into the low digits
    big_integer t2 = k_mul(al, bl);

    assert(t2.Size >= 0);
    assert(t2.Size <= 2 * shift); /* no overlap with high digits */
    copy_memory(get_digits(&result), get_digits(&t2), t2.Size * sizeof(digit));

    // Zero out remaining digits
    i = 2 * shift - t2.Size;  // number of uninitialized digits
    if (i) zero_memory(get_digits(&result) + t2.Size, i * sizeof(digit));

    // 4 & 5. Subtract ah*bh (t1) and al*bl (t2).
    // We do al*bl first because it's fresher in cache.
    i = result.Size - shift;  // # digits after shift
    v_isub(get_digits(&result) + shift, i, get_digits(&t2), t2.Size);

    v_isub(get_digits(&result) + shift, i, get_digits(&t1), t1.Size);

    // 6. t3 <- (ah+al)(bh+bl), and add into result
    t1 = x_add(ah, al);
    ah = al = null;

    if (*a == *b) {
        t2 = t1;
    } else {
        t2 = x_add(bh, bl);
    }

    bh = bl = null;

    big_integer t3 = k_mul(&t1, &t2);
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
    v_iadd(get_digits(&result) + shift, i, get_digits(&t3), t3.Size);

    normalize(&result);
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

struct div1_result {
    big_integer Quot;
    digit Rem;
};

// Divide an integer by a digit, returning both the quotient
// and the remainder. The sign of _a_ is ignored.
constexpr div1_result divrem1(big_integer a, digit n) {
    if (n == 0) panic("Division by zero");

    assert(n > 0 && n <= MASK);

    s64 size = abs(a.Size);

    big_integer result = create_big_integer_and_set_size(size);

    digit rem = inplace_divrem1(get_digits(&result), get_digits(&a), size, n);
    normalize(&result);

    return {result, rem};
}

constexpr div_result x_divrem(big_integer a, big_integer b) {
    // We follow Knuth [The Art of Computer Programming, Vol. 2 (3rd edn.), section 4.3.1, Algorithm D].
    // This divides an n-word dividend by an m-word divisor
    // and gives an n-m+1-word quotient and m-word remainder.

    //
    // https://skanthak.homepage.t-online.de/division.html
    // https://surface.syr.edu/cgi/viewcontent.cgi?article=1162&context=eecs_techreports
    //

    s64 sizea = abs(a.Size), sizeb = abs(b.Size);
    if (!sizeb) panic("Division by zero");

    big_integer na, nb;  // Normalized form of _a_ and _b_; _na_ also stores the normalized remainder in the end.

    // Normalize by shifting _b_ left just enough so that its high-order
    // bit is on, and shift _a_ left the same amount. We may have to append a
    // high-order digit on the dividend; we do that unconditionally.

    // This counts the number of leading zeros
    auto s = SHIFT - (msb(get_digit(&b, sizeb - 1)) + 1);

    assert(s >= 0 && s <= 31);

    na = lshift(a, s);
    nb = lshift(b, s);

    // If _a_ overflows..
    sizea = abs(na.Size);

    if (get_digit(&na, sizea - 1) >= get_digit(&nb, sizeb - 1)) {
        ensure_digits(&na, sizea + 1);
        set_digit(&na, sizea, 0);
        sizea += 1;
    }

    s64 j = sizea - sizeb;

    big_integer q = create_big_integer_and_set_size(j);  // _q_ holds the quotient

    while (j--) {
        digit atop = get_digit(&na, j + sizeb);
        assert(atop <= get_digit(&nb, sizeb - 1));

        // Compute estimate qhat; may overestimate by 1 (rare).
        double_digit vv = ((double_digit) atop << SHIFT) | get_digit(&na, j + sizeb - 1);

        digit qhat = (digit) (vv / get_digit(&nb, sizeb - 1));
        digit rhat = (digit) (vv - qhat * (double_digit) get_digit(&nb, sizeb - 1));

        while ((double_digit) get_digit(&nb, sizeb - 2) * qhat > (((double_digit) rhat << SHIFT) | get_digit(&na, j + sizeb - 2))) {
            --qhat;
            rhat += get_digit(&nb, sizeb - 1);
            if (rhat >= BASE) break;
        }
        assert(qhat <= BASE);

        // Multiply and subtract (qhat*nb[0:sizeb] from na[j:j+sizeb+1])
        sdouble_digit z;
        sdigit zhi = 0;

        For(range(sizeb)) {
            // Invariants: -BASE <= -qhat <= zhi <= 0;
            //             -BASE * qhat <= z < BASE

            z = (sdigit) get_digit(&na, it + j) + zhi -
                (sdouble_digit) qhat * (sdouble_digit) get_digit(&nb, it);

            set_digit(&na, it + j, z & MASK);
            zhi = (sdigit) (z >> SHIFT);
        }

        assert((sdigit) atop + zhi == -1 || (sdigit) atop + zhi == 0);

        // If we subtracted too much, add back.
        if ((sdigit) atop + zhi < 0) [[unlikely]] {
            digit carry = 0;
            For(range(sizeb)) {
                carry += get_digit(&na, it + j) + get_digit(&nb, it);
                set_digit(&na, it + j, carry & MASK);
                carry >>= SHIFT;
            }
            --qhat;
        }

        // Store quotient digit
        assert(qhat < BASE);
        set_digit(&q, j, qhat);
    }

    // Unnormalize the remainder, which was stored in _na_ during the computation.
    na.Size       = sizeb;
    big_integer r = rshift(na, s);

    normalize(&q);
    normalize(&r);

    return {q, r};
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
constexpr big_integer bitwise(big_integer lhs, byte op, big_integer rhs) {
    // Bitwise operations for negative numbers operate as though
    // on a two's complement representation. So convert arguments
    // from sign-magnitude to two's complement, and convert the
    // result back to sign-magnitude at the end.

    s64 sizea = abs(lhs.Size), sizeb = abs(rhs.Size);
    bool negLhs = lhs.Size < 0, negRhs = rhs.Size < 0;

    big_integer aComp, bComp;

    big_integer *a = &lhs, *b = &rhs;

    if (negLhs) {
        ensure_digits(&aComp, sizea);
        aComp.Size = sizea;

        v_complement(get_digits(&aComp), get_digits(&lhs), sizea);
        a = &aComp;
    }

    if (negRhs) {
        ensure_digits(&bComp, sizeb);
        bComp.Size = sizeb;

        v_complement(get_digits(&bComp), get_digits(&rhs), sizeb);
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

    // We allow an extra digit if z is negative, to make sure that
    // the final two's complement of z doesn't overflow.
    big_integer result = create_big_integer_and_set_size(sizea + (negZ ? 1 : 0));

    s64 i = 0;

    // Compute digits for overlap of a and b
    if (op == '&') {
        while (i < sizeb) {
            set_digit(&result, i, get_digit(a, i) & get_digit(b, i));
            ++i;
        }
    } else if (op == '|') {
        while (i < sizeb) {
            set_digit(&result, i, get_digit(a, i) | get_digit(b, i));
            ++i;
        }
    } else if (op == '^') {
        while (i < sizeb) {
            set_digit(&result, i, get_digit(a, i) ^ get_digit(b, i));
            ++i;
        }
    } else {
        assert(false);
    }

    // Copy any remaining digits of a, inverting if necessary
    if (op == '^' && negRhs) {
        while (i < sizez) {
            set_digit(&result, i, get_digit(a, i) ^ MASK);
            ++i;
        }
    } else if (i < sizez) {
        copy_memory(get_digits(&result) + i, get_digits(a) + i, (sizez - i) * sizeof(digit));
    }

    // Complement result if negative
    if (negZ) {
        result.Size = -result.Size;
        set_digit(&result, sizez, MASK);
        v_complement(get_digits(&result), get_digits(&result), sizez + 1);
    }

    normalize(&result);
    return result;
}

constexpr big_integer operator&(big_integer lhs, big_integer rhs) { return bitwise(lhs, '&', rhs); }
constexpr big_integer operator|(big_integer lhs, big_integer rhs) { return bitwise(lhs, '|', rhs); }
constexpr big_integer operator^(big_integer lhs, big_integer rhs) { return bitwise(lhs, '^', rhs); }

constexpr big_integer add_one(big_integer b) {
    if (!b.Size) return cast_big(1);

    s64 size = abs(b.Size);

    digit carry = (get_digit(&b, size) + 1) >> SHIFT;
    if (carry) ++size;

    big_integer result = create_big_integer_and_set_size(size);
    copy_memory(get_digits(&result), get_digits(&b), abs(b.Size) * sizeof(digit));

    if (carry) {
        set_digit(&result, size - 2, 0);
        set_digit(&result, size - 1, 1);
    } else {
        // set_digit(&result, size - 1, get_digit(&result, size - 1) + 1);
        ++*(get_digits(&result) + size - 1);
    }
    return result;
}

constexpr big_integer invert(big_integer b) {
    big_integer result = add_one(b);
    result.Size        = -result.Size;
    return result;
}

constexpr big_integer lshift(big_integer lhs, s64 n) {
    if (n == 0) return lhs;
    if (!lhs.Size) return cast_big(0);

    s64 wordshift = n / SHIFT;
    u32 remshift  = n % SHIFT;

    // This version due to Tim Peters

    s64 oldSize = abs(lhs.Size);
    s64 newSize = oldSize + wordshift;

    if (remshift) ++newSize;

    big_integer result = create_big_integer(newSize);
    if (lhs.Size < 0) {
        result.Size = -newSize;
    } else {
        result.Size = newSize;
    }

    For(range(wordshift)) {
        set_digit(&result, it, 0);
    }

    s64 accum = 0;

    s64 i, j;
    for (i = wordshift, j = 0; j < oldSize; ++i, ++j) {
        accum |= (double_digit) get_digit(&lhs, j) << remshift;
        set_digit(&result, i, (digit) (accum & MASK));
        accum >>= SHIFT;
    }

    if (remshift) {
        set_digit(&result, newSize - 1, (digit) accum);
    } else {
        assert(!accum);
    }

    normalize(&result);
    return result;
}

constexpr big_integer rshift(big_integer lhs, s64 n) {
    if (n == 0) return lhs;
    if (!lhs.Size) return cast_big(0);

    s64 wordshift = n / SHIFT;
    u32 remshift  = n % SHIFT;

    if (lhs.Size < 0) {
        // Right shifting negative numbers is harder
        return invert(rshift(invert(lhs), n));
    } else {
        s64 newSize = lhs.Size - wordshift;
        if (newSize <= 0) return cast_big(0);

        s64 hishift  = SHIFT - remshift;
        digit lomask = (1ul << hishift) - 1;
        digit himask = MASK ^ lomask;

        big_integer result = create_big_integer_and_set_size(newSize);

        s64 i = 0, j = wordshift;
        while (i < newSize) {
            set_digit(&result, i, (get_digit(&lhs, j) >> remshift) & lomask);
            if (i + 1 < newSize) {
                auto *d = get_digits(&result) + i;
                *d |= (get_digit(&lhs, j + 1) << hishift) & himask;
            }
            ++i, ++j;
        }
        normalize(&result);
        return result;
    }
}

constexpr div_result divrem(big_integer lhs, big_integer rhs) {
    if (!rhs.Size) panic("Division by zero");

    s64 sizea = abs(lhs.Size), sizeb = abs(rhs.Size);

    div_result result;

    if (sizea < sizeb || (sizea == sizeb && (get_digit(&lhs, sizea - 1) < get_digit(&rhs, sizeb - 1)))) {
        // |lhs| < |rhs|
        result.Q = cast_big(0);
        result.R = lhs;
        return result;
    }

    //
    // |lhs| >= |rhs|
    //

    if (sizeb == 1) {
        auto [div, rem] = divrem1(lhs, get_digit(&rhs, 0));
        result.Q        = div;
        result.R        = cast_big(rem);
    } else {
        result = x_divrem(lhs, rhs);
    }

    // Set the signs.
    // The quotient z has the sign of lhs * rhs;
    // the remainder r has the sign of lhs, so lhs = rhs * z + r.
    if ((lhs.Size < 0) != (rhs.Size < 0)) {
        result.Q.Size = -result.Q.Size;
    }
    if (lhs.Size < 0 && result.R.Size != 0) {
        result.R.Size = -result.R.Size;
    }
    return result;
}

constexpr div_result divmod(big_integer lhs, big_integer rhs) {
    auto [div, mod] = divrem(lhs, rhs);

    // To get from rem to mod, we have to add rhs if lhs and rhs
    // have different signs. We then subtract one from the 'div'
    // part of the outcome to keep the invariant intact.
    if ((mod.Size < 0 && rhs.Size > 0) || (mod.Size > 0 && rhs.Size < 0)) {
        return {div - cast_big(1), rhs + mod};
    } else {
        return {div, mod};
    }
}

constexpr s32 compare(big_integer a, big_integer b_) {
    big_integer b = cast_big(b_);

    s32 sign = (s32) (a.Size - b.Size);
    if (sign == 0) {
        if (!a.Size) return 0;  // Two zeroes

        s64 i       = abs(a.Size);
        sdigit diff = 0;
        while (--i >= 0) {
            diff = (sdigit) get_digit(&a, i) - (sdigit) get_digit(&b, i);
            if (diff) break;
        }
        sign = a.Size < 0 ? -1 : 1;
    }
    return sign;
}

constexpr big_integer operator+(big_integer lhs, big_integer rhs) {
    if (lhs.Size < 0) {
        if (rhs.Size < 0) {
            auto z = x_add(&lhs, &rhs);
            z.Size = -z.Size;
            return z;
        } else {
            return x_sub(&rhs, &lhs);
        }
    } else {
        if (rhs.Size < 0) {
            return x_sub(&lhs, &rhs);
        } else {
            return x_add(&lhs, &rhs);
        }
    }
}

constexpr big_integer operator-(big_integer lhs, big_integer rhs) {
    if (lhs.Size < 0) {
        if (rhs.Size < 0) {
            return x_sub(&rhs, &lhs);
        } else {
            auto z = x_add(&lhs, &rhs);
            z.Size = -z.Size;
            return z;
        }
    } else {
        if (rhs.Size < 0) {
            return x_add(&lhs, &rhs);
        } else {
            return x_sub(&lhs, &rhs);
        }
    }
}

constexpr big_integer operator*(big_integer lhs, big_integer rhs) {
    big_integer result = k_mul(&lhs, &rhs);
    // Negate if exactly one of the inputs is negative
    if ((lhs.Size ^ rhs.Size) < 0 && result.Size) {
        result.Size = -result.Size;
    }

    return result;
}

constexpr big_integer operator/(big_integer lhs, big_integer rhs) {
    auto [div, rem] = divrem(lhs, rhs);
    return div;
}

constexpr big_integer operator%(big_integer lhs, big_integer rhs) {
    auto [div, mod] = divmod(lhs, rhs);
    return mod;
}

LSTD_END_NAMESPACE
