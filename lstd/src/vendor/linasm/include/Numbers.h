/*                                                                     Numbers.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                         NUMBERS CONVERSION FUNCTIONS                         #
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
class Numbers
{
public:

//****************************************************************************//
//      Binary numbers conversion                                             //
//****************************************************************************//

// Unsigned integer types
static u32 BinToNum (u8 *number, const utf8 string[]);
static u32 BinToNum (u16 *number, const utf8 string[]);
static u32 BinToNum (u32 *number, const utf8 string[]);
static u32 BinToNum (u64 *number, const utf8 string[]);

// Signed integer types
static u32 BinToNum (s8 *number, const utf8 string[]);
static u32 BinToNum (s16 *number, const utf8 string[]);
static u32 BinToNum (s32 *number, const utf8 string[]);
static u32 BinToNum (s64 *number, const utf8 string[]);

// Other types
static u32 BinToNum (u32 *number, const utf8 string[]);

//****************************************************************************//
//      Octal numbers conversion                                              //
//****************************************************************************//

// Unsigned integer types
static u32 OctToNum (u8 *number, const utf8 string[]);
static u32 OctToNum (u16 *number, const utf8 string[]);
static u32 OctToNum (u32 *number, const utf8 string[]);
static u32 OctToNum (u64 *number, const utf8 string[]);

// Signed integer types
static u32 OctToNum (s8 *number, const utf8 string[]);
static u32 OctToNum (s16 *number, const utf8 string[]);
static u32 OctToNum (s32 *number, const utf8 string[]);
static u32 OctToNum (s64 *number, const utf8 string[]);

// Other types
static u32 OctToNum (u32 *number, const utf8 string[]);

//****************************************************************************//
//      Hexadecimal numbers conversion                                        //
//****************************************************************************//

// Unsigned integer types
static u32 HexToNum (u8 *number, const utf8 string[]);
static u32 HexToNum (u16 *number, const utf8 string[]);
static u32 HexToNum (u32 *number, const utf8 string[]);
static u32 HexToNum (u64 *number, const utf8 string[]);

// Signed integer types
static u32 HexToNum (s8 *number, const utf8 string[]);
static u32 HexToNum (s16 *number, const utf8 string[]);
static u32 HexToNum (s32 *number, const utf8 string[]);
static u32 HexToNum (s64 *number, const utf8 string[]);

// Floating-point types
static u32 HexToNum (f32 *number, const utf8 string[]);
static u32 HexToNum (f64 *number, const utf8 string[]);

// Other types
static u32 HexToNum (u32 *number, const utf8 string[]);

//****************************************************************************//
//      Decimal numbers conversion                                            //
//****************************************************************************//

// Unsigned integer types
static u32 DecToNum (u8 *number, const utf8 string[]);
static u32 DecToNum (u16 *number, const utf8 string[]);
static u32 DecToNum (u32 *number, const utf8 string[]);
static u32 DecToNum (u64 *number, const utf8 string[]);

// Signed integer types
static u32 DecToNum (s8 *number, const utf8 string[]);
static u32 DecToNum (s16 *number, const utf8 string[]);
static u32 DecToNum (s32 *number, const utf8 string[]);
static u32 DecToNum (s64 *number, const utf8 string[]);

// Floating-point types
static u32 DecToNum (f32 *number, const utf8 string[]);
static u32 DecToNum (f64 *number, const utf8 string[]);

// Other types
static u32 DecToNum (u32 *number, const utf8 string[]);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Binary numbers conversion                                             //
//****************************************************************************//

// Unsigned integer types
u32 Numbers_BinToNum_uint8 (u8 *number, const utf8 string[]);
u32 Numbers_BinToNum_uint16 (u16 *number, const utf8 string[]);
u32 Numbers_BinToNum_uint32 (u32 *number, const utf8 string[]);
u32 Numbers_BinToNum_uint64 (u64 *number, const utf8 string[]);

// Signed integer types
u32 Numbers_BinToNum_sint8 (s8 *number, const utf8 string[]);
u32 Numbers_BinToNum_sint16 (s16 *number, const utf8 string[]);
u32 Numbers_BinToNum_sint32 (s32 *number, const utf8 string[]);
u32 Numbers_BinToNum_sint64 (s64 *number, const utf8 string[]);

// Other types
u32 Numbers_BinToNum_size (u32 *number, const utf8 string[]);

//****************************************************************************//
//      Octal numbers conversion                                              //
//****************************************************************************//

// Unsigned integer types
u32 Numbers_OctToNum_uint8 (u8 *number, const utf8 string[]);
u32 Numbers_OctToNum_uint16 (u16 *number, const utf8 string[]);
u32 Numbers_OctToNum_uint32 (u32 *number, const utf8 string[]);
u32 Numbers_OctToNum_uint64 (u64 *number, const utf8 string[]);

// Signed integer types
u32 Numbers_OctToNum_sint8 (s8 *number, const utf8 string[]);
u32 Numbers_OctToNum_sint16 (s16 *number, const utf8 string[]);
u32 Numbers_OctToNum_sint32 (s32 *number, const utf8 string[]);
u32 Numbers_OctToNum_sint64 (s64 *number, const utf8 string[]);

// Other types
u32 Numbers_OctToNum_size (u32 *number, const utf8 string[]);

//****************************************************************************//
//      Hexadecimal numbers conversion                                        //
//****************************************************************************//

// Unsigned integer types
u32 Numbers_HexToNum_uint8 (u8 *number, const utf8 string[]);
u32 Numbers_HexToNum_uint16 (u16 *number, const utf8 string[]);
u32 Numbers_HexToNum_uint32 (u32 *number, const utf8 string[]);
u32 Numbers_HexToNum_uint64 (u64 *number, const utf8 string[]);

// Signed integer types
u32 Numbers_HexToNum_sint8 (s8 *number, const utf8 string[]);
u32 Numbers_HexToNum_sint16 (s16 *number, const utf8 string[]);
u32 Numbers_HexToNum_sint32 (s32 *number, const utf8 string[]);
u32 Numbers_HexToNum_sint64 (s64 *number, const utf8 string[]);

// Floating-point types
u32 Numbers_HexToNum_flt32 (f32 *number, const utf8 string[]);
u32 Numbers_HexToNum_flt64 (f64 *number, const utf8 string[]);

// Other types
u32 Numbers_HexToNum_size (u32 *number, const utf8 string[]);

//****************************************************************************//
//      Decimal numbers conversion                                            //
//****************************************************************************//

// Unsigned integer types
u32 Numbers_DecToNum_uint8 (u8 *number, const utf8 string[]);
u32 Numbers_DecToNum_uint16 (u16 *number, const utf8 string[]);
u32 Numbers_DecToNum_uint32 (u32 *number, const utf8 string[]);
u32 Numbers_DecToNum_uint64 (u64 *number, const utf8 string[]);

// Signed integer types
u32 Numbers_DecToNum_sint8 (s8 *number, const utf8 string[]);
u32 Numbers_DecToNum_sint16 (s16 *number, const utf8 string[]);
u32 Numbers_DecToNum_sint32 (s32 *number, const utf8 string[]);
u32 Numbers_DecToNum_sint64 (s64 *number, const utf8 string[]);

// Floating-point types
u32 Numbers_DecToNum_flt32 (f32 *number, const utf8 string[]);
u32 Numbers_DecToNum_flt64 (f64 *number, const utf8 string[]);

// Other types
u32 Numbers_DecToNum_size (u32 *number, const utf8 string[]);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
