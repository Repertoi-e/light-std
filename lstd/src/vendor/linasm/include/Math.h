/*                                                                        Math.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                             FAST MATH FUNCTIONS                              #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Types.h>

//****************************************************************************//
//      Mathematical constants                                                //
//****************************************************************************//

// Basic constants
# define	M_E			2.7182818284590452353602874713526625	// Base of natural logarithm
# define	M_PI		3.1415926535897932384626433832795029	// Pi const
# define	M_FI		1.6180339887498948482045868343656381	// The "Golden Ratio"
# define	M_1_FI		0.6180339887498948482045868343656381	// The inverse "Golden Ratio"

// Logarithmic constants
# define	M_LN_2		0.6931471805599453094172321214581766	// Natural logarithm of 2
# define	M_LN_10		2.3025850929940456840179914546843642	// Natural logarithm of 10
# define	M_LOG2_10	3.3219280948873623478703194294893902	// Binary logarithm of 10
# define	M_LOG2_E	1.4426950408889634073599246810018921	// Binary logarithm of e
# define	M_LOG10_2	0.3010299956639811952137388947244930	// Decimal logarithm of 2
# define	M_LOG10_E	0.4342944819032518276511289189166051	// Decimal logarithm of e

// Square root constants
# define	M_SQRT2		1.4142135623730950488016887242096981	// Square root of 2
# define	M_SQRT1_2	0.7071067811865475244008443621048490	// Square root of 1/2

// Special constants
# define	M_INF		(1.0 / 0.0)								// Infinity
# define	M_NAN		(0.0 / 0.0)								// Not a Number

