/*                                                                    Sequence.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                       SUBSEQUENCE SEARCHING ALGORITHMS                       #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Types.h>

// Since the original library is compiled for Linux, the assembly source code contains
// mangled names in the linux convention (they aren't compatible with MSVC). We can write
// a script to convert between the two and update the .asm source files with the new names.
// This would take a while and I'm not sure how to match the unmangled names yet.
// @TODO: This can be automated with Python. It would just take a while and I'm in the mood at the moment.
// I'm tired of Microsoft's BS already on a separate note.
// :WeAreDoingCNamesInsteadOfC++ButEventuallyWeWillDoC++:
//
// # ifdef	__cplusplus
#ifdef EVENTUALLY_DO_MANGLED
/*
################################################################################
#       C++ prototypes                                                         #
################################################################################
*/
//****************************************************************************//
//      Boyer-Moore-Horspool pattern hash                                     //
//****************************************************************************//
class BMH
{
private:
	const u32	hash [256];		// BMH hash
	const void		*pattern;		// Pattern
	const u32	size;			// Size of pattern
	const bool		backward;		// Flag to search in backward direction

public:

// Unsigned integer types
BMH (const u8 pattern[], u32 size, bool backward);
BMH (const u16 pattern[], u32 size, bool backward);
BMH (const u32 pattern[], u32 size, bool backward);
BMH (const u64 pattern[], u32 size, bool backward);

// Signed integer types
BMH (const s8 pattern[], u32 size, bool backward);
BMH (const s16 pattern[], u32 size, bool backward);
BMH (const s32 pattern[], u32 size, bool backward);
BMH (const s64 pattern[], u32 size, bool backward);

// Other types
BMH (const u32 pattern[], u32 size, bool backward);
BMH (const void *pattern, u32 size, bool backward);
};

//****************************************************************************//
//      Boyer-Moore-Horspool subsequence searching algorithm                  //
//****************************************************************************//
class Sequence
{
public:

// Unsigned integer types
static u32 Find (const u8 source[], u32 size, const BMH *pattern);
static u32 Find (const u16 source[], u32 size, const BMH *pattern);
static u32 Find (const u32 source[], u32 size, const BMH *pattern);
static u32 Find (const u64 source[], u32 size, const BMH *pattern);

// Signed integer types
static u32 Find (const s8 source[], u32 size, const BMH *pattern);
static u32 Find (const s16 source[], u32 size, const BMH *pattern);
static u32 Find (const s32 source[], u32 size, const BMH *pattern);
static u32 Find (const s64 source[], u32 size, const BMH *pattern);

// Other types
static u32 Find (const u32 source[], u32 size, const BMH *pattern);
static u32 Find (const void *source, u32 size, const BMH *pattern);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Boyer-Moore-Horspool pattern hash                                     //
//****************************************************************************//
struct BMH
{
	const u32	hash [256];		// BMH hash
	const void		*pattern;		// Pattern
	const u32	size;			// Size of pattern
	const bool		backward;		// Flag to search in backward direction
};

// Unsigned integer types
void BMH_uint8 (struct BMH *hash, const u8 pattern[], u32 size, bool backward);
void BMH_uint16 (struct BMH *hash, const u16 pattern[], u32 size, bool backward);
void BMH_uint32 (struct BMH *hash, const u32 pattern[], u32 size, bool backward);
void BMH_uint64 (struct BMH *hash, const u64 pattern[], u32 size, bool backward);

// Signed integer types
void BMH_sint8 (struct BMH *hash, const s8 pattern[], u32 size, bool backward);
void BMH_sint16 (struct BMH *hash, const s16 pattern[], u32 size, bool backward);
void BMH_sint32 (struct BMH *hash, const s32 pattern[], u32 size, bool backward);
void BMH_sint64 (struct BMH *hash, const s64 pattern[], u32 size, bool backward);

// Other types
void BMH_size (struct BMH *hash, const u32 pattern[], u32 size, bool backward);
void BMH (struct BMH *hash, const void *pattern, u32 size, bool backward);

//****************************************************************************//
//      Boyer-Moore-Horspool subsequence searching algorithm                  //
//****************************************************************************//

// Unsigned integer types
u32 Sequence_Find_uint8 (const u8 source[], u32 size, const struct BMH *pattern);
u32 Sequence_Find_uint16 (const u16 source[], u32 size, const struct BMH *pattern);
u32 Sequence_Find_uint32 (const u32 source[], u32 size, const struct BMH *pattern);
u32 Sequence_Find_uint64 (const u64 source[], u32 size, const struct BMH *pattern);

// Signed integer types
u32 Sequence_Find_sint8 (const s8 source[], u32 size, const struct BMH *pattern);
u32 Sequence_Find_sint16 (const s16 source[], u32 size, const struct BMH *pattern);
u32 Sequence_Find_sint32 (const s32 source[], u32 size, const struct BMH *pattern);
u32 Sequence_Find_sint64 (const s64 source[], u32 size, const struct BMH *pattern);

// Other types
u32 Sequence_Find_size (const u32 source[], u32 size, const struct BMH *pattern);
u32 Sequence_Find (const void *source, u32 size, const struct BMH *pattern);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
