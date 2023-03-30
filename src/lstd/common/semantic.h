#pragma once

///
/// @brief File for some semantic sugar.
///

//
// Some personal preferences:
//
// I prefer to type null over nullptr but they are exactly the same
using null_t = decltype(nullptr);
inline const null_t null = nullptr;

#ifndef NULL
#define NULL 0
#endif

// Semantics to avoid the use of & when the symbol is not used as a unary or
// binary operator.
//
// e.g.
//      void print_array_to_file(array<u8> no_copy bytes) { ... }
//      void modify_array(array<u8> ref bytes) { ... }
//
#define no_copy const &
#define ref &

// Used to mark functions for which the caller is supposed to free the result.
// This at leasts makes the compiler warn the caller if they've decided to
// discard the result.
//
// e.g.
//		mark_as_leak string make_string(...) { ... }
//
#define mark_as_leak [[nodiscard("Leak")]]

//
// Personal preference
// e.g. cast(int) 5.0
//
#define cast(x) (x)

// Helper macro for, e.g flag enums
//
// enum flags {
//	Flag_1 = BIT(0),
//  Flag_1 = BIT(1)
//  Flag_1 = BIT(2)
//  ...
// };
//
#define BIT(x) (1 << (x))

// Gives the offset of a member in a struct (in bytes)
#define offset_of(s, field) ((u64) & ((s *)(0))->field)