// Since the original library is compiled for Linux, the assembly source code contains 
// mangled names in the linux convention (they aren't compatible with MSVC). We can write
// a script to convert between the two and update the .asm source files with the new names.
// This would take a while and I'm not sure how to match the unmangled names yet.
// @TODO: This can be automated with Python. It would just take a while and I'm in the mood at the moment.
// I'm tired of Microsoft's BS already on a separate note.
// :WeAreDoingCNamesInsteadOfC++ButEventuallyWeWillDoC++: 
// 
// # ifdef	__cplusplus
# ifdef EVENTUALLY_DO_MANGLED
/*
################################################################################
#       C++ prototypes                                                         #
################################################################################
*/
class Math
{
public:

//****************************************************************************//
//      Bitwise operations                                                    //
//****************************************************************************//

//============================================================================//
//      Byte swap                                                             //
//============================================================================//

// Unsigned integer types
static u8 ByteSwap (u8 value);
static u16 ByteSwap (u16 value);
static u32 ByteSwap (u32 value);
static u64 ByteSwap (u64 value);
static u8v16 ByteSwap (u8v16 value);
static u16v8 ByteSwap (u16v8 value);
static u32v4 ByteSwap (u32v4 value);
static u64v2 ByteSwap (u64v2 value);

// Signed integer types
static s8 ByteSwap (s8 value);
static s16 ByteSwap (s16 value);
static s32 ByteSwap (s32 value);
static s64 ByteSwap (s64 value);
static s8v16 ByteSwap (s8v16 value);
static s16v8 ByteSwap (s16v8 value);
static s32v4 ByteSwap (s32v4 value);
static s64v2 ByteSwap (s64v2 value);

//============================================================================//
//      Bit reversal permutation                                              //
//============================================================================//

// Unsigned integer types
static u8 BitReverse (u8 value);
static u16 BitReverse (u16 value);
static u32 BitReverse (u32 value);
static u64 BitReverse (u64 value);
static u8v16 BitReverse (u8v16 value);
static u16v8 BitReverse (u16v8 value);
static u32v4 BitReverse (u32v4 value);
static u64v2 BitReverse (u64v2 value);

// Signed integer types
static s8 BitReverse (s8 value);
static s16 BitReverse (s16 value);
static s32 BitReverse (s32 value);
static s64 BitReverse (s64 value);
static s8v16 BitReverse (s8v16 value);
static s16v8 BitReverse (s16v8 value);
static s32v4 BitReverse (s32v4 value);
static s64v2 BitReverse (s64v2 value);

//============================================================================//
//      Bit scan                                                              //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Bit scan forward                                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u8 ScanForward (u8 value);
static u16 ScanForward (u16 value);
static u32 ScanForward (u32 value);
static u64 ScanForward (u64 value);

// Unsigned integer types
static s8 ScanForward (s8 value);
static s16 ScanForward (s16 value);
static s32 ScanForward (s32 value);
static s64 ScanForward (s64 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Bit scan backward                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u8 ScanBackward (u8 value);
static u16 ScanBackward (u16 value);
static u32 ScanBackward (u32 value);
static u64 ScanBackward (u64 value);

// Unsigned integer types
static s8 ScanBackward (s8 value);
static s16 ScanBackward (s16 value);
static s32 ScanBackward (s32 value);
static s64 ScanBackward (s64 value);

//============================================================================//
//      Circular rotation                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Circular rotation to the left                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u8 RotateLeft (u8 value, u8 shift);
static u16 RotateLeft (u16 value, u8 shift);
static u32 RotateLeft (u32 value, u8 shift);
static u64 RotateLeft (u64 value, u8 shift);

// Signed integer types
static s8 RotateLeft (s8 value, u8 shift);
static s16 RotateLeft (s16 value, u8 shift);
static s32 RotateLeft (s32 value, u8 shift);
static s64 RotateLeft (s64 value, u8 shift);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Circular rotation to the right                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u8 RotateRight (u8 value, u8 shift);
static u16 RotateRight (u16 value, u8 shift);
static u32 RotateRight (u32 value, u8 shift);
static u64 RotateRight (u64 value, u8 shift);

// Signed integer types
static s8 RotateRight (s8 value, u8 shift);
static s16 RotateRight (s16 value, u8 shift);
static s32 RotateRight (s32 value, u8 shift);
static s64 RotateRight (s64 value, u8 shift);

//============================================================================//
//      Population count                                                      //
//============================================================================//

// Unsigned integer types
static u8 PopCount (u8 value);
static u16 PopCount (u16 value);
static u32 PopCount (u32 value);
static u64 PopCount (u64 value);
static u8v16 PopCount (u8v16 value);
static u16v8 PopCount (u16v8 value);
static u32v4 PopCount (u32v4 value);
static u64v2 PopCount (u64v2 value);

// Signed integer types
static s8 PopCount (s8 value);
static s16 PopCount (s16 value);
static s32 PopCount (s32 value);
static s64 PopCount (s64 value);
static s8v16 PopCount (s8v16 value);
static s16v8 PopCount (s16v8 value);
static s32v4 PopCount (s32v4 value);
static s64v2 PopCount (s64v2 value);

//****************************************************************************//
//      Arithmetic operations                                                 //
//****************************************************************************//

//============================================================================//
//      Absolute value                                                        //
//============================================================================//

// Signed integer types
static u8 Abs (s8 value);
static u16 Abs (s16 value);
static u32 Abs (s32 value);
static u64 Abs (s64 value);
static u8v16 Abs (s8v16 value);
static u16v8 Abs (s16v8 value);
static u32v4 Abs (s32v4 value);
static u64v2 Abs (s64v2 value);

// Floating-point types
static f32 Abs (f32 value);
static f64 Abs (f64 value);
static f32v4 Abs (f32v4 value);
static f64v2 Abs (f64v2 value);

//============================================================================//
//      Negative absolute value                                               //
//============================================================================//

// Signed integer types
static s8 NegAbs (s8 value);
static s16 NegAbs (s16 value);
static s32 NegAbs (s32 value);
static s64 NegAbs (s64 value);
static s8v16 NegAbs (s8v16 value);
static s16v8 NegAbs (s16v8 value);
static s32v4 NegAbs (s32v4 value);
static s64v2 NegAbs (s64v2 value);

// Floating-point types
static f32 NegAbs (f32 value);
static f64 NegAbs (f64 value);
static f32v4 NegAbs (f32v4 value);
static f64v2 NegAbs (f64v2 value);

//============================================================================//
//      Number sign                                                           //
//============================================================================//

// Signed integer types
static s8 Sign (s8 value);
static s16 Sign (s16 value);
static s32 Sign (s32 value);
static s64 Sign (s64 value);
static s8v16 Sign (s8v16 value);
static s16v8 Sign (s16v8 value);
static s32v4 Sign (s32v4 value);
static s64v2 Sign (s64v2 value);

// Floating-point types
static f32 Sign (f32 value);
static f64 Sign (f64 value);
static f32v4 Sign (f32v4 value);
static f64v2 Sign (f64v2 value);

//============================================================================//
//      Square root                                                           //
//============================================================================//

// Unsigned integer types
static u8 Sqrt (u8 value);
static u16 Sqrt (u16 value);
static u32 Sqrt (u32 value);
static u64 Sqrt (u64 value);

// Floating-point types
static f32 Sqrt (f32 value);
static f64 Sqrt (f64 value);
static f32v4 Sqrt (f32v4 value);
static f64v2 Sqrt (f64v2 value);

//============================================================================//
//      Square value                                                          //
//============================================================================//

// Unsigned integer types
static u8 Sqr (u8 value);
static u16 Sqr (u16 value);
static u32 Sqr (u32 value);
static u64 Sqr (u64 value);

// Signed integer types
static s8 Sqr (s8 value);
static s16 Sqr (s16 value);
static s32 Sqr (s32 value);
static s64 Sqr (s64 value);

// Floating-point types
static f32 Sqr (f32 value);
static f64 Sqr (f64 value);
static f32v4 Sqr (f32v4 value);
static f64v2 Sqr (f64v2 value);

//============================================================================//
//      Cube value                                                            //
//============================================================================//

// Unsigned integer types
static u8 Cube (u8 value);
static u16 Cube (u16 value);
static u32 Cube (u32 value);
static u64 Cube (u64 value);

// Signed integer types
static s8 Cube (s8 value);
static s16 Cube (s16 value);
static s32 Cube (s32 value);
static s64 Cube (s64 value);

// Floating-point types
static f32 Cube (f32 value);
static f64 Cube (f64 value);
static f32v4 Cube (f32v4 value);
static f64v2 Cube (f64v2 value);

//============================================================================//
//      Inverse value                                                         //
//============================================================================//
static f32 InverseValue (f32 value);
static f64 InverseValue (f64 value);
static f32v4 InverseValue (f32v4 value);
static f64v2 InverseValue (f64v2 value);

//============================================================================//
//      Inverse square value                                                  //
//============================================================================//
static f32 InverseSquare (f32 value);
static f64 InverseSquare (f64 value);
static f32v4 InverseSquare (f32v4 value);
static f64v2 InverseSquare (f64v2 value);

//============================================================================//
//      Inverse cube value                                                    //
//============================================================================//
static f32 InverseCube (f32 value);
static f64 InverseCube (f64 value);
static f32v4 InverseCube (f32v4 value);
static f64v2 InverseCube (f64v2 value);

//============================================================================//
//      Three-state comparison                                                //
//============================================================================//

// Unsigned integer types
static s8 Compare (u8 value1, u8 value2);
static s16 Compare (u16 value1, u16 value2);
static s32 Compare (u32 value1, u32 value2);
static s64 Compare (u64 value1, u64 value2);
static s8v16 Compare (u8v16 value1, u8v16 value2);
static s16v8 Compare (u16v8 value1, u16v8 value2);
static s32v4 Compare (u32v4 value1, u32v4 value2);
static s64v2 Compare (u64v2 value1, u64v2 value2);

// Signed integer types
static s8 Compare (s8 value1, s8 value2);
static s16 Compare (s16 value1, s16 value2);
static s32 Compare (s32 value1, s32 value2);
static s64 Compare (s64 value1, s64 value2);
static s8v16 Compare (s8v16 value1, s8v16 value2);
static s16v8 Compare (s16v8 value1, s16v8 value2);
static s32v4 Compare (s32v4 value1, s32v4 value2);
static s64v2 Compare (s64v2 value1, s64v2 value2);

// Floating-point types
static f32 Compare (f32 value1, f32 value2);
static f64 Compare (f64 value1, f64 value2);
static f32v4 Compare (f32v4 value1, f32v4 value2);
static f64v2 Compare (f64v2 value1, f64v2 value2);

//============================================================================//
//      Minimum and maximum absolute value                                    //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Minimum absolute value                                                //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
static u8 MinAbs (s8 value1, s8 value2);
static u16 MinAbs (s16 value1, s16 value2);
static u32 MinAbs (s32 value1, s32 value2);
static u64 MinAbs (s64 value1, s64 value2);
static u8v16 MinAbs (s8v16 value1, s8v16 value2);
static u16v8 MinAbs (s16v8 value1, s16v8 value2);
static u32v4 MinAbs (s32v4 value1, s32v4 value2);

// Floating-point types
static f32 MinAbs (f32 value1, f32 value2);
static f64 MinAbs (f64 value1, f64 value2);
static f32v4 MinAbs (f32v4 value1, f32v4 value2);
static f64v2 MinAbs (f64v2 value1, f64v2 value2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Maximum absolute value                                                //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
static u8 MaxAbs (s8 value1, s8 value2);
static u16 MaxAbs (s16 value1, s16 value2);
static u32 MaxAbs (s32 value1, s32 value2);
static u64 MaxAbs (s64 value1, s64 value2);
static u8v16 MaxAbs (s8v16 value1, s8v16 value2);
static u16v8 MaxAbs (s16v8 value1, s16v8 value2);
static u32v4 MaxAbs (s32v4 value1, s32v4 value2);

// Floating-point types
static f32 MaxAbs (f32 value1, f32 value2);
static f64 MaxAbs (f64 value1, f64 value2);
static f32v4 MaxAbs (f32v4 value1, f32v4 value2);
static f64v2 MaxAbs (f64v2 value1, f64v2 value2);

//============================================================================//
//      Minimum and maximum value                                             //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Minimum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u8 Min (u8 value1, u8 value2);
static u16 Min (u16 value1, u16 value2);
static u32 Min (u32 value1, u32 value2);
static u64 Min (u64 value1, u64 value2);
static u8v16 Min (u8v16 value1, u8v16 value2);
static u16v8 Min (u16v8 value1, u16v8 value2);
static u32v4 Min (u32v4 value1, u32v4 value2);

// Signed integer types
static s8 Min (s8 value1, s8 value2);
static s16 Min (s16 value1, s16 value2);
static s32 Min (s32 value1, s32 value2);
static s64 Min (s64 value1, s64 value2);
static s8v16 Min (s8v16 value1, s8v16 value2);
static s16v8 Min (s16v8 value1, s16v8 value2);
static s32v4 Min (s32v4 value1, s32v4 value2);

// Floating-point types
static f32 Min (f32 value1, f32 value2);
static f64 Min (f64 value1, f64 value2);
static f32v4 Min (f32v4 value1, f32v4 value2);
static f64v2 Min (f64v2 value1, f64v2 value2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Maximum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u8 Max (u8 value1, u8 value2);
static u16 Max (u16 value1, u16 value2);
static u32 Max (u32 value1, u32 value2);
static u64 Max (u64 value1, u64 value2);
static u8v16 Max (u8v16 value1, u8v16 value2);
static u16v8 Max (u16v8 value1, u16v8 value2);
static u32v4 Max (u32v4 value1, u32v4 value2);

// Signed integer types
static s8 Max (s8 value1, s8 value2);
static s16 Max (s16 value1, s16 value2);
static s32 Max (s32 value1, s32 value2);
static s64 Max (s64 value1, s64 value2);
static s8v16 Max (s8v16 value1, s8v16 value2);
static s16v8 Max (s16v8 value1, s16v8 value2);
static s32v4 Max (s32v4 value1, s32v4 value2);

// Floating-point types
static f32 Max (f32 value1, f32 value2);
static f64 Max (f64 value1, f64 value2);
static f32v4 Max (f32v4 value1, f32v4 value2);
static f64v2 Max (f64v2 value1, f64v2 value2);

//============================================================================//
//      Greatest common divisor                                               //
//============================================================================//

// Unsigned integer types
static u8 GCD (u8 value1, u8 value2);
static u16 GCD (u16 value1, u16 value2);
static u32 GCD (u32 value1, u32 value2);
static u64 GCD (u64 value1, u64 value2);

// Signed integer types
static u8 GCD (s8 value1, s8 value2);
static u16 GCD (s16 value1, s16 value2);
static u32 GCD (s32 value1, s32 value2);
static u64 GCD (s64 value1, s64 value2);

//============================================================================//
//      Least common multiple                                                 //
//============================================================================//

// Unsigned integer types
static u8 LCM (u8 value1, u8 value2);
static u16 LCM (u16 value1, u16 value2);
static u32 LCM (u32 value1, u32 value2);
static u64 LCM (u64 value1, u64 value2);

// Signed integer types
static u8 LCM (s8 value1, s8 value2);
static u16 LCM (s16 value1, s16 value2);
static u32 LCM (s32 value1, s32 value2);
static u64 LCM (s64 value1, s64 value2);

//============================================================================//
//      Cancellation                                                          //
//============================================================================//

// Unsigned integer types
static void Cancel (u8 *value1, u8 *value2);
static void Cancel (u16 *value1, u16 *value2);
static void Cancel (u32 *value1, u32 *value2);
static void Cancel (u64 *value1, u64 *value2);

// Signed integer types
static void Cancel (s8 *value1, s8 *value2);
static void Cancel (s16 *value1, s16 *value2);
static void Cancel (s32 *value1, s32 *value2);
static void Cancel (s64 *value1, s64 *value2);

//****************************************************************************//
//      Observational error                                                   //
//****************************************************************************//

//============================================================================//
//      Absolute error                                                        //
//============================================================================//
static f32 AbsError (f32 approximate, f32 accurate);
static f64 AbsError (f64 approximate, f64 accurate);
static f32v4 AbsError (f32v4 approximate, f32v4 accurate);
static f64v2 AbsError (f64v2 approximate, f64v2 accurate);

//============================================================================//
//      Relative error                                                        //
//============================================================================//
static f32 RelError (f32 approximate, f32 accurate);
static f64 RelError (f64 approximate, f64 accurate);
static f32v4 RelError (f32v4 approximate, f32v4 accurate);
static f64v2 RelError (f64v2 approximate, f64v2 accurate);

//****************************************************************************//
//      Scale functions                                                       //
//****************************************************************************//

// Scale by power of 2
static f32 Scale2 (f32 value, s16 exp);
static f64 Scale2 (f64 value, s16 exp);

// Scale by power of 10
static f32 Scale10 (f32 value, s16 exp);
static f64 Scale10 (f64 value, s16 exp);

//****************************************************************************//
//      Exponentiation functions                                              //
//****************************************************************************//

//============================================================================//
//      Exponentiation by base 2                                              //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer exponentiation by base 2                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u64 Exp2i (u8 exp);
static f32 Exp2i (s8 exp);
static f64 Exp2i (s16 exp);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real exponentiation by base 2                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static f32 Exp2 (f32 exp);
static f64 Exp2 (f64 exp);
static f32 Exp2m1 (f32 exp);
static f64 Exp2m1 (f64 exp);
static f32v4 Exp2 (f32v4 exp);
static f64v2 Exp2 (f64v2 exp);
static f32v4 Exp2m1 (f32v4 exp);
static f64v2 Exp2m1 (f64v2 exp);

//============================================================================//
//      Exponentiation by base 10                                             //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer exponentiation by base 10                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u64 Exp10i (u8 exp);
static f32 Exp10i (s8 exp);
static f64 Exp10i (s16 exp);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real exponentiation by base 10                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static f32 Exp10 (f32 exp);
static f64 Exp10 (f64 exp);
static f32 Exp10m1 (f32 exp);
static f64 Exp10m1 (f64 exp);
static f32v4 Exp10 (f32v4 exp);
static f64v2 Exp10 (f64v2 exp);
static f32v4 Exp10m1 (f32v4 exp);
static f64v2 Exp10m1 (f64v2 exp);

//============================================================================//
//      Exponentiation by base E (natural logarithm)                          //
//============================================================================//
static f32 Exp (f32 exp);
static f64 Exp (f64 exp);
static f32 Expm1 (f32 exp);
static f64 Expm1 (f64 exp);
static f32v4 Exp (f32v4 exp);
static f64v2 Exp (f64v2 exp);
static f32v4 Expm1 (f32v4 exp);
static f64v2 Expm1 (f64v2 exp);

//============================================================================//
//      Exponentiation by custom base                                         //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer exponentiation by custom base                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u8 ExpBi (u8 base, u8 exp);
static u16 ExpBi (u16 base, u8 exp);
static u32 ExpBi (u32 base, u8 exp);
static u64 ExpBi (u64 base, u8 exp);

// Signed integer types
static s8 ExpBi (s8 base, u8 exp);
static s16 ExpBi (s16 base, u8 exp);
static s32 ExpBi (s32 base, u8 exp);
static s64 ExpBi (s64 base, u8 exp);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real exponentiation by custom base                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static f32 ExpB (f32 base, f32 exp);
static f64 ExpB (f64 base, f64 exp);
static f32 ExpBm1 (f32 base, f32 exp);
static f64 ExpBm1 (f64 base, f64 exp);
static f32v4 ExpB (f32v4 base, f32v4 exp);
static f64v2 ExpB (f64v2 base, f64v2 exp);
static f32v4 ExpBm1 (f32v4 base, f32v4 exp);
static f64v2 ExpBm1 (f64v2 base, f64v2 exp);

//****************************************************************************//
//      Logarithmic functions                                                 //
//****************************************************************************//

//============================================================================//
//      Logarithm to base 2                                                   //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer logarithm to base 2                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u8 Log2i (u8 value);
static u8 Log2i (u16 value);
static u8 Log2i (u32 value);
static u8 Log2i (u64 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real logarithm to base 2                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static f32 Log2 (f32 value);
static f64 Log2 (f64 value);
static f32 Log2p1 (f32 value);
static f64 Log2p1 (f64 value);
static f32v4 Log2 (f32v4 value);
static f64v2 Log2 (f64v2 value);
static f32v4 Log2p1 (f32v4 value);
static f64v2 Log2p1 (f64v2 value);

//============================================================================//
//      Logarithm to base 10                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer logarithm to base 10                                          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u8 Log10i (u8 value);
static u8 Log10i (u16 value);
static u8 Log10i (u32 value);
static u8 Log10i (u64 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real logarithm to base 10                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static f32 Log10 (f32 value);
static f64 Log10 (f64 value);
static f32 Log10p1 (f32 value);
static f64 Log10p1 (f64 value);
static f32v4 Log10 (f32v4 value);
static f64v2 Log10 (f64v2 value);
static f32v4 Log10p1 (f32v4 value);
static f64v2 Log10p1 (f64v2 value);

//============================================================================//
//      Logarithm to base E (natural logarithm)                               //
//============================================================================//
static f32 Log (f32 value);
static f64 Log (f64 value);
static f32 Logp1 (f32 value);
static f64 Logp1 (f64 value);
static f32v4 Log (f32v4 value);
static f64v2 Log (f64v2 value);
static f32v4 Logp1 (f32v4 value);
static f64v2 Logp1 (f64v2 value);

//****************************************************************************//
//      Trigonometric functions                                               //
//****************************************************************************//

//============================================================================//
//      Hypotenuse                                                            //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      2 dimensional hypotenuse                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static f32 Hypot2D (f32 cath1, f32 cath2);
static f64 Hypot2D (f64 cath1, f64 cath2);
static f32v4 Hypot2D (f32v4 cath1, f32v4 cath2);
static f64v2 Hypot2D (f64v2 cath1, f64v2 cath2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      3 dimensional hypotenuse                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static f32 Hypot3D (f32 cath1, f32 cath2, f32 cath3);
static f64 Hypot3D (f64 cath1, f64 cath2, f64 cath3);
static f32v4 Hypot3D (f32v4 cath1, f32v4 cath2, f32v4 cath3);
static f64v2 Hypot3D (f64v2 cath1, f64v2 cath2, f64v2 cath3);

//============================================================================//
//      Cathetus                                                              //
//============================================================================//
static f32 Cath (f32 hypot, f32 cath);
static f64 Cath (f64 hypot, f64 cath);
static f32v4 Cath (f32v4 hypot, f32v4 cath);
static f64v2 Cath (f64v2 hypot, f64v2 cath);

//============================================================================//
//      Trigonometric sine                                                    //
//============================================================================//
static f32 Sin (f32 value);
static f64 Sin (f64 value);
static f32v4 Sin (f32v4 value);
static f64v2 Sin (f64v2 value);

//============================================================================//
//      Trigonometric cosine                                                  //
//============================================================================//
static f32 Cos (f32 value);
static f64 Cos (f64 value);
static f32v4 Cos (f32v4 value);
static f64v2 Cos (f64v2 value);

//============================================================================//
//      Trigonometric sine and cosine                                         //
//============================================================================//
static void SinCos (f32 *sin, f32 *cos, f32 value);
static void SinCos (f64 *sin, f64 *cos, f64 value);
static void SinCos (f32v4 *sin, f32v4 *cos, f32v4 value);
static void SinCos (f64v2 *sin, f64v2 *cos, f64v2 value);

//============================================================================//
//      Trigonometric tangent                                                 //
//============================================================================//
static f32 Tan (f32 value);
static f64 Tan (f64 value);
static f32v4 Tan (f32v4 value);
static f64v2 Tan (f64v2 value);

//****************************************************************************//
//      Inverse trigonometric functions                                       //
//****************************************************************************//

//============================================================================//
//      Inverse trigonometric sine                                            //
//============================================================================//
static f32 ArcSin (f32 value);
static f64 ArcSin (f64 value);
static f32v4 ArcSin (f32v4 value);
static f64v2 ArcSin (f64v2 value);

//============================================================================//
//      Inverse trigonometric cosine                                          //
//============================================================================//
static f32 ArcCos (f32 value);
static f64 ArcCos (f64 value);
static f32v4 ArcCos (f32v4 value);
static f64v2 ArcCos (f64v2 value);

//============================================================================//
//      Inverse trigonometric tangent                                         //
//============================================================================//
static f32 ArcTan (f32 value);
static f64 ArcTan (f64 value);
static f32 ArcTan2 (f32 sin, f32 cos);
static f64 ArcTan2 (f64 sin, f64 cos);
static f32v4 ArcTan (f32v4 value);
static f64v2 ArcTan (f64v2 value);
static f32v4 ArcTan2 (f32v4 sin, f32v4 cos);
static f64v2 ArcTan2 (f64v2 sin, f64v2 cos);

//****************************************************************************//
//      Hyperbolic functions                                                  //
//****************************************************************************//

//============================================================================//
//      Hyperbolic sine                                                       //
//============================================================================//
static f32 SinH (f32 value);
static f64 SinH (f64 value);
static f32v4 SinH (f32v4 value);
static f64v2 SinH (f64v2 value);

//============================================================================//
//      Hyperbolic cosine                                                     //
//============================================================================//
static f32 CosH (f32 value);
static f64 CosH (f64 value);
static f32v4 CosH (f32v4 value);
static f64v2 CosH (f64v2 value);

//============================================================================//
//      Hyperbolic tangent                                                    //
//============================================================================//
static f32 TanH (f32 value);
static f64 TanH (f64 value);
static f32v4 TanH (f32v4 value);
static f64v2 TanH (f64v2 value);

//****************************************************************************//
//      Inverse hyperbolic functions                                          //
//****************************************************************************//

//============================================================================//
//      Inverse hyperbolic sine                                               //
//============================================================================//
static f32 ArcSinH (f32 value);
static f64 ArcSinH (f64 value);
static f32v4 ArcSinH (f32v4 value);
static f64v2 ArcSinH (f64v2 value);

//============================================================================//
//      Inverse hyperbolic cosine                                             //
//============================================================================//
static f32 ArcCosH (f32 value);
static f64 ArcCosH (f64 value);
static f32v4 ArcCosH (f32v4 value);
static f64v2 ArcCosH (f64v2 value);

//============================================================================//
//      Inverse hyperbolic tangent                                            //
//============================================================================//
static f32 ArcTanH (f32 value);
static f64 ArcTanH (f64 value);
static f32v4 ArcTanH (f32v4 value);
static f64v2 ArcTanH (f64v2 value);

//****************************************************************************//
//      Rounding                                                              //
//****************************************************************************//

// Round down (floor)
static f32 RoundDown (f32 value);
static f64 RoundDown (f64 value);
static f32v4 RoundDown (f32v4 value);
static f64v2 RoundDown (f64v2 value);

// Round up (ceil)
static f32 RoundUp (f32 value);
static f64 RoundUp (f64 value);
static f32v4 RoundUp (f32v4 value);
static f64v2 RoundUp (f64v2 value);

// Round to nearest even integer
static f32 RoundInt (f32 value);
static f64 RoundInt (f64 value);
static f32v4 RoundInt (f32v4 value);
static f64v2 RoundInt (f64v2 value);

// Round to nearest integer, away from zero
static f32 Round (f32 value);
static f64 Round (f64 value);
static f32v4 Round (f32v4 value);
static f64v2 Round (f64v2 value);

// Round to nearest integer, toward zero (truncation)
static f32 Truncate (f32 value);
static f64 Truncate (f64 value);
static f32v4 Truncate (f32v4 value);
static f64v2 Truncate (f64v2 value);

// Fractional part
static f32 Frac (f32 value);
static f64 Frac (f64 value);
static f32v4 Frac (f32v4 value);
static f64v2 Frac (f64v2 value);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

// Check for normal value
static bool IsNormal (f32 value);
static bool IsNormal (f64 value);

// Check for subnormal value
static bool IsSubnormal (f32 value);
static bool IsSubnormal (f64 value);

// Check for finite value
static bool IsFinite (f32 value);
static bool IsFinite (f64 value);

// Check for infinite value
static bool IsInfinite (f32 value);
static bool IsInfinite (f64 value);

// Check for NaN value
static bool IsNaN (f32 value);
static bool IsNaN (f64 value);
};
# else
extern "C" {
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Bitwise operations                                                    //
//****************************************************************************//

//============================================================================//
//      Byte swap                                                             //
//============================================================================//

// Unsigned integer types
u8 Math_ByteSwap_uint8(u8 value);
u16 Math_ByteSwap_uint16(u16 value);
u32 Math_ByteSwap_uint32(u32 value);
u64 Math_ByteSwap_uint64(u64 value);
u8v16 Math_ByteSwap_uint8v16(u8v16 value);
u16v8 Math_ByteSwap_uint16v8(u16v8 value);
u32v4 Math_ByteSwap_uint32v4(u32v4 value);
u64v2 Math_ByteSwap_uint64v2(u64v2 value);

// Signed integer types
s8 Math_ByteSwap_sint8(s8 value);
s16 Math_ByteSwap_sint16(s16 value);
s32 Math_ByteSwap_sint32(s32 value);
s64 Math_ByteSwap_sint64(s64 value);
s8v16 Math_ByteSwap_sint8v16(s8v16 value);
s16v8 Math_ByteSwap_sint16v8(s16v8 value);
s32v4 Math_ByteSwap_sint32v4(s32v4 value);
s64v2 Math_ByteSwap_sint64v2(s64v2 value);

//============================================================================//
//      Bit reversal permutation                                              //
//============================================================================//

// Unsigned integer types
u8 Math_BitReverse_uint8(u8 value);
u16 Math_BitReverse_uint16(u16 value);
u32 Math_BitReverse_uint32(u32 value);
u64 Math_BitReverse_uint64(u64 value);
u8v16 Math_BitReverse_uint8v16(u8v16 value);
u16v8 Math_BitReverse_uint16v8(u16v8 value);
u32v4 Math_BitReverse_uint32v4(u32v4 value);
u64v2 Math_BitReverse_uint64v2(u64v2 value);

// Signed integer types
s8 Math_BitReverse_sint8(s8 value);
s16 Math_BitReverse_sint16(s16 value);
s32 Math_BitReverse_sint32(s32 value);
s64 Math_BitReverse_sint64(s64 value);
s8v16 Math_BitReverse_sint8v16(s8v16 value);
s16v8 Math_BitReverse_sint16v8(s16v8 value);
s32v4 Math_BitReverse_sint32v4(s32v4 value);
s64v2 Math_BitReverse_sint64v2(s64v2 value);

//============================================================================//
//      Bit scan                                                              //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Bit scan forward                                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u8 Math_ScanForward_uint8(u8 value);
u16 Math_ScanForward_uint16(u16 value);
u32 Math_ScanForward_uint32(u32 value);
u64 Math_ScanForward_uint64(u64 value);

// Signed integer types
s8 Math_ScanForward_sint8(s8 value);
s16 Math_ScanForward_sint16(s16 value);
s32 Math_ScanForward_sint32(s32 value);
s64 Math_ScanForward_sint64(s64 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Bit scan backward                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u8 Math_ScanBackward_uint8(u8 value);
u16 Math_ScanBackward_uint16(u16 value);
u32 Math_ScanBackward_uint32(u32 value);
u64 Math_ScanBackward_uint64(u64 value);

// Signed integer types
s8 Math_ScanBackward_sint8(s8 value);
s16 Math_ScanBackward_sint16(s16 value);
s32 Math_ScanBackward_sint32(s32 value);
s64 Math_ScanBackward_sint64(s64 value);

//============================================================================//
//      Circular rotation                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Circular rotation to the left                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u8 Math_RotateLeft_uint8(u8 value, u8 shift);
u16 Math_RotateLeft_uint16(u16 value, u8 shift);
u32 Math_RotateLeft_uint32(u32 value, u8 shift);
u64 Math_RotateLeft_uint64(u64 value, u8 shift);

// Signed integer types
s8 Math_RotateLeft_sint8(s8 value, u8 shift);
s16 Math_RotateLeft_sint16(s16 value, u8 shift);
s32 Math_RotateLeft_sint32(s32 value, u8 shift);
s64 Math_RotateLeft_sint64(s64 value, u8 shift);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Circular rotation to the right                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u8 Math_RotateRight_uint8(u8 value, u8 shift);
u16 Math_RotateRight_uint16(u16 value, u8 shift);
u32 Math_RotateRight_uint32(u32 value, u8 shift);
u64 Math_RotateRight_uint64(u64 value, u8 shift);

// Signed integer types
s8 Math_RotateRight_sint8(s8 value, u8 shift);
s16 Math_RotateRight_sint16(s16 value, u8 shift);
s32 Math_RotateRight_sint32(s32 value, u8 shift);
s64 Math_RotateRight_sint64(s64 value, u8 shift);

//============================================================================//
//      Population count                                                      //
//============================================================================//

// Unsigned integer types
u8 Math_PopCount_uint8(u8 value);
u16 Math_PopCount_uint16(u16 value);
u32 Math_PopCount_uint32(u32 value);
u64 Math_PopCount_uint64(u64 value);
u8v16 Math_PopCount_uint8v16(u8v16 value);
u16v8 Math_PopCount_uint16v8(u16v8 value);
u32v4 Math_PopCount_uint32v4(u32v4 value);
u64v2 Math_PopCount_uint64v2(u64v2 value);

// Signed integer types
s8 Math_PopCount_sint8(s8 value);
s16 Math_PopCount_sint16(s16 value);
s32 Math_PopCount_sint32(s32 value);
s64 Math_PopCount_sint64(s64 value);
s8v16 Math_PopCount_sint8v16(s8v16 value);
s16v8 Math_PopCount_sint16v8(s16v8 value);
s32v4 Math_PopCount_sint32v4(s32v4 value);
s64v2 Math_PopCount_sint64v2(s64v2 value);

//****************************************************************************//
//      Arithmetic operations                                                 //
//****************************************************************************//

//============================================================================//
//      Absolute value                                                        //
//============================================================================//

// Signed integer types
u8 Math_Abs_sint8(s8 value);
u16 Math_Abs_sint16(s16 value);
u32 Math_Abs_sint32(s32 value);
u64 Math_Abs_sint64(s64 value);
u8v16 Math_Abs_sint8v16(s8v16 value);
u16v8 Math_Abs_sint16v8(s16v8 value);
u32v4 Math_Abs_sint32v4(s32v4 value);
u64v2 Math_Abs_sint64v2(s64v2 value);

// Floating-point types
f32 Math_Abs_flt32(f32 value);
f64 Math_Abs_flt64(f64 value);
f32v4 Math_Abs_flt32v4(f32v4 value);
f64v2 Math_Abs_flt64v2(f64v2 value);

//============================================================================//
//      Negative absolute value                                               //
//============================================================================//

// Signed integer types
s8 Math_NegAbs_sint8(s8 value);
s16 Math_NegAbs_sint16(s16 value);
s32 Math_NegAbs_sint32(s32 value);
s64 Math_NegAbs_sint64(s64 value);
s8v16 Math_NegAbs_sint8v16(s8v16 value);
s16v8 Math_NegAbs_sint16v8(s16v8 value);
s32v4 Math_NegAbs_sint32v4(s32v4 value);
s64v2 Math_NegAbs_sint64v2(s64v2 value);

// Floating-point types
f32 Math_NegAbs_flt32(f32 value);
f64 Math_NegAbs_flt64(f64 value);
f32v4 Math_NegAbs_flt32v4(f32v4 value);
f64v2 Math_NegAbs_flt64v2(f64v2 value);

//============================================================================//
//      Number sign                                                           //
//============================================================================//

// Signed integer types
s8 Math_Sign_sint8(s8 value);
s16 Math_Sign_sint16(s16 value);
s32 Math_Sign_sint32(s32 value);
s64 Math_Sign_sint64(s64 value);
s8v16 Math_Sign_sint8v16(s8v16 value);
s16v8 Math_Sign_sint16v8(s16v8 value);
s32v4 Math_Sign_sint32v4(s32v4 value);
s64v2 Math_Sign_sint64v2(s64v2 value);

// Floating-point types
f32 Math_Sign_flt32(f32 value);
f64 Math_Sign_flt64(f64 value);
f32v4 Math_Sign_flt32v4(f32v4 value);
f64v2 Math_Sign_flt64v2(f64v2 value);

//============================================================================//
//      Square root                                                           //
//============================================================================//

// Unsigned integer types
u8 Math_Sqrt_uint8(u8 value);
u16 Math_Sqrt_uint16(u16 value);
u32 Math_Sqrt_uint32(u32 value);
u64 Math_Sqrt_uint64(u64 value);

// Floating-point types
f32 Math_Sqrt_flt32(f32 value);
f64 Math_Sqrt_flt64(f64 value);
f32v4 Math_Sqrt_flt32v4(f32v4 value);
f64v2 Math_Sqrt_flt64v2(f64v2 value);

//============================================================================//
//      Square value                                                          //
//============================================================================//

// Unsigned integer types
u8 Math_Sqr_uint8(u8 value);
u16 Math_Sqr_uint16(u16 value);
u32 Math_Sqr_uint32(u32 value);
u64 Math_Sqr_uint64(u64 value);

// Signed integer types
s8 Math_Sqr_sint8(s8 value);
s16 Math_Sqr_sint16(s16 value);
s32 Math_Sqr_sint32(s32 value);
s64 Math_Sqr_sint64(s64 value);

// Floating-point types
f32 Math_Sqr_flt32(f32 value);
f64 Math_Sqr_flt64(f64 value);
f32v4 Math_Sqr_flt32v4(f32v4 value);
f64v2 Math_Sqr_flt64v2(f64v2 value);

//============================================================================//
//      Cube value                                                            //
//============================================================================//

// Unsigned integer types
u8 Math_Cube_uint8(u8 value);
u16 Math_Cube_uint16(u16 value);
u32 Math_Cube_uint32(u32 value);
u64 Math_Cube_uint64(u64 value);

// Signed integer types
s8 Math_Cube_sint8(s8 value);
s16 Math_Cube_sint16(s16 value);
s32 Math_Cube_sint32(s32 value);
s64 Math_Cube_sint64(s64 value);

// Floating-point types
f32 Math_Cube_flt32(f32 value);
f64 Math_Cube_flt64(f64 value);
f32v4 Math_Cube_flt32v4(f32v4 value);
f64v2 Math_Cube_flt64v4(f64v2 value);

//============================================================================//
//      Inverse value                                                         //
//============================================================================//
f32 Math_InverseValue_flt32(f32 value);
f64 Math_InverseValue_flt64(f64 value);
f32v4 Math_InverseValue_flt32v4(f32v4 value);
f64v2 Math_InverseValue_flt64v2(f64v2 value);

//============================================================================//
//      Inverse square value                                                  //
//============================================================================//
f32 Math_InverseSquare_flt32(f32 value);
f64 Math_InverseSquare_flt64(f64 value);
f32v4 Math_InverseSquare_flt32v4(f32v4 value);
f64v2 Math_InverseSquare_flt64v2(f64v2 value);

//============================================================================//
//      Inverse cube value                                                    //
//============================================================================//
f32 Math_InverseCube_flt32(f32 value);
f64 Math_InverseCube_flt64(f64 value);
f32v4 Math_InverseCube_flt32v4(f32v4 value);
f64v2 Math_InverseCube_flt64v2(f64v2 value);

//============================================================================//
//      Three-state comparison                                                //
//============================================================================//

// Unsigned integer types
s8 Math_Compare_uint8(u8 value1, u8 value2);
s16 Math_Compare_uint16(u16 value1, u16 value2);
s32 Math_Compare_uint32(u32 value1, u32 value2);
s64 Math_Compare_uint64(u64 value1, u64 value2);
s8v16 Math_Compare_uint8v16(u8v16 value1, u8v16 value2);
s16v8 Math_Compare_uint16v8(u16v8 value1, u16v8 value2);
s32v4 Math_Compare_uint32v4(u32v4 value1, u32v4 value2);
s64v2 Math_Compare_uint64v2(u64v2 value1, u64v2 value2);

// Signed integer types
s8 Math_Compare_sint8(s8 value1, s8 value2);
s16 Math_Compare_sint16(s16 value1, s16 value2);
s32 Math_Compare_sint32(s32 value1, s32 value2);
s64 Math_Compare_sint64(s64 value1, s64 value2);
s8v16 Math_Compare_sint8v16(s8v16 value1, s8v16 value2);
s16v8 Math_Compare_sint16v8(s16v8 value1, s16v8 value2);
s32v4 Math_Compare_sint32v4(s32v4 value1, s32v4 value2);
s64v2 Math_Compare_sint64v2(s64v2 value1, s64v2 value2);

// Floating-point types
f32 Math_Compare_flt32(f32 value1, f32 value2);
f64 Math_Compare_flt64(f64 value1, f64 value2);
f32v4 Math_Compare_flt32v4(f32v4 value1, f32v4 value2);
f64v2 Math_Compare_flt64v2(f64v2 value1, f64v2 value2);

//============================================================================//
//      Minimum and maximum absolute value                                    //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Minimum absolute value                                                //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
u8 Math_MinAbs_sint8(s8 value1, s8 value2);
u16 Math_MinAbs_sint16(s16 value1, s16 value2);
u32 Math_MinAbs_sint32(s32 value1, s32 value2);
u64 Math_MinAbs_sint64(s64 value1, s64 value2);
u8v16 Math_MinAbs_sint8v16(s8v16 value1, s8v16 value2);
u16v8 Math_MinAbs_sint16v8(s16v8 value1, s16v8 value2);
u32v4 Math_MinAbs_sint32v4(s32v4 value1, s32v4 value2);

// Floating-point types
f32 Math_MinAbs_flt32(f32 value1, f32 value2);
f64 Math_MinAbs_flt64(f64 value1, f64 value2);
f32v4 Math_MinAbs_flt32v4(f32v4 value1, f32v4 value2);
f64v2 Math_MinAbs_flt64v2(f64v2 value1, f64v2 value2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Maximum absolute value                                                //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
u8 Math_MaxAbs_sint8(s8 value1, s8 value2);
u16 Math_MaxAbs_sint16(s16 value1, s16 value2);
u32 Math_MaxAbs_sint32(s32 value1, s32 value2);
u64 Math_MaxAbs_sint64(s64 value1, s64 value2);
u8v16 Math_MaxAbs_sint8v16(s8v16 value1, s8v16 value2);
u16v8 Math_MaxAbs_sint16v8(s16v8 value1, s16v8 value2);
u32v4 Math_MaxAbs_sint32v4(s32v4 value1, s32v4 value2);

// Floating-point types
f32 Math_MaxAbs_flt32(f32 value1, f32 value2);
f64 Math_MaxAbs_flt64(f64 value1, f64 value2);
f32v4 Math_MaxAbs_flt32v4(f32v4 value1, f32v4 value2);
f64v2 Math_MaxAbs_flt64v2(f64v2 value1, f64v2 value2);

//============================================================================//
//      Minimum and maximum value                                             //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Minimum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u8 Math_Min_uint8(u8 value1, u8 value2);
u16 Math_Min_uint16(u16 value1, u16 value2);
u32 Math_Min_uint32(u32 value1, u32 value2);
u64 Math_Min_uint64(u64 value1, u64 value2);
u8v16 Math_Min_uint8v16(u8v16 value1, u8v16 value2);
u16v8 Math_Min_uint16v8(u16v8 value1, u16v8 value2);
u32v4 Math_Min_uint32v4(u32v4 value1, u32v4 value2);

// Signed integer types
s8 Math_Min_sint8(s8 value1, s8 value2);
s16 Math_Min_sint16(s16 value1, s16 value2);
s32 Math_Min_sint32(s32 value1, s32 value2);
s64 Math_Min_sint64(s64 value1, s64 value2);
s8v16 Math_Min_sint8v16(s8v16 value1, s8v16 value2);
s16v8 Math_Min_sint16v8(s16v8 value1, s16v8 value2);
s32v4 Math_Min_sint32v4(s32v4 value1, s32v4 value2);

// Floating-point types
f32 Math_Min_flt32(f32 value1, f32 value2);
f64 Math_Min_flt64(f64 value1, f64 value2);
f32v4 Math_Min_flt32v4(f32v4 value1, f32v4 value2);
f64v2 Math_Min_flt64v2(f64v2 value1, f64v2 value2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Maximum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u8 Math_Max_uint8(u8 value1, u8 value2);
u16 Math_Max_uint16(u16 value1, u16 value2);
u32 Math_Max_uint32(u32 value1, u32 value2);
u64 Math_Max_uint64(u64 value1, u64 value2);
u8v16 Math_Max_uint8v16(u8v16 value1, u8v16 value2);
u16v8 Math_Max_uint16v8(u16v8 value1, u16v8 value2);
u32v4 Math_Max_uint32v4(u32v4 value1, u32v4 value2);

// Signed integer types
s8 Math_Max_sint8(s8 value1, s8 value2);
s16 Math_Max_sint16(s16 value1, s16 value2);
s32 Math_Max_sint32(s32 value1, s32 value2);
s64 Math_Max_sint64(s64 value1, s64 value2);
s8v16 Math_Max_sint8v16(s8v16 value1, s8v16 value2);
s16v8 Math_Max_sint16v8(s16v8 value1, s16v8 value2);
s32v4 Math_Max_sint32v4(s32v4 value1, s32v4 value2);

// Floating-point types
f32 Math_Max_flt32(f32 value1, f32 value2);
f64 Math_Max_flt64(f64 value1, f64 value2);
f32v4 Math_Max_flt32v4(f32v4 value1, f32v4 value2);
f64v2 Math_Max_flt64v2(f64v2 value1, f64v2 value2);

//============================================================================//
//      Greatest common divisor                                               //
//============================================================================//

// Unsigned integer types
u8 Math_GCD_uint8(u8 value1, u8 value2);
u16 Math_GCD_uint16(u16 value1, u16 value2);
u32 Math_GCD_uint32(u32 value1, u32 value2);
u64 Math_GCD_uint64(u64 value1, u64 value2);

// Signed integer types
u8 Math_GCD_sint8(s8 value1, s8 value2);
u16 Math_GCD_sint16(s16 value1, s16 value2);
u32 Math_GCD_sint32(s32 value1, s32 value2);
u64 Math_GCD_sint64(s64 value1, s64 value2);

//============================================================================//
//      Least common multiple                                                 //
//============================================================================//

// Unsigned integer types
u8 Math_LCM_uint8(u8 value1, u8 value2);
u16 Math_LCM_uint16(u16 value1, u16 value2);
u32 Math_LCM_uint32(u32 value1, u32 value2);
u64 Math_LCM_uint64(u64 value1, u64 value2);

// Signed integer types
u8 Math_LCM_sint8(s8 value1, s8 value2);
u16 Math_LCM_sint16(s16 value1, s16 value2);
u32 Math_LCM_sint32(s32 value1, s32 value2);
u64 Math_LCM_sint64(s64 value1, s64 value2);

//============================================================================//
//      Cancellation                                                          //
//============================================================================//

// Unsigned integer types
void Math_Cancel_uint8(u8 *value1, u8 *value2);
void Math_Cancel_uint16(u16 *value1, u16 *value2);
void Math_Cancel_uint32(u32 *value1, u32 *value2);
void Math_Cancel_uint64(u64 *value1, u64 *value2);

// Signed integer types
void Math_Cancel_sint8(s8 *value1, s8 *value2);
void Math_Cancel_sint16(s16 *value1, s16 *value2);
void Math_Cancel_sint32(s32 *value1, s32 *value2);
void Math_Cancel_sint64(s64 *value1, s64 *value2);

//****************************************************************************//
//      Observational error                                                   //
//****************************************************************************//

//============================================================================//
//      Absolute error                                                        //
//============================================================================//
f32 Math_AbsError_flt32(f32 approximate, f32 accurate);
f64 Math_AbsError_flt64(f64 approximate, f64 accurate);
f32v4 Math_AbsError_flt32v4(f32v4 approximate, f32v4 accurate);
f64v2 Math_AbsError_flt64v2(f64v2 approximate, f64v2 accurate);

//============================================================================//
//      Relative error                                                        //
//============================================================================//
f32 Math_RelError_flt32(f32 approximate, f32 accurate);
f64 Math_RelError_flt64(f64 approximate, f64 accurate);
f32v4 Math_RelError_flt32v4(f32v4 approximate, f32v4 accurate);
f64v2 Math_RelError_flt64v2(f64v2 approximate, f64v2 accurate);

//****************************************************************************//
//      Scale functions                                                       //
//****************************************************************************//

// Scale by power of 2
f32 Math_Scale2_flt32(f32 value, s16 exp);
f64 Math_Scale2_flt64(f64 value, s16 exp);

// Scale by power of 10
f32 Math_Scale10_flt32(f32 value, s16 exp);
f64 Math_Scale10_flt64(f64 value, s16 exp);

//****************************************************************************//
//      Exponentiation functions                                              //
//****************************************************************************//

//============================================================================//
//      Exponentiation by base 2                                              //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer exponentiation by base 2                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u64 Math_Exp2i_uint64(u8 exp);
f32 Math_Exp2i_flt32(s8 exp);
f64 Math_Exp2i_flt64(s16 exp);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real exponentiation by base 2                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
f32 Math_Exp2_flt32(f32 exp);
f64 Math_Exp2_flt64(f64 exp);
f32 Math_Exp2m1_flt32(f32 exp);
f64 Math_Exp2m1_flt64(f64 exp);
f32v4 Math_Exp2_flt32v4(f32v4 exp);
f64v2 Math_Exp2_flt64v2(f64v2 exp);
f32v4 Math_Exp2m1_flt32v4(f32v4 exp);
f64v2 Math_Exp2m1_flt64v2(f64v2 exp);

//============================================================================//
//      Exponentiation by base 10                                             //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer exponentiation by base 10                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u64 Math_Exp10i_uint64(u8 exp);
f32 Math_Exp10i_flt32(s8 exp);
f64 Math_Exp10i_flt64(s16 exp);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real exponentiation by base 10                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
f32 Math_Exp10_flt32(f32 exp);
f64 Math_Exp10_flt64(f64 exp);
f32 Math_Exp10m1_flt32(f32 exp);
f64 Math_Exp10m1_flt64(f64 exp);
f32v4 Math_Exp10_flt32v4(f32v4 exp);
f64v2 Math_Exp10_flt64v2(f64v2 exp);
f32v4 Math_Exp10m1_flt32v4(f32v4 exp);
f64v2 Math_Exp10m1_flt64v2(f64v2 exp);

//============================================================================//
//      Exponentiation by base E (natural logarithm)                          //
//============================================================================//
f32 Math_Exp_flt32(f32 exp);
f64 Math_Exp_flt64(f64 exp);
f32 Math_Expm1_flt32(f32 exp);
f64 Math_Expm1_flt64(f64 exp);
f32v4 Math_Exp_flt32v4(f32v4 exp);
f64v2 Math_Exp_flt64v2(f64v2 exp);
f32v4 Math_Expm1_flt32v4(f32v4 exp);
f64v2 Math_Expm1_flt64v2(f64v2 exp);

//============================================================================//
//      Exponentiation by custom base                                         //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer exponentiation by custom base                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u8 Math_ExpBi_uint8(u8 base, u8 exp);
u16 Math_ExpBi_uint16(u16 base, u8 exp);
u32 Math_ExpBi_uint32(u32 base, u8 exp);
u64 Math_ExpBi_uint64(u64 base, u8 exp);

// Signed integer types
s8 Math_ExpBi_sint8(s8 base, u8 exp);
s16 Math_ExpBi_sint16(s16 base, u8 exp);
s32 Math_ExpBi_sint32(s32 base, u8 exp);
s64 Math_ExpBi_sint64(s64 base, u8 exp);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real exponentiation by custom base                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
f32 Math_ExpB_flt32(f32 base, f32 exp);
f64 Math_ExpB_flt64(f64 base, f64 exp);
f32 Math_ExpBm1_flt32(f32 base, f32 exp);
f64 Math_ExpBm1_flt64(f64 base, f64 exp);
f32v4 Math_ExpB_flt32v4(f32v4 base, f32v4 exp);
f64v2 Math_ExpB_flt64v2(f64v2 base, f64v2 exp);
f32v4 Math_ExpBm1_flt32v4(f32v4 base, f32v4 exp);
f64v2 Math_ExpBm1_flt64v2(f64v2 base, f64v2 exp);

//****************************************************************************//
//      Logarithmic functions                                                 //
//****************************************************************************//

//============================================================================//
//      Logarithm to base 2                                                   //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer logarithm to base 2                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u8 Math_Log2i_uint8(u8 value);
u8 Math_Log2i_uint16(u16 value);
u8 Math_Log2i_uint32(u32 value);
u8 Math_Log2i_uint64(u64 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real logarithm to base 2                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
f32 Math_Log2_flt32(f32 value);
f64 Math_Log2_flt64(f64 value);
f32 Math_Log2p1_flt32(f32 value);
f64 Math_Log2p1_flt64(f64 value);
f32v4 Math_Log2_flt32v4(f32v4 value);
f64v2 Math_Log2_flt64v2(f64v2 value);
f32v4 Math_Log2p1_flt32v4(f32v4 value);
f64v2 Math_Log2p1_flt64v2(f64v2 value);

//============================================================================//
//      Logarithm to base 10                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Integer logarithm to base 10                                          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u8 Math_Log10i_uint8(u8 value);
u8 Math_Log10i_uint16(u16 value);
u8 Math_Log10i_uint32(u32 value);
u8 Math_Log10i_uint64(u64 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Real logarithm to base 10                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
f32 Math_Log10_flt32(f32 value);
f64 Math_Log10_flt64(f64 value);
f32 Math_Log10p1_flt32(f32 value);
f64 Math_Log10p1_flt64(f64 value);
f32v4 Math_Log10_flt32v4(f32v4 value);
f64v2 Math_Log10_flt64v2(f64v2 value);
f32v4 Math_Log10p1_flt32v4(f32v4 value);
f64v2 Math_Log10p1_flt64v2(f64v2 value);

//============================================================================//
//      Logarithm to base E (natural logarithm)                               //
//============================================================================//
f32 Math_Log_flt32(f32 value);
f64 Math_Log_flt64(f64 value);
f32 Math_Logp1_flt32(f32 value);
f64 Math_Logp1_flt64(f64 value);
f32v4 Math_Log_flt32v4(f32v4 value);
f64v2 Math_Log_flt64v2(f64v2 value);
f32v4 Math_Logp1_flt32v4(f32v4 value);
f64v2 Math_Logp1_flt64v2(f64v2 value);

//****************************************************************************//
//      Trigonometric functions                                               //
//****************************************************************************//

//============================================================================//
//      Hypotenuse                                                            //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      2 dimensional hypotenuse                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
f32 Math_Hypot2D_flt32(f32 cath1, f32 cath2);
f64 Math_Hypot2D_flt64(f64 cath1, f64 cath2);
f32v4 Math_Hypot2D_flt32v4(f32v4 cath1, f32v4 cath2);
f64v2 Math_Hypot2D_flt64v2(f64v2 cath1, f64v2 cath2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      3 dimensional hypotenuse                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
f32 Math_Hypot3D_flt32(f32 cath1, f32 cath2, f32 cath3);
f64 Math_Hypot3D_flt64(f64 cath1, f64 cath2, f64 cath3);
f32v4 Math_Hypot3D_flt32v4(f32v4 cath1, f32v4 cath2, f32v4 cath3);
f64v2 Math_Hypot3D_flt64v2(f64v2 cath1, f64v2 cath2, f64v2 cath3);

//============================================================================//
//      Cathetus                                                              //
//============================================================================//
f32 Math_Cath_flt32(f32 hypot, f32 cath);
f64 Math_Cath_flt64(f64 hypot, f64 cath);
f32v4 Math_Cath_flt32v4(f32v4 hypot, f32v4 cath);
f64v2 Math_Cath_flt64v2(f64v2 hypot, f64v2 cath);

//============================================================================//
//      Trigonometric sine                                                    //
//============================================================================//
f32 Math_Sin_flt32(f32 value);
f64 Math_Sin_flt64(f64 value);
f32v4 Math_Sin_flt32v4(f32v4 value);
f64v2 Math_Sin_flt64v2(f64v2 value);

//============================================================================//
//      Trigonometric cosine                                                  //
//============================================================================//
f32 Math_Cos_flt32(f32 value);
f64 Math_Cos_flt64(f64 value);
f32v4 Math_Cos_flt32v4(f32v4 value);
f64v2 Math_Cos_flt64v2(f64v2 value);

//============================================================================//
//      Trigonometric sine and cosine                                         //
//============================================================================//
void Math_SinCos_flt32(f32 *sin, f32 *cos, f32 value);
void Math_SinCos_flt64(f64 *sin, f64 *cos, f64 value);
void Math_SinCos_flt32v4(f32v4 *sin, f32v4 *cos, f32v4 value);
void Math_SinCos_flt64v2(f64v2 *sin, f64v2 *cos, f64v2 value);

//============================================================================//
//      Trigonometric tangent                                                 //
//============================================================================//
f32 Math_Tan_flt32(f32 value);
f64 Math_Tan_flt64(f64 value);
f32v4 Math_Tan_flt32v4(f32v4 value);
f64v2 Math_Tan_flt64v2(f64v2 value);

//****************************************************************************//
//      Inverse trigonometric functions                                       //
//****************************************************************************//

//============================================================================//
//      Inverse trigonometric sine                                            //
//============================================================================//
f32 Math_ArcSin_flt32(f32 value);
f64 Math_ArcSin_flt64(f64 value);
f32v4 Math_ArcSin_flt32v4(f32v4 value);
f64v2 Math_ArcSin_flt64v2(f64v2 value);

//============================================================================//
//      Inverse trigonometric cosine                                          //
//============================================================================//
f32 Math_ArcCos_flt32(f32 value);
f64 Math_ArcCos_flt64(f64 value);
f32v4 Math_ArcCos_flt32v4(f32v4 value);
f64v2 Math_ArcCos_flt64v2(f64v2 value);

//============================================================================//
//      Inverse trigonometric tangent                                         //
//============================================================================//
f32 Math_ArcTan_flt32(f32 value);
f64 Math_ArcTan_flt64(f64 value);
f32 Math_ArcTan2_flt32(f32 sin, f32 cos);
f64 Math_ArcTan2_flt64(f64 sin, f64 cos);
f32v4 Math_ArcTan_flt32v4(f32v4 value);
f64v2 Math_ArcTan_flt64v2(f64v2 value);
f32v4 Math_ArcTan2_flt32v4(f32v4 sin, f32v4 cos);
f64v2 Math_ArcTan2_flt64v2(f64v2 sin, f64v2 cos);

//****************************************************************************//
//      Hyperbolic functions                                                  //
//****************************************************************************//

//============================================================================//
//      Hyperbolic sine                                                       //
//============================================================================//
f32 Math_SinH_flt32(f32 value);
f64 Math_SinH_flt64(f64 value);
f32v4 Math_SinH_flt32v4(f32v4 value);
f64v2 Math_SinH_flt64v2(f64v2 value);

//============================================================================//
//      Hyperbolic cosine                                                     //
//============================================================================//
f32 Math_CosH_flt32(f32 value);
f64 Math_CosH_flt64(f64 value);
f32v4 Math_CosH_flt32v4(f32v4 value);
f64v2 Math_CosH_flt64v2(f64v2 value);

//============================================================================//
//      Hyperbolic tangent                                                    //
//============================================================================//
f32 Math_TanH_flt32(f32 value);
f64 Math_TanH_flt64(f64 value);
f32v4 Math_TanH_flt32v4(f32v4 value);
f64v2 Math_TanH_flt64v2(f64v2 value);

//****************************************************************************//
//      Inverse hyperbolic functions                                          //
//****************************************************************************//

//============================================================================//
//      Inverse hyperbolic sine                                               //
//============================================================================//
f32 Math_ArcSinH_flt32(f32 value);
f64 Math_ArcSinH_flt64(f64 value);
f32v4 Math_ArcSinH_flt32v4(f32v4 value);
f64v2 Math_ArcSinH_flt64v2(f64v2 value);

//============================================================================//
//      Inverse hyperbolic cosine                                             //
//============================================================================//
f32 Math_ArcCosH_flt32(f32 value);
f64 Math_ArcCosH_flt64(f64 value);
f32v4 Math_ArcCosH_flt32v4(f32v4 value);
f64v2 Math_ArcCosH_flt64v2(f64v2 value);

//============================================================================//
//      Inverse hyperbolic tangent                                            //
//============================================================================//
f32 Math_ArcTanH_flt32(f32 value);
f64 Math_ArcTanH_flt64(f64 value);
f32v4 Math_ArcTanH_flt32v4(f32v4 value);
f64v2 Math_ArcTanH_flt64v2(f64v2 value);

//****************************************************************************//
//      Rounding                                                              //
//****************************************************************************//

// Round down (floor)
f32 Math_RoundDown_flt32(f32 value);
f64 Math_RoundDown_flt64(f64 value);
f32v4 Math_RoundDown_flt32v4(f32v4 value);
f64v2 Math_RoundDown_flt64v2(f64v2 value);

// Round up (ceil)
f32 Math_RoundUp_flt32(f32 value);
f64 Math_RoundUp_flt64(f64 value);
f32v4 Math_RoundUp_flt32v4(f32v4 value);
f64v2 Math_RoundUp_flt64v2(f64v2 value);

// Round to nearest even integer
f32 Math_RoundInt_flt32(f32 value);
f64 Math_RoundInt_flt64(f64 value);
f32v4 Math_RoundInt_flt32v4(f32v4 value);
f64v2 Math_RoundInt_flt64v2(f64v2 value);

// Round to nearest integer, away from zero
f32 Math_Round_flt32(f32 value);
f64 Math_Round_flt64(f64 value);
f32v4 Math_Round_flt32v4(f32v4 value);
f64v2 Math_Round_flt64v2(f64v2 value);

// Round to nearest integer, toward zero (truncation)
f32 Math_Truncate_flt32(f32 value);
f64 Math_Truncate_flt64(f64 value);
f32v4 Math_Truncate_flt32v4(f32v4 value);
f64v2 Math_Truncate_flt64v2(f64v2 value);

// Fractional part
f32 Math_Frac_flt32(f32 value);
f64 Math_Frac_flt64(f64 value);
f32v4 Math_Frac_flt32v4(f32v4 value);
f64v2 Math_Frac_flt64v2(f64v2 value);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

// Check for normal value
bool Math_IsNormal_flt32(f32 value);
bool Math_IsNormal_flt64(f64 value);

// Check for subnormal value
bool Math_IsSubnormal_flt32(f32 value);
bool Math_IsSubnormal_flt64(f64 value);

// Check for finite value
bool Math_IsFinite_flt32(f32 value);
bool Math_IsFinite_flt64(f64 value);

// Check for infinite value
bool Math_IsInfinite_flt32(f32 value);
bool Math_IsInfinite_flt64(f64 value);

// Check for NaN value
bool Math_IsNaN_flt32(f32 value);
bool Math_IsNaN_flt64(f64 value);
}
# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
