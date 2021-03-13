/*                                                                       Array.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                  COMMON ROUTINES THAT ARE USEFUL FOR ARRAYS                  #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Types.h>

//****************************************************************************//
//      Key compare function prototype                                        //
//****************************************************************************//
typedef s64 (*Cmp) (const void *key1, const void *key2);

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
class Array
{
public:

//****************************************************************************//
//      Initialization                                                        //
//****************************************************************************//

// Unsigned integer types
static void Init (u8 array[], u32 size, u8 value);
static void Init (u16 array[], u32 size, u16 value);
static void Init (u32 array[], u32 size, u32 value);
static void Init (u64 array[], u32 size, u64 value);

// Signed integer types
static void Init (s8 array[], u32 size, s8 value);
static void Init (s16 array[], u32 size, s16 value);
static void Init (s32 array[], u32 size, s32 value);
static void Init (s64 array[], u32 size, s64 value);

// Floating-point types
static void Init (f32 array[], u32 size, f32 value);
static void Init (f64 array[], u32 size, f64 value);

// Other types
static void Init (u32 array[], u32 size, u32 value);

//****************************************************************************//
//      Copying arrays                                                        //
//****************************************************************************//

// Unsigned integer types
static void Copy (u8 target[], const u8 source[], u32 size);
static void Copy (u16 target[], const u16 source[], u32 size);
static void Copy (u32 target[], const u32 source[], u32 size);
static void Copy (u64 target[], const u64 source[], u32 size);

// Signed integer types
static void Copy (s8 target[], const s8 source[], u32 size);
static void Copy (s16 target[], const s16 source[], u32 size);
static void Copy (s32 target[], const s32 source[], u32 size);
static void Copy (s64 target[], const s64 source[], u32 size);

// Floating-point types
static void Copy (f32 target[], const f32 source[], u32 size);
static void Copy (f64 target[], const f64 source[], u32 size);

// Other types
static void Copy (u32 target[], const u32 source[], u32 size);
static void Copy (void *target, const void *source, u32 size);

//****************************************************************************//
//      Moving arrays                                                         //
//****************************************************************************//

// Unsigned integer types
static void Move (u8 target[], u8 source[], u32 size);
static void Move (u16 target[], u16 source[], u32 size);
static void Move (u32 target[], u32 source[], u32 size);
static void Move (u64 target[], u64 source[], u32 size);

// Signed integer types
static void Move (s8 target[], s8 source[], u32 size);
static void Move (s16 target[], s16 source[], u32 size);
static void Move (s32 target[], s32 source[], u32 size);
static void Move (s64 target[], s64 source[], u32 size);

// Floating-point types
static void Move (f32 target[], f32 source[], u32 size);
static void Move (f64 target[], f64 source[], u32 size);

// Other types
static void Move (u32 target[], u32 source[], u32 size);
static void Move (void *target, void *source, u32 size);

//****************************************************************************//
//      Pattern cloning                                                       //
//****************************************************************************//

// Unsigned integer types
static void Clone (u8 array[], u32 size, u32 psize);
static void Clone (u16 array[], u32 size, u32 psize);
static void Clone (u32 array[], u32 size, u32 psize);
static void Clone (u64 array[], u32 size, u32 psize);

// Signed integer types
static void Clone (s8 array[], u32 size, u32 psize);
static void Clone (s16 array[], u32 size, u32 psize);
static void Clone (s32 array[], u32 size, u32 psize);
static void Clone (s64 array[], u32 size, u32 psize);

// Floating-point types
static void Clone (f32 array[], u32 size, u32 psize);
static void Clone (f64 array[], u32 size, u32 psize);

// Other types
static void Clone (u32 array[], u32 size, u32 psize);
static void Clone (void *array, u32 size, u32 psize);

//****************************************************************************//
//      Data conversion                                                       //
//****************************************************************************//

// Conversion between floating-point types
static void ConvertToFlt32 (f32 target[], const f64 source[], u32 size);
static void ConvertToFlt64 (f64 target[], const f32 source[], u32 size);

// Conversion from signed integer types to floating-point types
static void ConvertToFlt32 (f32 target[], const s32 source[], u32 size);
static void ConvertToFlt32 (f32 target[], const s64 source[], u32 size);
static void ConvertToFlt64 (f64 target[], const s32 source[], u32 size);
static void ConvertToFlt64 (f64 target[], const s64 source[], u32 size);

// Conversion from floating-point types to signed integer types
static void ConvertToSint32 (s32 target[], const f32 source[], u32 size);
static void ConvertToSint32 (s32 target[], const f64 source[], u32 size);
static void ConvertToSint64 (s64 target[], const f32 source[], u32 size);
static void ConvertToSint64 (s64 target[], const f64 source[], u32 size);

// Truncating from floating-point types to signed integer types
static void TruncateToSint32 (s32 target[], const f32 source[], u32 size);
static void TruncateToSint32 (s32 target[], const f64 source[], u32 size);
static void TruncateToSint64 (s64 target[], const f32 source[], u32 size);
static void TruncateToSint64 (s64 target[], const f64 source[], u32 size);

//****************************************************************************//
//      Bit field operations                                                  //
//****************************************************************************//

//============================================================================//
//      Get bit value from bit field                                          //
//============================================================================//

// Unsigned integer types
static bool GetBit (const u8 array[], u32 index);
static bool GetBit (const u16 array[], u32 index);
static bool GetBit (const u32 array[], u32 index);
static bool GetBit (const u64 array[], u32 index);

// Signed integer types
static bool GetBit (const s8 array[], u32 index);
static bool GetBit (const s16 array[], u32 index);
static bool GetBit (const s32 array[], u32 index);
static bool GetBit (const s64 array[], u32 index);

// Other types
static bool GetBit (const u32 array[], u32 index);
static bool GetBit (const void *array, u32 index);

//============================================================================//
//      Set bit value in bit field                                            //
//============================================================================//

// Unsigned integer types
static void SetBit (u8 array[], u32 index);
static void SetBit (u16 array[], u32 index);
static void SetBit (u32 array[], u32 index);
static void SetBit (u64 array[], u32 index);

// Signed integer types
static void SetBit (s8 array[], u32 index);
static void SetBit (s16 array[], u32 index);
static void SetBit (s32 array[], u32 index);
static void SetBit (s64 array[], u32 index);

// Other types
static void SetBit (u32 array[], u32 index);
static void SetBit (void *array, u32 index);

//============================================================================//
//      Reset bit value in bit field                                          //
//============================================================================//

// Unsigned integer types
static void ResetBit (u8 array[], u32 index);
static void ResetBit (u16 array[], u32 index);
static void ResetBit (u32 array[], u32 index);
static void ResetBit (u64 array[], u32 index);

// Signed integer types
static void ResetBit (s8 array[], u32 index);
static void ResetBit (s16 array[], u32 index);
static void ResetBit (s32 array[], u32 index);
static void ResetBit (s64 array[], u32 index);

// Other types
static void ResetBit (u32 array[], u32 index);
static void ResetBit (void *array, u32 index);

//============================================================================//
//      Invert bit value in bit field                                         //
//============================================================================//

// Unsigned integer types
static void InvertBit (u8 array[], u32 index);
static void InvertBit (u16 array[], u32 index);
static void InvertBit (u32 array[], u32 index);
static void InvertBit (u64 array[], u32 index);

// Signed integer types
static void InvertBit (s8 array[], u32 index);
static void InvertBit (s16 array[], u32 index);
static void InvertBit (s32 array[], u32 index);
static void InvertBit (s64 array[], u32 index);

// Other types
static void InvertBit (u32 array[], u32 index);
static void InvertBit (void *array, u32 index);

//****************************************************************************//
//      Bitwise operations                                                    //
//****************************************************************************//

//============================================================================//
//      Byte swap                                                             //
//============================================================================//

// Unsigned integer types
static void ByteSwap (u8 array[], u32 size);
static void ByteSwap (u16 array[], u32 size);
static void ByteSwap (u32 array[], u32 size);
static void ByteSwap (u64 array[], u32 size);

// Signed integer types
static void ByteSwap (s8 array[], u32 size);
static void ByteSwap (s16 array[], u32 size);
static void ByteSwap (s32 array[], u32 size);
static void ByteSwap (s64 array[], u32 size);

// Other types
static void ByteSwap (u32 array[], u32 size);

//============================================================================//
//      Bit reversal permutation                                              //
//============================================================================//

// Unsigned integer types
static void BitReverse (u8 array[], u32 size);
static void BitReverse (u16 array[], u32 size);
static void BitReverse (u32 array[], u32 size);
static void BitReverse (u64 array[], u32 size);

// Signed integer types
static void BitReverse (s8 array[], u32 size);
static void BitReverse (s16 array[], u32 size);
static void BitReverse (s32 array[], u32 size);
static void BitReverse (s64 array[], u32 size);

// Other types
static void BitReverse (u32 array[], u32 size);

//============================================================================//
//      Population count                                                      //
//============================================================================//

// Unsigned integer types
static void PopCount (u8 array[], u32 size);
static void PopCount (u16 array[], u32 size);
static void PopCount (u32 array[], u32 size);
static void PopCount (u64 array[], u32 size);

// Signed integer types
static void PopCount (s8 array[], u32 size);
static void PopCount (s16 array[], u32 size);
static void PopCount (s32 array[], u32 size);
static void PopCount (s64 array[], u32 size);

// Other types
static void PopCount (u32 array[], u32 size);

//============================================================================//
//      Bitwise NOT                                                           //
//============================================================================//

// Unsigned integer types
static void Not (u8 array[], u32 size);
static void Not (u16 array[], u32 size);
static void Not (u32 array[], u32 size);
static void Not (u64 array[], u32 size);

// Signed integer types
static void Not (s8 array[], u32 size);
static void Not (s16 array[], u32 size);
static void Not (s32 array[], u32 size);
static void Not (s64 array[], u32 size);

// Other types
static void Not (u32 array[], u32 size);

//============================================================================//
//      Bitwise AND                                                           //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Scalar bitwise AND                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void AndScalar (u8 array[], u32 size, u8 value);
static void AndScalar (u16 array[], u32 size, u16 value);
static void AndScalar (u32 array[], u32 size, u32 value);
static void AndScalar (u64 array[], u32 size, u64 value);

// Signed integer types
static void AndScalar (s8 array[], u32 size, s8 value);
static void AndScalar (s16 array[], u32 size, s16 value);
static void AndScalar (s32 array[], u32 size, s32 value);
static void AndScalar (s64 array[], u32 size, s64 value);

// Other types
static void AndScalar (u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Vector bitwise AND                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void AndVector (u8 target[], const u8 source[], u32 size);
static void AndVector (u16 target[], const u16 source[], u32 size);
static void AndVector (u32 target[], const u32 source[], u32 size);
static void AndVector (u64 target[], const u64 source[], u32 size);

// Signed integer types
static void AndVector (s8 target[], const s8 source[], u32 size);
static void AndVector (s16 target[], const s16 source[], u32 size);
static void AndVector (s32 target[], const s32 source[], u32 size);
static void AndVector (s64 target[], const s64 source[], u32 size);

// Other types
static void AndVector (u32 target[], const u32 source[], u32 size);

//============================================================================//
//      Bitwise OR                                                            //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Scalar bitwise OR                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void OrScalar (u8 array[], u32 size, u8 value);
static void OrScalar (u16 array[], u32 size, u16 value);
static void OrScalar (u32 array[], u32 size, u32 value);
static void OrScalar (u64 array[], u32 size, u64 value);

// Signed integer types
static void OrScalar (s8 array[], u32 size, s8 value);
static void OrScalar (s16 array[], u32 size, s16 value);
static void OrScalar (s32 array[], u32 size, s32 value);
static void OrScalar (s64 array[], u32 size, s64 value);

// Other types
static void OrScalar (u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Vector bitwise OR                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void OrVector (u8 target[], const u8 source[], u32 size);
static void OrVector (u16 target[], const u16 source[], u32 size);
static void OrVector (u32 target[], const u32 source[], u32 size);
static void OrVector (u64 target[], const u64 source[], u32 size);

// Signed integer types
static void OrVector (s8 target[], const s8 source[], u32 size);
static void OrVector (s16 target[], const s16 source[], u32 size);
static void OrVector (s32 target[], const s32 source[], u32 size);
static void OrVector (s64 target[], const s64 source[], u32 size);

// Other types
static void OrVector (u32 target[], const u32 source[], u32 size);

//============================================================================//
//      Bitwise XOR                                                           //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Scalar bitwise XOR                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void XorScalar (u8 array[], u32 size, u8 value);
static void XorScalar (u16 array[], u32 size, u16 value);
static void XorScalar (u32 array[], u32 size, u32 value);
static void XorScalar (u64 array[], u32 size, u64 value);

// Signed integer types
static void XorScalar (s8 array[], u32 size, s8 value);
static void XorScalar (s16 array[], u32 size, s16 value);
static void XorScalar (s32 array[], u32 size, s32 value);
static void XorScalar (s64 array[], u32 size, s64 value);

// Other types
static void XorScalar (u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Vector bitwise XOR                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void XorVector (u8 target[], const u8 source[], u32 size);
static void XorVector (u16 target[], const u16 source[], u32 size);
static void XorVector (u32 target[], const u32 source[], u32 size);
static void XorVector (u64 target[], const u64 source[], u32 size);

// Signed integer types
static void XorVector (s8 target[], const s8 source[], u32 size);
static void XorVector (s16 target[], const s16 source[], u32 size);
static void XorVector (s32 target[], const s32 source[], u32 size);
static void XorVector (s64 target[], const s64 source[], u32 size);

// Other types
static void XorVector (u32 target[], const u32 source[], u32 size);

//****************************************************************************//
//      Arithmetic operations                                                 //
//****************************************************************************//

//============================================================================//
//      Unary operations                                                      //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Negative value                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
static void Neg (s8 array[], u32 size);
static void Neg (s16 array[], u32 size);
static void Neg (s32 array[], u32 size);
static void Neg (s64 array[], u32 size);

// Floating-point types
static void Neg (f32 array[], u32 size);
static void Neg (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Absolute value                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
static void Abs (s8 array[], u32 size);
static void Abs (s16 array[], u32 size);
static void Abs (s32 array[], u32 size);
static void Abs (s64 array[], u32 size);

// Floating-point types
static void Abs (f32 array[], u32 size);
static void Abs (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Negative absolute value                                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
static void NegAbs (s8 array[], u32 size);
static void NegAbs (s16 array[], u32 size);
static void NegAbs (s32 array[], u32 size);
static void NegAbs (s64 array[], u32 size);

// Floating-point types
static void NegAbs (f32 array[], u32 size);
static void NegAbs (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Number sign                                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
static void Sign (s8 array[], u32 size);
static void Sign (s16 array[], u32 size);
static void Sign (s32 array[], u32 size);
static void Sign (s64 array[], u32 size);

// Floating-point types
static void Sign (f32 array[], u32 size);
static void Sign (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Square                                                                //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void Sqr (f32 array[], u32 size);
static void Sqr (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Square root                                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void Sqrt (f32 array[], u32 size);
static void Sqrt (f64 array[], u32 size);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Addition                                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar addition                                                       //
//----------------------------------------------------------------------------//

// Unsigned integer types
static void AddScalar (u8 array[], u32 size, u8 value);
static void AddScalar (u16 array[], u32 size, u16 value);
static void AddScalar (u32 array[], u32 size, u32 value);
static void AddScalar (u64 array[], u32 size, u64 value);

// Signed integer types
static void AddScalar (s8 array[], u32 size, s8 value);
static void AddScalar (s16 array[], u32 size, s16 value);
static void AddScalar (s32 array[], u32 size, s32 value);
static void AddScalar (s64 array[], u32 size, s64 value);

// Floating-point types
static void AddScalar (f32 array[], u32 size, f32 value);
static void AddScalar (f64 array[], u32 size, f64 value);

// Other types
static void AddScalar (u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Vector addition                                                       //
//----------------------------------------------------------------------------//

// Unsigned integer types
static void AddVector (u8 target[], const u8 source[], u32 size);
static void AddVector (u16 target[], const u16 source[], u32 size);
static void AddVector (u32 target[], const u32 source[], u32 size);
static void AddVector (u64 target[], const u64 source[], u32 size);

// Signed integer types
static void AddVector (s8 target[], const s8 source[], u32 size);
static void AddVector (s16 target[], const s16 source[], u32 size);
static void AddVector (s32 target[], const s32 source[], u32 size);
static void AddVector (s64 target[], const s64 source[], u32 size);

// Floating-point types
static void AddVector (f32 target[], const f32 source[], u32 size);
static void AddVector (f64 target[], const f64 source[], u32 size);

// Other types
static void AddVector (u32 target[], const u32 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Subtraction                                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar subtraction                                                    //
//----------------------------------------------------------------------------//

// Unsigned integer types
static void SubScalar (u8 array[], u32 size, u8 value);
static void SubScalar (u16 array[], u32 size, u16 value);
static void SubScalar (u32 array[], u32 size, u32 value);
static void SubScalar (u64 array[], u32 size, u64 value);

// Signed integer types
static void SubScalar (s8 array[], u32 size, s8 value);
static void SubScalar (s16 array[], u32 size, s16 value);
static void SubScalar (s32 array[], u32 size, s32 value);
static void SubScalar (s64 array[], u32 size, s64 value);

// Floating-point types
static void SubScalar (f32 array[], u32 size, f32 value);
static void SubScalar (f64 array[], u32 size, f64 value);

// Other types
static void SubScalar (u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Vector subtraction                                                    //
//----------------------------------------------------------------------------//

// Unsigned integer types
static void SubVector (u8 target[], const u8 source[], u32 size);
static void SubVector (u16 target[], const u16 source[], u32 size);
static void SubVector (u32 target[], const u32 source[], u32 size);
static void SubVector (u64 target[], const u64 source[], u32 size);

// Signed integer types
static void SubVector (s8 target[], const s8 source[], u32 size);
static void SubVector (s16 target[], const s16 source[], u32 size);
static void SubVector (s32 target[], const s32 source[], u32 size);
static void SubVector (s64 target[], const s64 source[], u32 size);

// Floating-point types
static void SubVector (f32 target[], const f32 source[], u32 size);
static void SubVector (f64 target[], const f64 source[], u32 size);

// Other types
static void SubVector (u32 target[], const u32 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Reverse subtraction                                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar reverse subtraction                                            //
//----------------------------------------------------------------------------//

// Unsigned integer types
static void ReverseSubScalar (u8 array[], u32 size, u8 value);
static void ReverseSubScalar (u16 array[], u32 size, u16 value);
static void ReverseSubScalar (u32 array[], u32 size, u32 value);
static void ReverseSubScalar (u64 array[], u32 size, u64 value);

// Signed integer types
static void ReverseSubScalar (s8 array[], u32 size, s8 value);
static void ReverseSubScalar (s16 array[], u32 size, s16 value);
static void ReverseSubScalar (s32 array[], u32 size, s32 value);
static void ReverseSubScalar (s64 array[], u32 size, s64 value);

// Floating-point types
static void ReverseSubScalar (f32 array[], u32 size, f32 value);
static void ReverseSubScalar (f64 array[], u32 size, f64 value);

// Other types
static void ReverseSubScalar (u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Vector reverse subtraction                                            //
//----------------------------------------------------------------------------//

// Unsigned integer types
static void ReverseSubVector (u8 target[], const u8 source[], u32 size);
static void ReverseSubVector (u16 target[], const u16 source[], u32 size);
static void ReverseSubVector (u32 target[], const u32 source[], u32 size);
static void ReverseSubVector (u64 target[], const u64 source[], u32 size);

// Signed integer types
static void ReverseSubVector (s8 target[], const s8 source[], u32 size);
static void ReverseSubVector (s16 target[], const s16 source[], u32 size);
static void ReverseSubVector (s32 target[], const s32 source[], u32 size);
static void ReverseSubVector (s64 target[], const s64 source[], u32 size);

// Floating-point types
static void ReverseSubVector (f32 target[], const f32 source[], u32 size);
static void ReverseSubVector (f64 target[], const f64 source[], u32 size);

// Other types
static void ReverseSubVector (u32 target[], const u32 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Multiplication                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar multiplication                                                 //
//----------------------------------------------------------------------------//
static void MulScalar (f32 array[], u32 size, f32 value);
static void MulScalar (f64 array[], u32 size, f64 value);

//----------------------------------------------------------------------------//
//      Vector multiplication                                                 //
//----------------------------------------------------------------------------//
static void MulVector (f32 target[], const f32 source[], u32 size);
static void MulVector (f64 target[], const f64 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Division                                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar division                                                       //
//----------------------------------------------------------------------------//
static void DivScalar (f32 array[], u32 size, f32 value);
static void DivScalar (f64 array[], u32 size, f64 value);

//----------------------------------------------------------------------------//
//      Vector division                                                       //
//----------------------------------------------------------------------------//
static void DivVector (f32 target[], const f32 source[], u32 size);
static void DivVector (f64 target[], const f64 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Reverse division                                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar reverse division                                               //
//----------------------------------------------------------------------------//
static void ReverseDivScalar (f32 array[], u32 size, f32 value);
static void ReverseDivScalar (f64 array[], u32 size, f64 value);

//----------------------------------------------------------------------------//
//      Vector reverse division                                               //
//----------------------------------------------------------------------------//
static void ReverseDivVector (f32 target[], const f32 source[], u32 size);
static void ReverseDivVector (f64 target[], const f64 source[], u32 size);

//****************************************************************************//
//      Rounding                                                              //
//****************************************************************************//

// Round down (floor)
static void RoundDown (f32 array[], u32 size);
static void RoundDown (f64 array[], u32 size);

// Round up (ceil)
static void RoundUp (f32 array[], u32 size);
static void RoundUp (f64 array[], u32 size);

// Round to nearest even integer
static void RoundInt (f32 array[], u32 size);
static void RoundInt (f64 array[], u32 size);

// Round to nearest integer, away from zero
static void Round (f32 array[], u32 size);
static void Round (f64 array[], u32 size);

// Round to nearest integer, toward zero (truncation)
static void Truncate (f32 array[], u32 size);
static void Truncate (f64 array[], u32 size);

// Fractional part
static void Frac (f32 array[], u32 size);
static void Frac (f64 array[], u32 size);

//****************************************************************************//
//      Numerical integration                                                 //
//****************************************************************************//

// Sum of elements
static f32 Sum (const f32 array[], u32 size);
static f64 Sum (const f64 array[], u32 size);

// Sum of squares
static f32 SumSqr (const f32 array[], u32 size);
static f64 SumSqr (const f64 array[], u32 size);

// Sum of absolute values
static f32 SumAbs (const f32 array[], u32 size);
static f64 SumAbs (const f64 array[], u32 size);

// Sum of multiplied values
static f32 SumMul (const f32 array1[], const f32 array2[], u32 size);
static f64 SumMul (const f64 array1[], const f64 array2[], u32 size);

// Sum of squared differences
static f32 SumSqrDiff (const f32 array1[], const f32 array2[], u32 size);
static f64 SumSqrDiff (const f64 array1[], const f64 array2[], u32 size);

// Sum of absolute differences
static f32 SumAbsDiff (const f32 array1[], const f32 array2[], u32 size);
static f64 SumAbsDiff (const f64 array1[], const f64 array2[], u32 size);

//****************************************************************************//
//      Minimum and maximum absolute value                                    //
//****************************************************************************//

//============================================================================//
//      Minimum absolute value                                                //
//============================================================================//

// Signed integer types
static u8 MinAbs (const s8 array[], u32 size);
static u16 MinAbs (const s16 array[], u32 size);
static u32 MinAbs (const s32 array[], u32 size);

// Floating-point types
static f32 MinAbs (const f32 array[], u32 size);
static f64 MinAbs (const f64 array[], u32 size);

//============================================================================//
//      Maximum absolute value                                                //
//============================================================================//

// Signed integer types
static u8 MaxAbs (const s8 array[], u32 size);
static u16 MaxAbs (const s16 array[], u32 size);
static u32 MaxAbs (const s32 array[], u32 size);

// Floating-point types
static f32 MaxAbs (const f32 array[], u32 size);
static f64 MaxAbs (const f64 array[], u32 size);

//****************************************************************************//
//      Minimum and maximum value                                             //
//****************************************************************************//

//============================================================================//
//      Regular array search                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Minimum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u8 Min (const u8 array[], u32 size);
static u16 Min (const u16 array[], u32 size);
static u32 Min (const u32 array[], u32 size);

// Signed integer types
static s8 Min (const s8 array[], u32 size);
static s16 Min (const s16 array[], u32 size);
static s32 Min (const s32 array[], u32 size);

// Floating-point types
static f32 Min (const f32 array[], u32 size);
static f64 Min (const f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Maximum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u8 Max (const u8 array[], u32 size);
static u16 Max (const u16 array[], u32 size);
static u32 Max (const u32 array[], u32 size);

// Signed integer types
static s8 Max (const s8 array[], u32 size);
static s16 Max (const s16 array[], u32 size);
static s32 Max (const s32 array[], u32 size);

// Floating-point types
static f32 Max (const f32 array[], u32 size);
static f64 Max (const f64 array[], u32 size);

//============================================================================//
//      Object array search                                                   //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Minimum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u32 MinObjFwd (const void* array[], u32 size, Cmp func);
static u32 MinObjBwd (const void* array[], u32 size, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Maximum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u32 MaxObjFwd (const void* array[], u32 size, Cmp func);
static u32 MaxObjBwd (const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Linear search                                                         //
//****************************************************************************//

//============================================================================//
//      Bit field search                                                      //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Forward direction search                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for set bit                                                 //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindSetBitFwd (const u8 array[], u32 spos, u32 epos);
static u32 FindSetBitFwd (const u16 array[], u32 spos, u32 epos);
static u32 FindSetBitFwd (const u32 array[], u32 spos, u32 epos);
static u32 FindSetBitFwd (const u64 array[], u32 spos, u32 epos);

// Signed integer types
static u32 FindSetBitFwd (const s8 array[], u32 spos, u32 epos);
static u32 FindSetBitFwd (const s16 array[], u32 spos, u32 epos);
static u32 FindSetBitFwd (const s32 array[], u32 spos, u32 epos);
static u32 FindSetBitFwd (const s64 array[], u32 spos, u32 epos);

// Other types
static u32 FindSetBitFwd (const u32 array[], u32 spos, u32 epos);
static u32 FindSetBitFwd (const void *array, u32 spos, u32 epos);

//----------------------------------------------------------------------------//
//      Searching for reset bit                                               //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindResetBitFwd (const u8 array[], u32 spos, u32 epos);
static u32 FindResetBitFwd (const u16 array[], u32 spos, u32 epos);
static u32 FindResetBitFwd (const u32 array[], u32 spos, u32 epos);
static u32 FindResetBitFwd (const u64 array[], u32 spos, u32 epos);

// Signed integer types
static u32 FindResetBitFwd (const s8 array[], u32 spos, u32 epos);
static u32 FindResetBitFwd (const s16 array[], u32 spos, u32 epos);
static u32 FindResetBitFwd (const s32 array[], u32 spos, u32 epos);
static u32 FindResetBitFwd (const s64 array[], u32 spos, u32 epos);

// Other types
static u32 FindResetBitFwd (const u32 array[], u32 spos, u32 epos);
static u32 FindResetBitFwd (const void *array, u32 spos, u32 epos);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Backward direction search                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for set bit                                                 //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindSetBitBwd (const u8 array[], u32 spos, u32 epos);
static u32 FindSetBitBwd (const u16 array[], u32 spos, u32 epos);
static u32 FindSetBitBwd (const u32 array[], u32 spos, u32 epos);
static u32 FindSetBitBwd (const u64 array[], u32 spos, u32 epos);

// Signed integer types
static u32 FindSetBitBwd (const s8 array[], u32 spos, u32 epos);
static u32 FindSetBitBwd (const s16 array[], u32 spos, u32 epos);
static u32 FindSetBitBwd (const s32 array[], u32 spos, u32 epos);
static u32 FindSetBitBwd (const s64 array[], u32 spos, u32 epos);

// Other types
static u32 FindSetBitBwd (const u32 array[], u32 spos, u32 epos);
static u32 FindSetBitBwd (const void *array, u32 spos, u32 epos);

//----------------------------------------------------------------------------//
//      Searching for reset bit                                               //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindResetBitBwd (const u8 array[], u32 spos, u32 epos);
static u32 FindResetBitBwd (const u16 array[], u32 spos, u32 epos);
static u32 FindResetBitBwd (const u32 array[], u32 spos, u32 epos);
static u32 FindResetBitBwd (const u64 array[], u32 spos, u32 epos);

// Signed integer types
static u32 FindResetBitBwd (const s8 array[], u32 spos, u32 epos);
static u32 FindResetBitBwd (const s16 array[], u32 spos, u32 epos);
static u32 FindResetBitBwd (const s32 array[], u32 spos, u32 epos);
static u32 FindResetBitBwd (const s64 array[], u32 spos, u32 epos);

// Other types
static u32 FindResetBitBwd (const u32 array[], u32 spos, u32 epos);
static u32 FindResetBitBwd (const void *array, u32 spos, u32 epos);

//============================================================================//
//      Regular array search                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Forward direction search                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u32 FindFwd (const u8 array[], u32 size, u8 value);
static u32 FindFwd (const u16 array[], u32 size, u16 value);
static u32 FindFwd (const u32 array[], u32 size, u32 value);
static u32 FindFwd (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindFwd (const s8 array[], u32 size, s8 value);
static u32 FindFwd (const s16 array[], u32 size, s16 value);
static u32 FindFwd (const s32 array[], u32 size, s32 value);
static u32 FindFwd (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindFwd (const u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Backward direction search                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u32 FindBwd (const u8 array[], u32 size, u8 value);
static u32 FindBwd (const u16 array[], u32 size, u16 value);
static u32 FindBwd (const u32 array[], u32 size, u32 value);
static u32 FindBwd (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindBwd (const s8 array[], u32 size, s8 value);
static u32 FindBwd (const s16 array[], u32 size, s16 value);
static u32 FindBwd (const s32 array[], u32 size, s32 value);
static u32 FindBwd (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindBwd (const u32 array[], u32 size, u32 value);

//============================================================================//
//      Object array search                                                   //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Forward direction search                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u32 FindObjFwd (const void* array[], u32 size, const void *value, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Backward direction search                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u32 FindObjBwd (const void* array[], u32 size, const void *value, Cmp func);

//****************************************************************************//
//      Binary search                                                         //
//****************************************************************************//

//============================================================================//
//      Regular array search                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for first equal element                                     //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindFirstEqualAsc (const u8 array[], u32 size, u8 value);
static u32 FindFirstEqualAsc (const u16 array[], u32 size, u16 value);
static u32 FindFirstEqualAsc (const u32 array[], u32 size, u32 value);
static u32 FindFirstEqualAsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindFirstEqualAsc (const s8 array[], u32 size, s8 value);
static u32 FindFirstEqualAsc (const s16 array[], u32 size, s16 value);
static u32 FindFirstEqualAsc (const s32 array[], u32 size, s32 value);
static u32 FindFirstEqualAsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindFirstEqualAsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for last equal element                                      //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindLastEqualAsc (const u8 array[], u32 size, u8 value);
static u32 FindLastEqualAsc (const u16 array[], u32 size, u16 value);
static u32 FindLastEqualAsc (const u32 array[], u32 size, u32 value);
static u32 FindLastEqualAsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindLastEqualAsc (const s8 array[], u32 size, s8 value);
static u32 FindLastEqualAsc (const s16 array[], u32 size, s16 value);
static u32 FindLastEqualAsc (const s32 array[], u32 size, s32 value);
static u32 FindLastEqualAsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindLastEqualAsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for greater element                                         //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindGreatAsc (const u8 array[], u32 size, u8 value);
static u32 FindGreatAsc (const u16 array[], u32 size, u16 value);
static u32 FindGreatAsc (const u32 array[], u32 size, u32 value);
static u32 FindGreatAsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindGreatAsc (const s8 array[], u32 size, s8 value);
static u32 FindGreatAsc (const s16 array[], u32 size, s16 value);
static u32 FindGreatAsc (const s32 array[], u32 size, s32 value);
static u32 FindGreatAsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindGreatAsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for greater or equal element                                //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindGreatOrEqualAsc (const u8 array[], u32 size, u8 value);
static u32 FindGreatOrEqualAsc (const u16 array[], u32 size, u16 value);
static u32 FindGreatOrEqualAsc (const u32 array[], u32 size, u32 value);
static u32 FindGreatOrEqualAsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindGreatOrEqualAsc (const s8 array[], u32 size, s8 value);
static u32 FindGreatOrEqualAsc (const s16 array[], u32 size, s16 value);
static u32 FindGreatOrEqualAsc (const s32 array[], u32 size, s32 value);
static u32 FindGreatOrEqualAsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindGreatOrEqualAsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for less element                                            //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindLessAsc (const u8 array[], u32 size, u8 value);
static u32 FindLessAsc (const u16 array[], u32 size, u16 value);
static u32 FindLessAsc (const u32 array[], u32 size, u32 value);
static u32 FindLessAsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindLessAsc (const s8 array[], u32 size, s8 value);
static u32 FindLessAsc (const s16 array[], u32 size, s16 value);
static u32 FindLessAsc (const s32 array[], u32 size, s32 value);
static u32 FindLessAsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindLessAsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for less or equal element                                   //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindLessOrEqualAsc (const u8 array[], u32 size, u8 value);
static u32 FindLessOrEqualAsc (const u16 array[], u32 size, u16 value);
static u32 FindLessOrEqualAsc (const u32 array[], u32 size, u32 value);
static u32 FindLessOrEqualAsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindLessOrEqualAsc (const s8 array[], u32 size, s8 value);
static u32 FindLessOrEqualAsc (const s16 array[], u32 size, s16 value);
static u32 FindLessOrEqualAsc (const s32 array[], u32 size, s32 value);
static u32 FindLessOrEqualAsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindLessOrEqualAsc (const u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for first equal element                                     //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindFirstEqualDsc (const u8 array[], u32 size, u8 value);
static u32 FindFirstEqualDsc (const u16 array[], u32 size, u16 value);
static u32 FindFirstEqualDsc (const u32 array[], u32 size, u32 value);
static u32 FindFirstEqualDsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindFirstEqualDsc (const s8 array[], u32 size, s8 value);
static u32 FindFirstEqualDsc (const s16 array[], u32 size, s16 value);
static u32 FindFirstEqualDsc (const s32 array[], u32 size, s32 value);
static u32 FindFirstEqualDsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindFirstEqualDsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for last equal element                                      //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindLastEqualDsc (const u8 array[], u32 size, u8 value);
static u32 FindLastEqualDsc (const u16 array[], u32 size, u16 value);
static u32 FindLastEqualDsc (const u32 array[], u32 size, u32 value);
static u32 FindLastEqualDsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindLastEqualDsc (const s8 array[], u32 size, s8 value);
static u32 FindLastEqualDsc (const s16 array[], u32 size, s16 value);
static u32 FindLastEqualDsc (const s32 array[], u32 size, s32 value);
static u32 FindLastEqualDsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindLastEqualDsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for less element                                            //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindLessDsc (const u8 array[], u32 size, u8 value);
static u32 FindLessDsc (const u16 array[], u32 size, u16 value);
static u32 FindLessDsc (const u32 array[], u32 size, u32 value);
static u32 FindLessDsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindLessDsc (const s8 array[], u32 size, s8 value);
static u32 FindLessDsc (const s16 array[], u32 size, s16 value);
static u32 FindLessDsc (const s32 array[], u32 size, s32 value);
static u32 FindLessDsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindLessDsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for less or equal element                                   //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindLessOrEqualDsc (const u8 array[], u32 size, u8 value);
static u32 FindLessOrEqualDsc (const u16 array[], u32 size, u16 value);
static u32 FindLessOrEqualDsc (const u32 array[], u32 size, u32 value);
static u32 FindLessOrEqualDsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindLessOrEqualDsc (const s8 array[], u32 size, s8 value);
static u32 FindLessOrEqualDsc (const s16 array[], u32 size, s16 value);
static u32 FindLessOrEqualDsc (const s32 array[], u32 size, s32 value);
static u32 FindLessOrEqualDsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindLessOrEqualDsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for greater element                                         //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindGreatDsc (const u8 array[], u32 size, u8 value);
static u32 FindGreatDsc (const u16 array[], u32 size, u16 value);
static u32 FindGreatDsc (const u32 array[], u32 size, u32 value);
static u32 FindGreatDsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindGreatDsc (const s8 array[], u32 size, s8 value);
static u32 FindGreatDsc (const s16 array[], u32 size, s16 value);
static u32 FindGreatDsc (const s32 array[], u32 size, s32 value);
static u32 FindGreatDsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindGreatDsc (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for greater or equal element                                //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 FindGreatOrEqualDsc (const u8 array[], u32 size, u8 value);
static u32 FindGreatOrEqualDsc (const u16 array[], u32 size, u16 value);
static u32 FindGreatOrEqualDsc (const u32 array[], u32 size, u32 value);
static u32 FindGreatOrEqualDsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 FindGreatOrEqualDsc (const s8 array[], u32 size, s8 value);
static u32 FindGreatOrEqualDsc (const s16 array[], u32 size, s16 value);
static u32 FindGreatOrEqualDsc (const s32 array[], u32 size, s32 value);
static u32 FindGreatOrEqualDsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 FindGreatOrEqualDsc (const u32 array[], u32 size, u32 value);

//============================================================================//
//      Object array search                                                   //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for first equal element                                     //
//----------------------------------------------------------------------------//
static u32 FindFirstEqualObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for last equal element                                      //
//----------------------------------------------------------------------------//
static u32 FindLastEqualObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for greater element                                         //
//----------------------------------------------------------------------------//
static u32 FindGreatObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for greater or equal element                                //
//----------------------------------------------------------------------------//
static u32 FindGreatOrEqualObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for less element                                            //
//----------------------------------------------------------------------------//
static u32 FindLessObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for less or equal element                                   //
//----------------------------------------------------------------------------//
static u32 FindLessOrEqualObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for first equal element                                     //
//----------------------------------------------------------------------------//
static u32 FindFirstEqualObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for last equal element                                      //
//----------------------------------------------------------------------------//
static u32 FindLastEqualObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for less element                                            //
//----------------------------------------------------------------------------//
static u32 FindLessObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for less or equal element                                   //
//----------------------------------------------------------------------------//
static u32 FindLessOrEqualObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for greater element                                         //
//----------------------------------------------------------------------------//
static u32 FindGreatObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for greater or equal element                                //
//----------------------------------------------------------------------------//
static u32 FindGreatOrEqualObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//****************************************************************************//
//      Linear counting                                                       //
//****************************************************************************//

//============================================================================//
//      Bit counting                                                          //
//============================================================================//

// Unsigned integer types
static u32 CountBits (const u8 array[], u32 spos, u32 epos);
static u32 CountBits (const u16 array[], u32 spos, u32 epos);
static u32 CountBits (const u32 array[], u32 spos, u32 epos);
static u32 CountBits (const u64 array[], u32 spos, u32 epos);

// Signed integer types
static u32 CountBits (const s8 array[], u32 spos, u32 epos);
static u32 CountBits (const s16 array[], u32 spos, u32 epos);
static u32 CountBits (const s32 array[], u32 spos, u32 epos);
static u32 CountBits (const s64 array[], u32 spos, u32 epos);

// Other types
static u32 CountBits (const u32 array[], u32 spos, u32 epos);
static u32 CountBits (const void *array, u32 spos, u32 epos);

//============================================================================//
//      Element counting                                                      //
//============================================================================//

// Unsigned integer types
static u32 Count (const u8 array[], u32 size, u8 value);
static u32 Count (const u16 array[], u32 size, u16 value);
static u32 Count (const u32 array[], u32 size, u32 value);
static u32 Count (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 Count (const s8 array[], u32 size, s8 value);
static u32 Count (const s16 array[], u32 size, s16 value);
static u32 Count (const s32 array[], u32 size, s32 value);
static u32 Count (const s64 array[], u32 size, s64 value);

// Other types
static u32 Count (const u32 array[], u32 size, u32 value);

//============================================================================//
//      Object counting                                                       //
//============================================================================//
static u32 CountObj (const void* array[], u32 size, const void *value, Cmp func);

//****************************************************************************//
//      Binary counting                                                       //
//****************************************************************************//

//============================================================================//
//      Element counting                                                      //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u32 CountAsc (const u8 array[], u32 size, u8 value);
static u32 CountAsc (const u16 array[], u32 size, u16 value);
static u32 CountAsc (const u32 array[], u32 size, u32 value);
static u32 CountAsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 CountAsc (const s8 array[], u32 size, s8 value);
static u32 CountAsc (const s16 array[], u32 size, s16 value);
static u32 CountAsc (const s32 array[], u32 size, s32 value);
static u32 CountAsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 CountAsc (const u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u32 CountDsc (const u8 array[], u32 size, u8 value);
static u32 CountDsc (const u16 array[], u32 size, u16 value);
static u32 CountDsc (const u32 array[], u32 size, u32 value);
static u32 CountDsc (const u64 array[], u32 size, u64 value);

// Signed integer types
static u32 CountDsc (const s8 array[], u32 size, s8 value);
static u32 CountDsc (const s16 array[], u32 size, s16 value);
static u32 CountDsc (const s32 array[], u32 size, s32 value);
static u32 CountDsc (const s64 array[], u32 size, s64 value);

// Other types
static u32 CountDsc (const u32 array[], u32 size, u32 value);

//============================================================================//
//      Object counting                                                       //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u32 CountObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u32 CountObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//****************************************************************************//
//      Replacing                                                             //
//****************************************************************************//

//============================================================================//
//      Element replacing                                                     //
//============================================================================//

// Unsigned integer types
static void Replace (u8 array[], u32 size, u8 pattern, u8 value);
static void Replace (u16 array[], u32 size, u16 pattern, u16 value);
static void Replace (u32 array[], u32 size, u32 pattern, u32 value);
static void Replace (u64 array[], u32 size, u64 pattern, u64 value);

// Signed integer types
static void Replace (s8 array[], u32 size, s8 pattern, s8 value);
static void Replace (s16 array[], u32 size, s16 pattern, s16 value);
static void Replace (s32 array[], u32 size, s32 pattern, s32 value);
static void Replace (s64 array[], u32 size, s64 pattern, s64 value);

// Other types
static void Replace (u32 array[], u32 size, u32 pattern, u32 value);

//============================================================================//
//      Object replacing                                                      //
//============================================================================//
static void ReplaceObj (const void* array[], u32 size, const void *pattern, const void *value, Cmp func);

//****************************************************************************//
//      Order reversing                                                       //
//****************************************************************************//

//============================================================================//
//      Regular array reversing                                               //
//============================================================================//

// Unsigned integer types
static void Reverse (u8 array[], u32 size);
static void Reverse (u16 array[], u32 size);
static void Reverse (u32 array[], u32 size);
static void Reverse (u64 array[], u32 size);

// Signed integer types
static void Reverse (s8 array[], u32 size);
static void Reverse (s16 array[], u32 size);
static void Reverse (s32 array[], u32 size);
static void Reverse (s64 array[], u32 size);

// Floating-point types
static void Reverse (f32 array[], u32 size);
static void Reverse (f64 array[], u32 size);

// Other types
static void Reverse (u32 array[], u32 size);

//============================================================================//
//      Object array reversing                                                //
//============================================================================//
static void ReverseObj (const void* array[], u32 size);

//****************************************************************************//
//      Unique values                                                         //
//****************************************************************************//

//============================================================================//
//      Unique elements                                                       //
//============================================================================//

// Unsigned integer types
static u32 Unique (u8 unique[], const u8 array[], u32 size);
static u32 Unique (u16 unique[], const u16 array[], u32 size);
static u32 Unique (u32 unique[], const u32 array[], u32 size);
static u32 Unique (u64 unique[], const u64 array[], u32 size);

// Signed integer types
static u32 Unique (s8 unique[], const s8 array[], u32 size);
static u32 Unique (s16 unique[], const s16 array[], u32 size);
static u32 Unique (s32 unique[], const s32 array[], u32 size);
static u32 Unique (s64 unique[], const s64 array[], u32 size);

// Other types
static u32 Unique (u32 unique[], const u32 array[], u32 size);

//============================================================================//
//      Unique objects                                                        //
//============================================================================//
static u32 UniqueObj (const void* unique[], const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Duplicate values                                                      //
//****************************************************************************//

//============================================================================//
//      Duplicate elements                                                    //
//============================================================================//

// Unsigned integer types
static u32 Duplicates (u8 unique[], u32 count[], const u8 array[], u32 size);
static u32 Duplicates (u16 unique[], u32 count[], const u16 array[], u32 size);
static u32 Duplicates (u32 unique[], u32 count[], const u32 array[], u32 size);
static u32 Duplicates (u64 unique[], u32 count[], const u64 array[], u32 size);

// Signed integer types
static u32 Duplicates (s8 unique[], u32 count[], const s8 array[], u32 size);
static u32 Duplicates (s16 unique[], u32 count[], const s16 array[], u32 size);
static u32 Duplicates (s32 unique[], u32 count[], const s32 array[], u32 size);
static u32 Duplicates (s64 unique[], u32 count[], const s64 array[], u32 size);

// Other types
static u32 Duplicates (u32 unique[], u32 count[], const u32 array[], u32 size);

//============================================================================//
//      Duplicate objects                                                     //
//============================================================================//
static u32 DuplicatesObj (const void* unique[], u32 count[], const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Insertion sort                                                        //
//****************************************************************************//

//============================================================================//
//      Regular array sorting                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void InsertSortAsc (u8 array[], u32 size);
static void InsertSortAsc (u16 array[], u32 size);
static void InsertSortAsc (u32 array[], u32 size);
static void InsertSortAsc (u64 array[], u32 size);

// Signed integer types
static void InsertSortAsc (s8 array[], u32 size);
static void InsertSortAsc (s16 array[], u32 size);
static void InsertSortAsc (s32 array[], u32 size);
static void InsertSortAsc (s64 array[], u32 size);

// Floating-point types
static void InsertSortAsc (f32 array[], u32 size);
static void InsertSortAsc (f64 array[], u32 size);

// Other types
static void InsertSortAsc (u32 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void InsertSortDsc (u8 array[], u32 size);
static void InsertSortDsc (u16 array[], u32 size);
static void InsertSortDsc (u32 array[], u32 size);
static void InsertSortDsc (u64 array[], u32 size);

// Signed integer types
static void InsertSortDsc (s8 array[], u32 size);
static void InsertSortDsc (s16 array[], u32 size);
static void InsertSortDsc (s32 array[], u32 size);
static void InsertSortDsc (s64 array[], u32 size);

// Floating-point types
static void InsertSortDsc (f32 array[], u32 size);
static void InsertSortDsc (f64 array[], u32 size);

// Other types
static void InsertSortDsc (u32 array[], u32 size);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void InsertSortKeyAsc (u8 key[], const void* ptr[], u32 size);
static void InsertSortKeyAsc (u16 key[], const void* ptr[], u32 size);
static void InsertSortKeyAsc (u32 key[], const void* ptr[], u32 size);
static void InsertSortKeyAsc (u64 key[], const void* ptr[], u32 size);

// Signed integer types
static void InsertSortKeyAsc (s8 key[], const void* ptr[], u32 size);
static void InsertSortKeyAsc (s16 key[], const void* ptr[], u32 size);
static void InsertSortKeyAsc (s32 key[], const void* ptr[], u32 size);
static void InsertSortKeyAsc (s64 key[], const void* ptr[], u32 size);

// Floating-point types
static void InsertSortKeyAsc (f32 key[], const void* ptr[], u32 size);
static void InsertSortKeyAsc (f64 key[], const void* ptr[], u32 size);

// Other types
static void InsertSortKeyAsc (u32 key[], const void* ptr[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void InsertSortKeyDsc (u8 key[], const void* ptr[], u32 size);
static void InsertSortKeyDsc (u16 key[], const void* ptr[], u32 size);
static void InsertSortKeyDsc (u32 key[], const void* ptr[], u32 size);
static void InsertSortKeyDsc (u64 key[], const void* ptr[], u32 size);

// Signed integer types
static void InsertSortKeyDsc (s8 key[], const void* ptr[], u32 size);
static void InsertSortKeyDsc (s16 key[], const void* ptr[], u32 size);
static void InsertSortKeyDsc (s32 key[], const void* ptr[], u32 size);
static void InsertSortKeyDsc (s64 key[], const void* ptr[], u32 size);

// Floating-point types
static void InsertSortKeyDsc (f32 key[], const void* ptr[], u32 size);
static void InsertSortKeyDsc (f64 key[], const void* ptr[], u32 size);

// Other types
static void InsertSortKeyDsc (u32 key[], const void* ptr[], u32 size);

//============================================================================//
//      Object array sorting                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void InsertSortObjAsc (const void* array[], u32 size, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void InsertSortObjDsc (const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Quick sort                                                            //
//****************************************************************************//

//============================================================================//
//      Regular array sorting                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void QuickSortAsc (u8 array[], u32 size);
static void QuickSortAsc (u16 array[], u32 size);
static void QuickSortAsc (u32 array[], u32 size);
static void QuickSortAsc (u64 array[], u32 size);

// Signed integer types
static void QuickSortAsc (s8 array[], u32 size);
static void QuickSortAsc (s16 array[], u32 size);
static void QuickSortAsc (s32 array[], u32 size);
static void QuickSortAsc (s64 array[], u32 size);

// Floating-point types
static void QuickSortAsc (f32 array[], u32 size);
static void QuickSortAsc (f64 array[], u32 size);

// Other types
static void QuickSortAsc (u32 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void QuickSortDsc (u8 array[], u32 size);
static void QuickSortDsc (u16 array[], u32 size);
static void QuickSortDsc (u32 array[], u32 size);
static void QuickSortDsc (u64 array[], u32 size);

// Signed integer types
static void QuickSortDsc (s8 array[], u32 size);
static void QuickSortDsc (s16 array[], u32 size);
static void QuickSortDsc (s32 array[], u32 size);
static void QuickSortDsc (s64 array[], u32 size);

// Floating-point types
static void QuickSortDsc (f32 array[], u32 size);
static void QuickSortDsc (f64 array[], u32 size);

// Other types
static void QuickSortDsc (u32 array[], u32 size);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void QuickSortKeyAsc (u8 key[], const void* ptr[], u32 size);
static void QuickSortKeyAsc (u16 key[], const void* ptr[], u32 size);
static void QuickSortKeyAsc (u32 key[], const void* ptr[], u32 size);
static void QuickSortKeyAsc (u64 key[], const void* ptr[], u32 size);

// Signed integer types
static void QuickSortKeyAsc (s8 key[], const void* ptr[], u32 size);
static void QuickSortKeyAsc (s16 key[], const void* ptr[], u32 size);
static void QuickSortKeyAsc (s32 key[], const void* ptr[], u32 size);
static void QuickSortKeyAsc (s64 key[], const void* ptr[], u32 size);

// Floating-point types
static void QuickSortKeyAsc (f32 key[], const void* ptr[], u32 size);
static void QuickSortKeyAsc (f64 key[], const void* ptr[], u32 size);

// Other types
static void QuickSortKeyAsc (u32 key[], const void* ptr[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void QuickSortKeyDsc (u8 key[], const void* ptr[], u32 size);
static void QuickSortKeyDsc (u16 key[], const void* ptr[], u32 size);
static void QuickSortKeyDsc (u32 key[], const void* ptr[], u32 size);
static void QuickSortKeyDsc (u64 key[], const void* ptr[], u32 size);

// Signed integer types
static void QuickSortKeyDsc (s8 key[], const void* ptr[], u32 size);
static void QuickSortKeyDsc (s16 key[], const void* ptr[], u32 size);
static void QuickSortKeyDsc (s32 key[], const void* ptr[], u32 size);
static void QuickSortKeyDsc (s64 key[], const void* ptr[], u32 size);

// Floating-point types
static void QuickSortKeyDsc (f32 key[], const void* ptr[], u32 size);
static void QuickSortKeyDsc (f64 key[], const void* ptr[], u32 size);

// Other types
static void QuickSortKeyDsc (u32 key[], const void* ptr[], u32 size);

//============================================================================//
//      Object array sorting                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void QuickSortObjAsc (const void* array[], u32 size, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void QuickSortObjDsc (const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Merge sort                                                            //
//****************************************************************************//

//============================================================================//
//      Regular array sorting                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void MergeSortAsc (u8 array[], u8 temp[], u32 size);
static void MergeSortAsc (u16 array[], u16 temp[], u32 size);
static void MergeSortAsc (u32 array[], u32 temp[], u32 size);
static void MergeSortAsc (u64 array[], u64 temp[], u32 size);

// Signed integer types
static void MergeSortAsc (s8 array[], s8 temp[], u32 size);
static void MergeSortAsc (s16 array[], s16 temp[], u32 size);
static void MergeSortAsc (s32 array[], s32 temp[], u32 size);
static void MergeSortAsc (s64 array[], s64 temp[], u32 size);

// Floating-point types
static void MergeSortAsc (f32 array[], f32 temp[], u32 size);
static void MergeSortAsc (f64 array[], f64 temp[], u32 size);

// Other types
static void MergeSortAsc (u32 array[], u32 temp[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void MergeSortDsc (u8 array[], u8 temp[], u32 size);
static void MergeSortDsc (u16 array[], u16 temp[], u32 size);
static void MergeSortDsc (u32 array[], u32 temp[], u32 size);
static void MergeSortDsc (u64 array[], u64 temp[], u32 size);

// Signed integer types
static void MergeSortDsc (s8 array[], s8 temp[], u32 size);
static void MergeSortDsc (s16 array[], s16 temp[], u32 size);
static void MergeSortDsc (s32 array[], s32 temp[], u32 size);
static void MergeSortDsc (s64 array[], s64 temp[], u32 size);

// Floating-point types
static void MergeSortDsc (f32 array[], f32 temp[], u32 size);
static void MergeSortDsc (f64 array[], f64 temp[], u32 size);

// Other types
static void MergeSortDsc (u32 array[], u32 temp[], u32 size);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void MergeSortKeyAsc (u8 key[], const void* ptr[], u8 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyAsc (u16 key[], const void* ptr[], u16 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyAsc (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyAsc (u64 key[], const void* ptr[], u64 tkey[], const void* tptr[], u32 size);

// Signed integer types
static void MergeSortKeyAsc (s8 key[], const void* ptr[], s8 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyAsc (s16 key[], const void* ptr[], s16 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyAsc (s32 key[], const void* ptr[], s32 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyAsc (s64 key[], const void* ptr[], s64 tkey[], const void* tptr[], u32 size);

// Floating-point types
static void MergeSortKeyAsc (f32 key[], const void* ptr[], f32 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyAsc (f64 key[], const void* ptr[], f64 tkey[], const void* tptr[], u32 size);

// Other types
static void MergeSortKeyAsc (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void MergeSortKeyDsc (u8 key[], const void* ptr[], u8 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyDsc (u16 key[], const void* ptr[], u16 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyDsc (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyDsc (u64 key[], const void* ptr[], u64 tkey[], const void* tptr[], u32 size);

// Signed integer types
static void MergeSortKeyDsc (s8 key[], const void* ptr[], s8 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyDsc (s16 key[], const void* ptr[], s16 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyDsc (s32 key[], const void* ptr[], s32 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyDsc (s64 key[], const void* ptr[], s64 tkey[], const void* tptr[], u32 size);

// Floating-point types
static void MergeSortKeyDsc (f32 key[], const void* ptr[], f32 tkey[], const void* tptr[], u32 size);
static void MergeSortKeyDsc (f64 key[], const void* ptr[], f64 tkey[], const void* tptr[], u32 size);

// Other types
static void MergeSortKeyDsc (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);

//============================================================================//
//      Object array sorting                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void MergeSortObjAsc (const void* array[], const void* temp[], u32 size, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void MergeSortObjDsc (const void* array[], const void* temp[], u32 size, Cmp func);

//****************************************************************************//
//      Radix sort                                                            //
//****************************************************************************//

//============================================================================//
//      Regular array sorting                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void RadixSortAsc (u8 array[], u8 temp[], u32 size);
static void RadixSortAsc (u16 array[], u16 temp[], u32 size);
static void RadixSortAsc (u32 array[], u32 temp[], u32 size);
static void RadixSortAsc (u64 array[], u64 temp[], u32 size);

// Signed integer types
static void RadixSortAsc (s8 array[], s8 temp[], u32 size);
static void RadixSortAsc (s16 array[], s16 temp[], u32 size);
static void RadixSortAsc (s32 array[], s32 temp[], u32 size);
static void RadixSortAsc (s64 array[], s64 temp[], u32 size);

// Floating-point types
static void RadixSortAsc (f32 array[], f32 temp[], u32 size);
static void RadixSortAsc (f64 array[], f64 temp[], u32 size);

// Other types
static void RadixSortAsc (u32 array[], u32 temp[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void RadixSortDsc (u8 array[], u8 temp[], u32 size);
static void RadixSortDsc (u16 array[], u16 temp[], u32 size);
static void RadixSortDsc (u32 array[], u32 temp[], u32 size);
static void RadixSortDsc (u64 array[], u64 temp[], u32 size);

// Signed integer types
static void RadixSortDsc (s8 array[], s8 temp[], u32 size);
static void RadixSortDsc (s16 array[], s16 temp[], u32 size);
static void RadixSortDsc (s32 array[], s32 temp[], u32 size);
static void RadixSortDsc (s64 array[], s64 temp[], u32 size);

// Floating-point types
static void RadixSortDsc (f32 array[], f32 temp[], u32 size);
static void RadixSortDsc (f64 array[], f64 temp[], u32 size);

// Other types
static void RadixSortDsc (u32 array[], u32 temp[], u32 size);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void RadixSortKeyAsc (u8 key[], const void* ptr[], u8 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyAsc (u16 key[], const void* ptr[], u16 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyAsc (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyAsc (u64 key[], const void* ptr[], u64 tkey[], const void* tptr[], u32 size);

// Signed integer types
static void RadixSortKeyAsc (s8 key[], const void* ptr[], s8 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyAsc (s16 key[], const void* ptr[], s16 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyAsc (s32 key[], const void* ptr[], s32 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyAsc (s64 key[], const void* ptr[], s64 tkey[], const void* tptr[], u32 size);

// Floating-point types
static void RadixSortKeyAsc (f32 key[], const void* ptr[], f32 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyAsc (f64 key[], const void* ptr[], f64 tkey[], const void* tptr[], u32 size);

// Other types
static void RadixSortKeyAsc (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void RadixSortKeyDsc (u8 key[], const void* ptr[], u8 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyDsc (u16 key[], const void* ptr[], u16 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyDsc (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyDsc (u64 key[], const void* ptr[], u64 tkey[], const void* tptr[], u32 size);

// Signed integer types
static void RadixSortKeyDsc (s8 key[], const void* ptr[], s8 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyDsc (s16 key[], const void* ptr[], s16 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyDsc (s32 key[], const void* ptr[], s32 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyDsc (s64 key[], const void* ptr[], s64 tkey[], const void* tptr[], u32 size);

// Floating-point types
static void RadixSortKeyDsc (f32 key[], const void* ptr[], f32 tkey[], const void* tptr[], u32 size);
static void RadixSortKeyDsc (f64 key[], const void* ptr[], f64 tkey[], const void* tptr[], u32 size);

// Other types
static void RadixSortKeyDsc (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);

//****************************************************************************//
//      Merging of sorted arrays                                              //
//****************************************************************************//

//============================================================================//
//      Regular array merging                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void MergeAsc (u8 target[], const u8 source1[], u32 size1, const u8 source2[], u32 size2);
static void MergeAsc (u16 target[], const u16 source1[], u32 size1, const u16 source2[], u32 size2);
static void MergeAsc (u32 target[], const u32 source1[], u32 size1, const u32 source2[], u32 size2);
static void MergeAsc (u64 target[], const u64 source1[], u32 size1, const u64 source2[], u32 size2);

// Signed integer types
static void MergeAsc (s8 target[], const s8 source1[], u32 size1, const s8 source2[], u32 size2);
static void MergeAsc (s16 target[], const s16 source1[], u32 size1, const s16 source2[], u32 size2);
static void MergeAsc (s32 target[], const s32 source1[], u32 size1, const s32 source2[], u32 size2);
static void MergeAsc (s64 target[], const s64 source1[], u32 size1, const s64 source2[], u32 size2);

// Floating-point types
static void MergeAsc (f32 target[], const f32 source1[], u32 size1, const f32 source2[], u32 size2);
static void MergeAsc (f64 target[], const f64 source1[], u32 size1, const f64 source2[], u32 size2);

// Other types
static void MergeAsc (u32 target[], const u32 source1[], u32 size1, const u32 source2[], u32 size2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void MergeDsc (u8 target[], const u8 source1[], u32 size1, const u8 source2[], u32 size2);
static void MergeDsc (u16 target[], const u16 source1[], u32 size1, const u16 source2[], u32 size2);
static void MergeDsc (u32 target[], const u32 source1[], u32 size1, const u32 source2[], u32 size2);
static void MergeDsc (u64 target[], const u64 source1[], u32 size1, const u64 source2[], u32 size2);

// Signed integer types
static void MergeDsc (s8 target[], const s8 source1[], u32 size1, const s8 source2[], u32 size2);
static void MergeDsc (s16 target[], const s16 source1[], u32 size1, const s16 source2[], u32 size2);
static void MergeDsc (s32 target[], const s32 source1[], u32 size1, const s32 source2[], u32 size2);
static void MergeDsc (s64 target[], const s64 source1[], u32 size1, const s64 source2[], u32 size2);

// Floating-point types
static void MergeDsc (f32 target[], const f32 source1[], u32 size1, const f32 source2[], u32 size2);
static void MergeDsc (f64 target[], const f64 source1[], u32 size1, const f64 source2[], u32 size2);

// Other types
static void MergeDsc (u32 target[], const u32 source1[], u32 size1, const u32 source2[], u32 size2);

//============================================================================//
//      Key array merging                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void MergeKeyAsc (u8 tkey[], const void* tptr[], const u8 skey1[], const void* sptr1[], u32 size1, const u8 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyAsc (u16 tkey[], const void* tptr[], const u16 skey1[], const void* sptr1[], u32 size1, const u16 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyAsc (u32 tkey[], const void* tptr[], const u32 skey1[], const void* sptr1[], u32 size1, const u32 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyAsc (u64 tkey[], const void* tptr[], const u64 skey1[], const void* sptr1[], u32 size1, const u64 skey2[], const void* sptr2[], u32 size2);

// Signed integer types
static void MergeKeyAsc (s8 tkey[], const void* tptr[], const s8 skey1[], const void* sptr1[], u32 size1, const s8 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyAsc (s16 tkey[], const void* tptr[], const s16 skey1[], const void* sptr1[], u32 size1, const s16 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyAsc (s32 tkey[], const void* tptr[], const s32 skey1[], const void* sptr1[], u32 size1, const s32 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyAsc (s64 tkey[], const void* tptr[], const s64 skey1[], const void* sptr1[], u32 size1, const s64 skey2[], const void* sptr2[], u32 size2);

// Floating-point types
static void MergeKeyAsc (f32 tkey[], const void* tptr[], const f32 skey1[], const void* sptr1[], u32 size1, const f32 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyAsc (f64 tkey[], const void* tptr[], const f64 skey1[], const void* sptr1[], u32 size1, const f64 skey2[], const void* sptr2[], u32 size2);

// Other types
static void MergeKeyAsc (u32 tkey[], const void* tptr[], const u32 skey1[], const void* sptr1[], u32 size1, const u32 skey2[], const void* sptr2[], u32 size2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static void MergeKeyDsc (u8 tkey[], const void* tptr[], const u8 skey1[], const void* sptr1[], u32 size1, const u8 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyDsc (u16 tkey[], const void* tptr[], const u16 skey1[], const void* sptr1[], u32 size1, const u16 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyDsc (u32 tkey[], const void* tptr[], const u32 skey1[], const void* sptr1[], u32 size1, const u32 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyDsc (u64 tkey[], const void* tptr[], const u64 skey1[], const void* sptr1[], u32 size1, const u64 skey2[], const void* sptr2[], u32 size2);

// Signed integer types
static void MergeKeyDsc (s8 tkey[], const void* tptr[], const s8 skey1[], const void* sptr1[], u32 size1, const s8 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyDsc (s16 tkey[], const void* tptr[], const s16 skey1[], const void* sptr1[], u32 size1, const s16 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyDsc (s32 tkey[], const void* tptr[], const s32 skey1[], const void* sptr1[], u32 size1, const s32 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyDsc (s64 tkey[], const void* tptr[], const s64 skey1[], const void* sptr1[], u32 size1, const s64 skey2[], const void* sptr2[], u32 size2);

// Floating-point types
static void MergeKeyDsc (f32 tkey[], const void* tptr[], const f32 skey1[], const void* sptr1[], u32 size1, const f32 skey2[], const void* sptr2[], u32 size2);
static void MergeKeyDsc (f64 tkey[], const void* tptr[], const f64 skey1[], const void* sptr1[], u32 size1, const f64 skey2[], const void* sptr2[], u32 size2);

// Other types
static void MergeKeyDsc (u32 tkey[], const void* tptr[], const u32 skey1[], const void* sptr1[], u32 size1, const u32 skey2[], const void* sptr2[], u32 size2);

//============================================================================//
//      Object array merging                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void MergeObjAsc (const void* target[], const void* source1[], u32 size1, const void* source2[], u32 size2, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void MergeObjDsc (const void* target[], const void* source1[], u32 size1, const void* source2[], u32 size2, Cmp func);

//****************************************************************************//
//      Comparison of arrays                                                  //
//****************************************************************************//

//============================================================================//
//      Regular array comparison                                              //
//============================================================================//

// Unsigned integer types
static s64 Compare (const u8 array1[], const u8 array2[], u32 size);
static s64 Compare (const u16 array1[], const u16 array2[], u32 size);
static s64 Compare (const u32 array1[], const u32 array2[], u32 size);
static s64 Compare (const u64 array1[], const u64 array2[], u32 size);

// Signed integer types
static s64 Compare (const s8 array1[], const s8 array2[], u32 size);
static s64 Compare (const s16 array1[], const s16 array2[], u32 size);
static s64 Compare (const s32 array1[], const s32 array2[], u32 size);
static s64 Compare (const s64 array1[], const s64 array2[], u32 size);

// Floating-point types
static s64 Compare (const f32 array1[], const f32 array2[], u32 size);
static s64 Compare (const f64 array1[], const f64 array2[], u32 size);

// Other types
static s64 Compare (const u32 array1[], const u32 array2[], u32 size);
static s64 Compare (const void *array1, const void *array2, u32 size);

//============================================================================//
//      Object array comparison                                               //
//============================================================================//
static s64 CompareObj (const void* array1[], const void* array2[], u32 size, Cmp func);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

//============================================================================//
//      Check for differences                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Regular array check                                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u32 CheckDiff (const u8 array1[], const u8 array2[], u32 size);
static u32 CheckDiff (const u16 array1[], const u16 array2[], u32 size);
static u32 CheckDiff (const u32 array1[], const u32 array2[], u32 size);
static u32 CheckDiff (const u64 array1[], const u64 array2[], u32 size);

// Signed integer types
static u32 CheckDiff (const s8 array1[], const s8 array2[], u32 size);
static u32 CheckDiff (const s16 array1[], const s16 array2[], u32 size);
static u32 CheckDiff (const s32 array1[], const s32 array2[], u32 size);
static u32 CheckDiff (const s64 array1[], const s64 array2[], u32 size);

// Floating-point types
static u32 CheckDiff (const f32 array1[], const f32 array2[], u32 size);
static u32 CheckDiff (const f64 array1[], const f64 array2[], u32 size);

// Other types
static u32 CheckDiff (const u32 array1[], const u32 array2[], u32 size);
static u32 CheckDiff (const void *array1, const void *array2, u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Object array check                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u32 CheckDiffObj (const void* array1[], const void* array2[], u32 size, Cmp func);

//============================================================================//
//      Check for duplicate values                                            //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Regular array check                                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
static u32 CheckDup (const u8 array[], u32 size);
static u32 CheckDup (const u16 array[], u32 size);
static u32 CheckDup (const u32 array[], u32 size);
static u32 CheckDup (const u64 array[], u32 size);

// Signed integer types
static u32 CheckDup (const s8 array[], u32 size);
static u32 CheckDup (const s16 array[], u32 size);
static u32 CheckDup (const s32 array[], u32 size);
static u32 CheckDup (const s64 array[], u32 size);

// Floating-point types
static u32 CheckDup (const f32 array[], u32 size);
static u32 CheckDup (const f64 array[], u32 size);

// Other types
static u32 CheckDup (const u32 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Object array check                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u32 CheckDupObj (const void* array[], u32 size, Cmp func);

//============================================================================//
//      Check for sort order                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Regular array check                                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Check for ascending sort order                                        //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 CheckSortAsc (const u8 array[], u32 size);
static u32 CheckSortAsc (const u16 array[], u32 size);
static u32 CheckSortAsc (const u32 array[], u32 size);
static u32 CheckSortAsc (const u64 array[], u32 size);

// Signed integer types
static u32 CheckSortAsc (const s8 array[], u32 size);
static u32 CheckSortAsc (const s16 array[], u32 size);
static u32 CheckSortAsc (const s32 array[], u32 size);
static u32 CheckSortAsc (const s64 array[], u32 size);

// Floating-point types
static u32 CheckSortAsc (const f32 array[], u32 size);
static u32 CheckSortAsc (const f64 array[], u32 size);

// Other types
static u32 CheckSortAsc (const u32 array[], u32 size);

//----------------------------------------------------------------------------//
//      Check for descending sort order                                       //
//----------------------------------------------------------------------------//

// Unsigned integer types
static u32 CheckSortDsc (const u8 array[], u32 size);
static u32 CheckSortDsc (const u16 array[], u32 size);
static u32 CheckSortDsc (const u32 array[], u32 size);
static u32 CheckSortDsc (const u64 array[], u32 size);

// Signed integer types
static u32 CheckSortDsc (const s8 array[], u32 size);
static u32 CheckSortDsc (const s16 array[], u32 size);
static u32 CheckSortDsc (const s32 array[], u32 size);
static u32 CheckSortDsc (const s64 array[], u32 size);

// Floating-point types
static u32 CheckSortDsc (const f32 array[], u32 size);
static u32 CheckSortDsc (const f64 array[], u32 size);

// Other types
static u32 CheckSortDsc (const u32 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Object array check                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Check for ascending sort order                                        //
//----------------------------------------------------------------------------//
static u32 CheckSortObjAsc (const void* array[], u32 size, Cmp func);

//----------------------------------------------------------------------------//
//      Check for descending sort order                                       //
//----------------------------------------------------------------------------//
static u32 CheckSortObjDsc (const void* array[], u32 size, Cmp func);

//============================================================================//
//      Check for infinite values                                             //
//============================================================================//
static u32 CheckInf (const f32 array[], u32 size);
static u32 CheckInf (const f64 array[], u32 size);

//============================================================================//
//      Check for NaN values                                                  //
//============================================================================//
static u32 CheckNaN (const f32 array[], u32 size);
static u32 CheckNaN (const f64 array[], u32 size);

//============================================================================//
//      Check for overlap                                                     //
//============================================================================//

// Unsigned integer types
static bool Overlap (const u8 array1[], u32 size1, const u8 array2[], u32 size2);
static bool Overlap (const u16 array1[], u32 size1, const u16 array2[], u32 size2);
static bool Overlap (const u32 array1[], u32 size1, const u32 array2[], u32 size2);
static bool Overlap (const u64 array1[], u32 size1, const u64 array2[], u32 size2);

// Signed integer types
static bool Overlap (const s8 array1[], u32 size1, const s8 array2[], u32 size2);
static bool Overlap (const s16 array1[], u32 size1, const s16 array2[], u32 size2);
static bool Overlap (const s32 array1[], u32 size1, const s32 array2[], u32 size2);
static bool Overlap (const s64 array1[], u32 size1, const s64 array2[], u32 size2);

// Floating-point types
static bool Overlap (const f32 array1[], u32 size1, const f32 array2[], u32 size2);
static bool Overlap (const f64 array1[], u32 size1, const f64 array2[], u32 size2);

// Other types
static bool Overlap (const u32 array1[], u32 size1, const u32 array2[], u32 size2);
static bool Overlap (const void *array1, u32 size1, const void *array2, u32 size2);

//****************************************************************************//
//      Array hashing                                                         //
//****************************************************************************//

//============================================================================//
//      32-bit hash functions                                                 //
//============================================================================//

// Unsigned integer types
static u32 Hash32 (const u8 array[], u32 size);
static u32 Hash32 (const u16 array[], u32 size);
static u32 Hash32 (const u32 array[], u32 size);
static u32 Hash32 (const u64 array[], u32 size);

// Unsigned integer types
static u32 Hash32 (const s8 array[], u32 size);
static u32 Hash32 (const s16 array[], u32 size);
static u32 Hash32 (const s32 array[], u32 size);
static u32 Hash32 (const s64 array[], u32 size);

// Floating-point types
static u32 Hash32 (const f32 array[], u32 size);
static u32 Hash32 (const f64 array[], u32 size);

// Other types
static u32 Hash32 (const u32 array[], u32 size);
static u32 Hash32 (const void *array, u32 size);

//============================================================================//
//      64-bit hash functions                                                 //
//============================================================================//

// Unsigned integer types
static u64 Hash64 (const u8 array[], u32 size);
static u64 Hash64 (const u16 array[], u32 size);
static u64 Hash64 (const u32 array[], u32 size);
static u64 Hash64 (const u64 array[], u32 size);

// Unsigned integer types
static u64 Hash64 (const s8 array[], u32 size);
static u64 Hash64 (const s16 array[], u32 size);
static u64 Hash64 (const s32 array[], u32 size);
static u64 Hash64 (const s64 array[], u32 size);

// Floating-point types
static u64 Hash64 (const f32 array[], u32 size);
static u64 Hash64 (const f64 array[], u32 size);

// Other types
static u64 Hash64 (const u32 array[], u32 size);
static u64 Hash64 (const void *array, u32 size);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Initialization                                                        //
//****************************************************************************//

// Unsigned integer types
void Array_Init_uint8 (u8 array[], u32 size, u8 value);
void Array_Init_uint16 (u16 array[], u32 size, u16 value);
void Array_Init_uint32 (u32 array[], u32 size, u32 value);
void Array_Init_uint64 (u64 array[], u32 size, u64 value);

// Signed integer types
void Array_Init_sint8 (s8 array[], u32 size, s8 value);
void Array_Init_sint16 (s16 array[], u32 size, s16 value);
void Array_Init_sint32 (s32 array[], u32 size, s32 value);
void Array_Init_sint64 (s64 array[], u32 size, s64 value);

// Floating-point types
void Array_Init_flt32 (f32 array[], u32 size, f32 value);
void Array_Init_flt64 (f64 array[], u32 size, f64 value);

// Other types
void Array_Init_size (u32 array[], u32 size, u32 value);

//****************************************************************************//
//      Copying arrays                                                        //
//****************************************************************************//

// Unsigned integer types
void Array_Copy_uint8 (u8 target[], const u8 source[], u32 size);
void Array_Copy_uint16 (u16 target[], const u16 source[], u32 size);
void Array_Copy_uint32 (u32 target[], const u32 source[], u32 size);
void Array_Copy_uint64 (u64 target[], const u64 source[], u32 size);

// Signed integer types
void Array_Copy_sint8 (s8 target[], const s8 source[], u32 size);
void Array_Copy_sint16 (s16 target[], const s16 source[], u32 size);
void Array_Copy_sint32 (s32 target[], const s32 source[], u32 size);
void Array_Copy_sint64 (s64 target[], const s64 source[], u32 size);

// Floating-point types
void Array_Copy_flt32 (f32 target[], const f32 source[], u32 size);
void Array_Copy_flt64 (f64 target[], const f64 source[], u32 size);

// Other types
void Array_Copy_size (u32 target[], const u32 source[], u32 size);
void Array_Copy (void *target, const void *source, u32 size);

//****************************************************************************//
//      Moving arrays                                                         //
//****************************************************************************//

// Unsigned integer types
void Array_Move_uint8 (u8 target[], u8 source[], u32 size);
void Array_Move_uint16 (u16 target[], u16 source[], u32 size);
void Array_Move_uint32 (u32 target[], u32 source[], u32 size);
void Array_Move_uint64 (u64 target[], u64 source[], u32 size);

// Signed integer types
void Array_Move_sint8 (s8 target[], s8 source[], u32 size);
void Array_Move_sint16 (s16 target[], s16 source[], u32 size);
void Array_Move_sint32 (s32 target[], s32 source[], u32 size);
void Array_Move_sint64 (s64 target[], s64 source[], u32 size);

// Floating-point types
void Array_Move_flt32 (f32 target[], f32 source[], u32 size);
void Array_Move_flt64 (f64 target[], f64 source[], u32 size);

// Other types
void Array_Move_size (u32 target[], u32 source[], u32 size);
void Array_Move (void *target, void *source, u32 size);

//****************************************************************************//
//      Pattern cloning                                                       //
//****************************************************************************//

// Unsigned integer types
void Array_Clone_uint8 (u8 array[], u32 size, u32 psize);
void Array_Clone_uint16 (u16 array[], u32 size, u32 psize);
void Array_Clone_uint32 (u32 array[], u32 size, u32 psize);
void Array_Clone_uint64 (u64 array[], u32 size, u32 psize);

// Signed integer types
void Array_Clone_sint8 (s8 array[], u32 size, u32 psize);
void Array_Clone_sint16 (s16 array[], u32 size, u32 psize);
void Array_Clone_sint32 (s32 array[], u32 size, u32 psize);
void Array_Clone_sint64 (s64 array[], u32 size, u32 psize);

// Floating-point types
void Array_Clone_flt32 (f32 array[], u32 size, u32 psize);
void Array_Clone_flt64 (f64 array[], u32 size, u32 psize);

// Other types
void Array_Clone_size (u32 array[], u32 size, u32 psize);
void Array_Clone (void *array, u32 size, u32 psize);

//****************************************************************************//
//      Data conversion                                                       //
//****************************************************************************//

// Conversion between floating-point types
void Array_ConvertToFlt32_flt64 (f32 target[], const f64 source[], u32 size);
void Array_ConvertToFlt64_flt32 (f64 target[], const f32 source[], u32 size);

// Conversion from signed integer types to floating-point types
void Array_ConvertToFlt32_sint32 (f32 target[], const s32 source[], u32 size);
void Array_ConvertToFlt32_sint64 (f32 target[], const s64 source[], u32 size);
void Array_ConvertToFlt64_sint32 (f64 target[], const s32 source[], u32 size);
void Array_ConvertToFlt64_sint64 (f64 target[], const s64 source[], u32 size);

// Conversion from floating-point types to signed integer types
void Array_ConvertToSint32_flt32 (s32 target[], const f32 source[], u32 size);
void Array_ConvertToSint32_flt64 (s32 target[], const f64 source[], u32 size);
void Array_ConvertToSint64_flt32 (s64 target[], const f32 source[], u32 size);
void Array_ConvertToSint64_flt64 (s64 target[], const f64 source[], u32 size);

// Truncating from floating-point types to signed integer types
void Array_TruncateToSint32_flt32 (s32 target[], const f32 source[], u32 size);
void Array_TruncateToSint32_flt64 (s32 target[], const f64 source[], u32 size);
void Array_TruncateToSint64_flt32 (s64 target[], const f32 source[], u32 size);
void Array_TruncateToSint64_flt64 (s64 target[], const f64 source[], u32 size);

//****************************************************************************//
//      Bit field operations                                                  //
//****************************************************************************//

//============================================================================//
//      Get bit value from bit field                                          //
//============================================================================//

// Unsigned integer types
bool Array_GetBit_uint8 (const u8 array[], u32 index);
bool Array_GetBit_uint16 (const u16 array[], u32 index);
bool Array_GetBit_uint32 (const u32 array[], u32 index);
bool Array_GetBit_uint64 (const u64 array[], u32 index);

// Signed integer types
bool Array_GetBit_sint8 (const s8 array[], u32 index);
bool Array_GetBit_sint16 (const s16 array[], u32 index);
bool Array_GetBit_sint32 (const s32 array[], u32 index);
bool Array_GetBit_sint64 (const s64 array[], u32 index);

// Other types
bool Array_GetBit_size (const u32 array[], u32 index);
bool Array_GetBit (const void *array, u32 index);

//============================================================================//
//      Set bit value in bit field                                            //
//============================================================================//

// Unsigned integer types
void Array_SetBit_uint8 (u8 array[], u32 index);
void Array_SetBit_uint16 (u16 array[], u32 index);
void Array_SetBit_uint32 (u32 array[], u32 index);
void Array_SetBit_uint64 (u64 array[], u32 index);

// Signed integer types
void Array_SetBit_sint8 (s8 array[], u32 index);
void Array_SetBit_sint16 (s16 array[], u32 index);
void Array_SetBit_sint32 (s32 array[], u32 index);
void Array_SetBit_sint64 (s64 array[], u32 index);

// Other types
void Array_SetBit_size (u32 array[], u32 index);
void Array_SetBit (void *array, u32 index);

//============================================================================//
//      Reset bit value in bit field                                          //
//============================================================================//

// Unsigned integer types
void Array_ResetBit_uint8 (u8 array[], u32 index);
void Array_ResetBit_uint16 (u16 array[], u32 index);
void Array_ResetBit_uint32 (u32 array[], u32 index);
void Array_ResetBit_uint64 (u64 array[], u32 index);

// Signed integer types
void Array_ResetBit_sint8 (s8 array[], u32 index);
void Array_ResetBit_sint16 (s16 array[], u32 index);
void Array_ResetBit_sint32 (s32 array[], u32 index);
void Array_ResetBit_sint64 (s64 array[], u32 index);

// Other types
void Array_ResetBit_size (u32 array[], u32 index);
void Array_ResetBit (void *array, u32 index);

//============================================================================//
//      Invert bit value in bit field                                         //
//============================================================================//

// Unsigned integer types
void Array_InvertBit_uint8 (u8 array[], u32 index);
void Array_InvertBit_uint16 (u16 array[], u32 index);
void Array_InvertBit_uint32 (u32 array[], u32 index);
void Array_InvertBit_uint64 (u64 array[], u32 index);

// Signed integer types
void Array_InvertBit_sint8 (s8 array[], u32 index);
void Array_InvertBit_sint16 (s16 array[], u32 index);
void Array_InvertBit_sint32 (s32 array[], u32 index);
void Array_InvertBit_sint64 (s64 array[], u32 index);

// Other types
void Array_InvertBit_size (u32 array[], u32 index);
void Array_InvertBit (void *array, u32 index);

//****************************************************************************//
//      Bitwise operations                                                    //
//****************************************************************************//

//============================================================================//
//      Byte swap                                                             //
//============================================================================//

// Unsigned integer types
void Array_ByteSwap_uint8 (u8 array[], u32 size);
void Array_ByteSwap_uint16 (u16 array[], u32 size);
void Array_ByteSwap_uint32 (u32 array[], u32 size);
void Array_ByteSwap_uint64 (u64 array[], u32 size);

// Signed integer types
void Array_ByteSwap_sint8 (s8 array[], u32 size);
void Array_ByteSwap_sint16 (s16 array[], u32 size);
void Array_ByteSwap_sint32 (s32 array[], u32 size);
void Array_ByteSwap_sint64 (s64 array[], u32 size);

// Other types
void Array_ByteSwap_size (u32 array[], u32 size);

//============================================================================//
//      Bit reversal permutation                                              //
//============================================================================//

// Unsigned integer types
void Array_BitReverse_uint8 (u8 array[], u32 size);
void Array_BitReverse_uint16 (u16 array[], u32 size);
void Array_BitReverse_uint32 (u32 array[], u32 size);
void Array_BitReverse_uint64 (u64 array[], u32 size);

// Signed integer types
void Array_BitReverse_sint8 (s8 array[], u32 size);
void Array_BitReverse_sint16 (s16 array[], u32 size);
void Array_BitReverse_sint32 (s32 array[], u32 size);
void Array_BitReverse_sint64 (s64 array[], u32 size);

// Other types
void Array_BitReverse_size (u32 array[], u32 size);

//============================================================================//
//      Population count                                                      //
//============================================================================//

// Unsigned integer types
void Array_PopCount_uint8 (u8 array[], u32 size);
void Array_PopCount_uint16 (u16 array[], u32 size);
void Array_PopCount_uint32 (u32 array[], u32 size);
void Array_PopCount_uint64 (u64 array[], u32 size);

// Signed integer types
void Array_PopCount_sint8 (s8 array[], u32 size);
void Array_PopCount_sint16 (s16 array[], u32 size);
void Array_PopCount_sint32 (s32 array[], u32 size);
void Array_PopCount_sint64 (s64 array[], u32 size);

// Other types
void Array_PopCount_size (u32 array[], u32 size);

//============================================================================//
//      Bitwise NOT                                                           //
//============================================================================//

// Unsigned integer types
void Array_Not_uint8 (u8 array[], u32 size);
void Array_Not_uint16 (u16 array[], u32 size);
void Array_Not_uint32 (u32 array[], u32 size);
void Array_Not_uint64 (u64 array[], u32 size);

// Signed integer types
void Array_Not_sint8 (s8 array[], u32 size);
void Array_Not_sint16 (s16 array[], u32 size);
void Array_Not_sint32 (s32 array[], u32 size);
void Array_Not_sint64 (s64 array[], u32 size);

// Other types
void Array_Not_size (u32 array[], u32 size);

//============================================================================//
//      Bitwise AND                                                           //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Scalar bitwise AND                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_AndScalar_uint8 (u8 array[], u32 size, u8 value);
void Array_AndScalar_uint16 (u16 array[], u32 size, u16 value);
void Array_AndScalar_uint32 (u32 array[], u32 size, u32 value);
void Array_AndScalar_uint64 (u64 array[], u32 size, u64 value);

// Signed integer types
void Array_AndScalar_sint8 (s8 array[], u32 size, s8 value);
void Array_AndScalar_sint16 (s16 array[], u32 size, s16 value);
void Array_AndScalar_sint32 (s32 array[], u32 size, s32 value);
void Array_AndScalar_sint64 (s64 array[], u32 size, s64 value);

// Other types
void Array_AndScalar_size (u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Vector bitwise AND                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_AndVector_uint8 (u8 target[], const u8 source[], u32 size);
void Array_AndVector_uint16 (u16 target[], const u16 source[], u32 size);
void Array_AndVector_uint32 (u32 target[], const u32 source[], u32 size);
void Array_AndVector_uint64 (u64 target[], const u64 source[], u32 size);

// Signed integer types
void Array_AndVector_sint8 (s8 target[], const s8 source[], u32 size);
void Array_AndVector_sint16 (s16 target[], const s16 source[], u32 size);
void Array_AndVector_sint32 (s32 target[], const s32 source[], u32 size);
void Array_AndVector_sint64 (s64 target[], const s64 source[], u32 size);

// Other types
void Array_AndVector_size (u32 target[], const u32 source[], u32 size);

//============================================================================//
//      Bitwise OR                                                            //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Scalar bitwise OR                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_OrScalar_uint8 (u8 array[], u32 size, u8 value);
void Array_OrScalar_uint16 (u16 array[], u32 size, u16 value);
void Array_OrScalar_uint32 (u32 array[], u32 size, u32 value);
void Array_OrScalar_uint64 (u64 array[], u32 size, u64 value);

// Signed integer types
void Array_OrScalar_sint8 (s8 array[], u32 size, s8 value);
void Array_OrScalar_sint16 (s16 array[], u32 size, s16 value);
void Array_OrScalar_sint32 (s32 array[], u32 size, s32 value);
void Array_OrScalar_sint64 (s64 array[], u32 size, s64 value);

// Other types
void Array_OrScalar_size (u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Vector bitwise OR                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_OrVector_uint8 (u8 target[], const u8 source[], u32 size);
void Array_OrVector_uint16 (u16 target[], const u16 source[], u32 size);
void Array_OrVector_uint32 (u32 target[], const u32 source[], u32 size);
void Array_OrVector_uint64 (u64 target[], const u64 source[], u32 size);

// Signed integer types
void Array_OrVector_sint8 (s8 target[], const s8 source[], u32 size);
void Array_OrVector_sint16 (s16 target[], const s16 source[], u32 size);
void Array_OrVector_sint32 (s32 target[], const s32 source[], u32 size);
void Array_OrVector_sint64 (s64 target[], const s64 source[], u32 size);

// Other types
void Array_OrVector_size (u32 target[], const u32 source[], u32 size);

//============================================================================//
//      Bitwise XOR                                                           //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Scalar bitwise XOR                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_XorScalar_uint8 (u8 array[], u32 size, u8 value);
void Array_XorScalar_uint16 (u16 array[], u32 size, u16 value);
void Array_XorScalar_uint32 (u32 array[], u32 size, u32 value);
void Array_XorScalar_uint64 (u64 array[], u32 size, u64 value);

// Signed integer types
void Array_XorScalar_sint8 (s8 array[], u32 size, s8 value);
void Array_XorScalar_sint16 (s16 array[], u32 size, s16 value);
void Array_XorScalar_sint32 (s32 array[], u32 size, s32 value);
void Array_XorScalar_sint64 (s64 array[], u32 size, s64 value);

// Other types
void Array_XorScalar_size (u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Vector bitwise XOR                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_XorVector_uint8 (u8 target[], const u8 source[], u32 size);
void Array_XorVector_uint16 (u16 target[], const u16 source[], u32 size);
void Array_XorVector_uint32 (u32 target[], const u32 source[], u32 size);
void Array_XorVector_uint64 (u64 target[], const u64 source[], u32 size);

// Signed integer types
void Array_XorVector_sint8 (s8 target[], const s8 source[], u32 size);
void Array_XorVector_sint16 (s16 target[], const s16 source[], u32 size);
void Array_XorVector_sint32 (s32 target[], const s32 source[], u32 size);
void Array_XorVector_sint64 (s64 target[], const s64 source[], u32 size);

// Other types
void Array_XorVector_size (u32 target[], const u32 source[], u32 size);

//****************************************************************************//
//      Arithmetic operations                                                 //
//****************************************************************************//

//============================================================================//
//      Unary operations                                                      //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Negative value                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
void Array_Neg_sint8 (s8 array[], u32 size);
void Array_Neg_sint16 (s16 array[], u32 size);
void Array_Neg_sint32 (s32 array[], u32 size);
void Array_Neg_sint64 (s64 array[], u32 size);

// Floating-point types
void Array_Neg_flt32 (f32 array[], u32 size);
void Array_Neg_flt64 (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Absolute value                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
void Array_Abs_sint8 (s8 array[], u32 size);
void Array_Abs_sint16 (s16 array[], u32 size);
void Array_Abs_sint32 (s32 array[], u32 size);
void Array_Abs_sint64 (s64 array[], u32 size);

// Floating-point types
void Array_Abs_flt32 (f32 array[], u32 size);
void Array_Abs_flt64 (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Negative absolute value                                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
void Array_NegAbs_sint8 (s8 array[], u32 size);
void Array_NegAbs_sint16 (s16 array[], u32 size);
void Array_NegAbs_sint32 (s32 array[], u32 size);
void Array_NegAbs_sint64 (s64 array[], u32 size);

// Floating-point types
void Array_NegAbs_flt32 (f32 array[], u32 size);
void Array_NegAbs_flt64 (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Number sign                                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Signed integer types
void Array_Sign_sint8 (s8 array[], u32 size);
void Array_Sign_sint16 (s16 array[], u32 size);
void Array_Sign_sint32 (s32 array[], u32 size);
void Array_Sign_sint64 (s64 array[], u32 size);

// Floating-point types
void Array_Sign_flt32 (f32 array[], u32 size);
void Array_Sign_flt64 (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Square                                                                //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_Sqr_flt32 (f32 array[], u32 size);
void Array_Sqr_flt64 (f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Square root                                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_Sqrt_flt32 (f32 array[], u32 size);
void Array_Sqrt_flt64 (f64 array[], u32 size);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Addition                                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar addition                                                       //
//----------------------------------------------------------------------------//

// Unsigned integer types
void Array_AddScalar_uint8 (u8 array[], u32 size, u8 value);
void Array_AddScalar_uint16 (u16 array[], u32 size, u16 value);
void Array_AddScalar_uint32 (u32 array[], u32 size, u32 value);
void Array_AddScalar_uint64 (u64 array[], u32 size, u64 value);

// Signed integer types
void Array_AddScalar_sint8 (s8 array[], u32 size, s8 value);
void Array_AddScalar_sint16 (s16 array[], u32 size, s16 value);
void Array_AddScalar_sint32 (s32 array[], u32 size, s32 value);
void Array_AddScalar_sint64 (s64 array[], u32 size, s64 value);

// Floating-point types
void Array_AddScalar_flt32 (f32 array[], u32 size, f32 value);
void Array_AddScalar_flt64 (f64 array[], u32 size, f64 value);

// Other types
void Array_AddScalar_size (u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Vector addition                                                       //
//----------------------------------------------------------------------------//

// Unsigned integer types
void Array_AddVector_uint8 (u8 target[], const u8 source[], u32 size);
void Array_AddVector_uint16 (u16 target[], const u16 source[], u32 size);
void Array_AddVector_uint32 (u32 target[], const u32 source[], u32 size);
void Array_AddVector_uint64 (u64 target[], const u64 source[], u32 size);

// Signed integer types
void Array_AddVector_sint8 (s8 target[], const s8 source[], u32 size);
void Array_AddVector_sint16 (s16 target[], const s16 source[], u32 size);
void Array_AddVector_sint32 (s32 target[], const s32 source[], u32 size);
void Array_AddVector_sint64 (s64 target[], const s64 source[], u32 size);

// Floating-point types
void Array_AddVector_flt32 (f32 target[], const f32 source[], u32 size);
void Array_AddVector_flt64 (f64 target[], const f64 source[], u32 size);

// Other types
void Array_AddVector_size (u32 target[], const u32 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Subtraction                                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar subtraction                                                    //
//----------------------------------------------------------------------------//

// Unsigned integer types
void Array_SubScalar_uint8 (u8 array[], u32 size, u8 value);
void Array_SubScalar_uint16 (u16 array[], u32 size, u16 value);
void Array_SubScalar_uint32 (u32 array[], u32 size, u32 value);
void Array_SubScalar_uint64 (u64 array[], u32 size, u64 value);

// Signed integer types
void Array_SubScalar_sint8 (s8 array[], u32 size, s8 value);
void Array_SubScalar_sint16 (s16 array[], u32 size, s16 value);
void Array_SubScalar_sint32 (s32 array[], u32 size, s32 value);
void Array_SubScalar_sint64 (s64 array[], u32 size, s64 value);

// Floating-point types
void Array_SubScalar_flt32 (f32 array[], u32 size, f32 value);
void Array_SubScalar_flt64 (f64 array[], u32 size, f64 value);

// Other types
void Array_SubScalar_size (u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Vector subtraction                                                    //
//----------------------------------------------------------------------------//

// Unsigned integer types
void Array_SubVector_uint8 (u8 target[], const u8 source[], u32 size);
void Array_SubVector_uint16 (u16 target[], const u16 source[], u32 size);
void Array_SubVector_uint32 (u32 target[], const u32 source[], u32 size);
void Array_SubVector_uint64 (u64 target[], const u64 source[], u32 size);

// Signed integer types
void Array_SubVector_sint8 (s8 target[], const s8 source[], u32 size);
void Array_SubVector_sint16 (s16 target[], const s16 source[], u32 size);
void Array_SubVector_sint32 (s32 target[], const s32 source[], u32 size);
void Array_SubVector_sint64 (s64 target[], const s64 source[], u32 size);

// Floating-point types
void Array_SubVector_flt32 (f32 target[], const f32 source[], u32 size);
void Array_SubVector_flt64 (f64 target[], const f64 source[], u32 size);

// Other types
void Array_SubVector_size (u32 target[], const u32 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Reverse subtraction                                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar reverse subtraction                                            //
//----------------------------------------------------------------------------//

// Unsigned integer types
void Array_ReverseSubScalar_uint8 (u8 array[], u32 size, u8 value);
void Array_ReverseSubScalar_uint16 (u16 array[], u32 size, u16 value);
void Array_ReverseSubScalar_uint32 (u32 array[], u32 size, u32 value);
void Array_ReverseSubScalar_uint64 (u64 array[], u32 size, u64 value);

// Signed integer types
void Array_ReverseSubScalar_sint8 (s8 array[], u32 size, s8 value);
void Array_ReverseSubScalar_sint16 (s16 array[], u32 size, s16 value);
void Array_ReverseSubScalar_sint32 (s32 array[], u32 size, s32 value);
void Array_ReverseSubScalar_sint64 (s64 array[], u32 size, s64 value);

// Floating-point types
void Array_ReverseSubScalar_flt32 (f32 array[], u32 size, f32 value);
void Array_ReverseSubScalar_flt64 (f64 array[], u32 size, f64 value);

// Other types
void Array_ReverseSubScalar_size (u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Vector reverse subtraction                                            //
//----------------------------------------------------------------------------//

// Unsigned integer types
void Array_ReverseSubVector_uint8 (u8 target[], const u8 source[], u32 size);
void Array_ReverseSubVector_uint16 (u16 target[], const u16 source[], u32 size);
void Array_ReverseSubVector_uint32 (u32 target[], const u32 source[], u32 size);
void Array_ReverseSubVector_uint64 (u64 target[], const u64 source[], u32 size);

// Signed integer types
void Array_ReverseSubVector_sint8 (s8 target[], const s8 source[], u32 size);
void Array_ReverseSubVector_sint16 (s16 target[], const s16 source[], u32 size);
void Array_ReverseSubVector_sint32 (s32 target[], const s32 source[], u32 size);
void Array_ReverseSubVector_sint64 (s64 target[], const s64 source[], u32 size);

// Floating-point types
void Array_ReverseSubVector_flt32 (f32 target[], const f32 source[], u32 size);
void Array_ReverseSubVector_flt64 (f64 target[], const f64 source[], u32 size);

// Other types
void Array_ReverseSubVector_size (u32 target[], const u32 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Multiplication                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar multiplication                                                 //
//----------------------------------------------------------------------------//
void Array_MulScalar_flt32 (f32 array[], u32 size, f32 value);
void Array_MulScalar_flt64 (f64 array[], u32 size, f64 value);

//----------------------------------------------------------------------------//
//      Vector multiplication                                                 //
//----------------------------------------------------------------------------//
void Array_MulVector_flt32 (f32 target[], const f32 source[], u32 size);
void Array_MulVector_flt64 (f64 target[], const f64 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Division                                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar division                                                       //
//----------------------------------------------------------------------------//
void Array_DivScalar_flt32 (f32 array[], u32 size, f32 value);
void Array_DivScalar_flt64 (f64 array[], u32 size, f64 value);

//----------------------------------------------------------------------------//
//      Vector division                                                       //
//----------------------------------------------------------------------------//
void Array_DivVector_flt32 (f32 target[], const f32 source[], u32 size);
void Array_DivVector_flt64 (f64 target[], const f64 source[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Reverse division                                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Scalar reverse division                                               //
//----------------------------------------------------------------------------//
void Array_ReverseDivScalar_flt32 (f32 array[], u32 size, f32 value);
void Array_ReverseDivScalar_flt64 (f64 array[], u32 size, f64 value);

//----------------------------------------------------------------------------//
//      Vector reverse division                                               //
//----------------------------------------------------------------------------//
void Array_ReverseDivVector_flt32 (f32 target[], const f32 source[], u32 size);
void Array_ReverseDivVector_flt64 (f64 target[], const f64 source[], u32 size);

//****************************************************************************//
//      Rounding                                                              //
//****************************************************************************//

// Round down (floor)
void Array_RoundDown_flt32 (f32 array[], u32 size);
void Array_RoundDown_flt64 (f64 array[], u32 size);

// Round up (ceil)
void Array_RoundUp_flt32 (f32 array[], u32 size);
void Array_RoundUp_flt64 (f64 array[], u32 size);

// Round to nearest even integer
void Array_RoundInt_flt32 (f32 array[], u32 size);
void Array_RoundInt_flt64 (f64 array[], u32 size);

// Round to nearest integer, away from zero
void Array_Round_flt32 (f32 array[], u32 size);
void Array_Round_flt64 (f64 array[], u32 size);

// Round to nearest integer, toward zero (truncation)
void Array_Truncate_flt32 (f32 array[], u32 size);
void Array_Truncate_flt64 (f64 array[], u32 size);

// Fractional part
void Array_Frac_flt32 (f32 array[], u32 size);
void Array_Frac_flt64 (f64 array[], u32 size);

//****************************************************************************//
//      Numerical integration                                                 //
//****************************************************************************//

// Sum of elements
f32 Array_Sum_flt32 (const f32 array[], u32 size);
f64 Array_Sum_flt64 (const f64 array[], u32 size);

// Sum of squares
f32 Array_SumSqr_flt32 (const f32 array[], u32 size);
f64 Array_SumSqr_flt64 (const f64 array[], u32 size);

// Sum of absolute values
f32 Array_SumAbs_flt32 (const f32 array[], u32 size);
f64 Array_SumAbs_flt32 (const f64 array[], u32 size);

// Sum of multiplied values
f32 Array_SumMul_flt32 (const f32 array1[], const f32 array2[], u32 size);
f64 Array_SumMul_flt64 (const f64 array1[], const f64 array2[], u32 size);

// Sum of squared differences
f32 Array_SumSqrDiff_flt32 (const f32 array1[], const f32 array2[], u32 size);
f64 Array_SumSqrDiff_flt64 (const f64 array1[], const f64 array2[], u32 size);

// Sum of absolute differences
f32 Array_SumAbsDiff_flt32 (const f32 array1[], const f32 array2[], u32 size);
f64 Array_SumAbsDiff_flt64 (const f64 array1[], const f64 array2[], u32 size);

//****************************************************************************//
//      Minimum and maximum absolute value                                    //
//****************************************************************************//

//============================================================================//
//      Minimum absolute value                                                //
//============================================================================//

// Signed integer types
u8 Array_MinAbs_sint8 (const s8 array[], u32 size);
u16 Array_MinAbs_sint16 (const s16 array[], u32 size);
u32 Array_MinAbs_sint32 (const s32 array[], u32 size);

// Floating-point types
f32 Array_MinAbs_flt32 (const f32 array[], u32 size);
f64 Array_MinAbs_flt64 (const f64 array[], u32 size);

//============================================================================//
//      Maximum absolute value                                                //
//============================================================================//

// Signed integer types
u8 Array_MaxAbs_sint8 (const s8 array[], u32 size);
u16 Array_MaxAbs_sint16 (const s16 array[], u32 size);
u32 Array_MaxAbs_sint32 (const s32 array[], u32 size);

// Floating-point types
f32 Array_MaxAbs_flt32 (const f32 array[], u32 size);
f64 Array_MaxAbs_flt64 (const f64 array[], u32 size);

//****************************************************************************//
//      Minimum and maximum value                                             //
//****************************************************************************//

//============================================================================//
//      Regular array search                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Minimum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u8 Array_Min_uint8 (const u8 array[], u32 size);
u16 Array_Min_uint16 (const u16 array[], u32 size);
u32 Array_Min_uint32 (const u32 array[], u32 size);

// Signed integer types
s8 Array_Min_sint8 (const s8 array[], u32 size);
s16 Array_Min_sint16 (const s16 array[], u32 size);
s32 Array_Min_sint32 (const s32 array[], u32 size);

// Floating-point types
f32 Array_Min_flt32 (const f32 array[], u32 size);
f64 Array_Min_flt64 (const f64 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Maximum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u8 Array_Max_uint8 (const u8 array[], u32 size);
u16 Array_Max_uint16 (const u16 array[], u32 size);
u32 Array_Max_uint32 (const u32 array[], u32 size);

// Signed integer types
s8 Array_Max_sint8 (const s8 array[], u32 size);
s16 Array_Max_sint16 (const s16 array[], u32 size);
s32 Array_Max_sint32 (const s32 array[], u32 size);

// Floating-point types
f32 Array_Max_flt32 (const f32 array[], u32 size);
f64 Array_Max_flt64 (const f64 array[], u32 size);

//============================================================================//
//      Object array search                                                   //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Minimum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u32 Array_MinObjFwd (const void* array[], u32 size, Cmp func);
u32 Array_MinObjBwd (const void* array[], u32 size, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Maximum value                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u32 Array_MaxObjFwd (const void* array[], u32 size, Cmp func);
u32 Array_MaxObjBwd (const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Linear search                                                         //
//****************************************************************************//

//============================================================================//
//      Bit field search                                                      //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Forward direction search                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for set bit                                                 //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindSetBitFwd_uint8 (const u8 array[], u32 spos, u32 epos);
u32 Array_FindSetBitFwd_uint16 (const u16 array[], u32 spos, u32 epos);
u32 Array_FindSetBitFwd_uint32 (const u32 array[], u32 spos, u32 epos);
u32 Array_FindSetBitFwd_uint64 (const u64 array[], u32 spos, u32 epos);

// Signed integer types
u32 Array_FindSetBitFwd_sint8 (const s8 array[], u32 spos, u32 epos);
u32 Array_FindSetBitFwd_sint16 (const s16 array[], u32 spos, u32 epos);
u32 Array_FindSetBitFwd_sint32 (const s32 array[], u32 spos, u32 epos);
u32 Array_FindSetBitFwd_sint64 (const s64 array[], u32 spos, u32 epos);

// Other types
u32 Array_FindSetBitFwd_size (const u32 array[], u32 spos, u32 epos);
u32 Array_FindSetBitFwd (const void *array, u32 spos, u32 epos);

//----------------------------------------------------------------------------//
//      Searching for reset bit                                               //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindResetBitFwd_uint8 (const u8 array[], u32 spos, u32 epos);
u32 Array_FindResetBitFwd_uint16 (const u16 array[], u32 spos, u32 epos);
u32 Array_FindResetBitFwd_uint32 (const u32 array[], u32 spos, u32 epos);
u32 Array_FindResetBitFwd_uint64 (const u64 array[], u32 spos, u32 epos);

// Signed integer types
u32 Array_FindResetBitFwd_sint8 (const s8 array[], u32 spos, u32 epos);
u32 Array_FindResetBitFwd_sint16 (const s16 array[], u32 spos, u32 epos);
u32 Array_FindResetBitFwd_sint32 (const s32 array[], u32 spos, u32 epos);
u32 Array_FindResetBitFwd_sint64 (const s64 array[], u32 spos, u32 epos);

// Other types
u32 Array_FindResetBitFwd_size (const u32 array[], u32 spos, u32 epos);
u32 Array_FindResetBitFwd (const void *array, u32 spos, u32 epos);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Backward direction search                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for set bit                                                 //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindSetBitBwd_uint8 (const u8 array[], u32 spos, u32 epos);
u32 Array_FindSetBitBwd_uint16 (const u16 array[], u32 spos, u32 epos);
u32 Array_FindSetBitBwd_uint32 (const u32 array[], u32 spos, u32 epos);
u32 Array_FindSetBitBwd_uint64 (const u64 array[], u32 spos, u32 epos);

// Signed integer types
u32 Array_FindSetBitBwd_sint8 (const s8 array[], u32 spos, u32 epos);
u32 Array_FindSetBitBwd_sint16 (const s16 array[], u32 spos, u32 epos);
u32 Array_FindSetBitBwd_sint32 (const s32 array[], u32 spos, u32 epos);
u32 Array_FindSetBitBwd_sint64 (const s64 array[], u32 spos, u32 epos);

// Other types
u32 Array_FindSetBitBwd_size (const u32 array[], u32 spos, u32 epos);
u32 Array_FindSetBitBwd (const void *array, u32 spos, u32 epos);

//----------------------------------------------------------------------------//
//      Searching for reset bit                                               //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindResetBitBwd_uint8 (const u8 array[], u32 spos, u32 epos);
u32 Array_FindResetBitBwd_uint16 (const u16 array[], u32 spos, u32 epos);
u32 Array_FindResetBitBwd_uint32 (const u32 array[], u32 spos, u32 epos);
u32 Array_FindResetBitBwd_uint64 (const u64 array[], u32 spos, u32 epos);

// Signed integer types
u32 Array_FindResetBitBwd_sint8 (const s8 array[], u32 spos, u32 epos);
u32 Array_FindResetBitBwd_sint16 (const s16 array[], u32 spos, u32 epos);
u32 Array_FindResetBitBwd_sint32 (const s32 array[], u32 spos, u32 epos);
u32 Array_FindResetBitBwd_sint64 (const s64 array[], u32 spos, u32 epos);

// Other types
u32 Array_FindResetBitBwd_size (const u32 array[], u32 spos, u32 epos);
u32 Array_FindResetBitBwd (const void *array, u32 spos, u32 epos);

//============================================================================//
//      Regular array search                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Forward direction search                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u32 Array_FindFwd_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindFwd_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindFwd_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindFwd_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindFwd_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindFwd_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindFwd_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindFwd_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindFwd_size (const u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Backward direction search                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u32 Array_FindBwd_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindBwd_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindBwd_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindBwd_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindBwd_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindBwd_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindBwd_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindBwd_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindBwd_size (const u32 array[], u32 size, u32 value);

//============================================================================//
//      Object array search                                                   //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Forward direction search                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u32 Array_FindObjFwd (const void* array[], u32 size, const void *value, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Backward direction search                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u32 Array_FindObjBwd (const void* array[], u32 size, const void *value, Cmp func);

//****************************************************************************//
//      Binary search                                                         //
//****************************************************************************//

//============================================================================//
//      Regular array search                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for first equal element                                     //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindFirstEqualAsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindFirstEqualAsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindFirstEqualAsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindFirstEqualAsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindFirstEqualAsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindFirstEqualAsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindFirstEqualAsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindFirstEqualAsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindFirstEqualAsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for last equal element                                      //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindLastEqualAsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindLastEqualAsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindLastEqualAsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindLastEqualAsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindLastEqualAsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindLastEqualAsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindLastEqualAsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindLastEqualAsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindLastEqualAsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for greater element                                         //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindGreatAsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindGreatAsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindGreatAsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindGreatAsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindGreatAsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindGreatAsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindGreatAsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindGreatAsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindGreatAsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for greater or equal element                                //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindGreatOrEqualAsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindGreatOrEqualAsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindGreatOrEqualAsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindGreatOrEqualAsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindGreatOrEqualAsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindGreatOrEqualAsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindGreatOrEqualAsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindGreatOrEqualAsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindGreatOrEqualAsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for less element                                            //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindLessAsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindLessAsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindLessAsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindLessAsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindLessAsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindLessAsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindLessAsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindLessAsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindLessAsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for less or equal element                                   //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindLessOrEqualAsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindLessOrEqualAsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindLessOrEqualAsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindLessOrEqualAsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindLessOrEqualAsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindLessOrEqualAsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindLessOrEqualAsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindLessOrEqualAsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindLessOrEqualAsc_size (const u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for first equal element                                     //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindFirstEqualDsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindFirstEqualDsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindFirstEqualDsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindFirstEqualDsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindFirstEqualDsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindFirstEqualDsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindFirstEqualDsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindFirstEqualDsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindFirstEqualDsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for last equal element                                      //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindLastEqualDsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindLastEqualDsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindLastEqualDsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindLastEqualDsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindLastEqualDsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindLastEqualDsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindLastEqualDsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindLastEqualDsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindLastEqualDsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for less element                                            //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindLessDsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindLessDsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindLessDsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindLessDsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindLessDsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindLessDsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindLessDsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindLessDsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindLessDsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for less or equal element                                   //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindLessOrEqualDsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindLessOrEqualDsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindLessOrEqualDsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindLessOrEqualDsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindLessOrEqualDsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindLessOrEqualDsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindLessOrEqualDsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindLessOrEqualDsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindLessOrEqualDsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for greater element                                         //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindGreatDsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindGreatDsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindGreatDsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindGreatDsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindGreatDsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindGreatDsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindGreatDsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindGreatDsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindGreatDsc_size (const u32 array[], u32 size, u32 value);

//----------------------------------------------------------------------------//
//      Searching for greater or equal element                                //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_FindGreatOrEqualDsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_FindGreatOrEqualDsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_FindGreatOrEqualDsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_FindGreatOrEqualDsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_FindGreatOrEqualDsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_FindGreatOrEqualDsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_FindGreatOrEqualDsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_FindGreatOrEqualDsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_FindGreatOrEqualDsc_size (const u32 array[], u32 size, u32 value);

//============================================================================//
//      Object array search                                                   //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for first equal element                                     //
//----------------------------------------------------------------------------//
u32 Array_FindFirstEqualObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for last equal element                                      //
//----------------------------------------------------------------------------//
u32 Array_FindLastEqualObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for greater element                                         //
//----------------------------------------------------------------------------//
u32 Array_FindGreatObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for greater or equal element                                //
//----------------------------------------------------------------------------//
u32 Array_FindGreatOrEqualObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for less element                                            //
//----------------------------------------------------------------------------//
u32 Array_FindLessObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for less or equal element                                   //
//----------------------------------------------------------------------------//
u32 Array_FindLessOrEqualObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Searching for first equal element                                     //
//----------------------------------------------------------------------------//
u32 Array_FindFirstEqualObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for last equal element                                      //
//----------------------------------------------------------------------------//
u32 Array_FindLastEqualObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for less element                                            //
//----------------------------------------------------------------------------//
u32 Array_FindLessObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for less or equal element                                   //
//----------------------------------------------------------------------------//
u32 Array_FindLessOrEqualObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for greater element                                         //
//----------------------------------------------------------------------------//
u32 Array_FindGreatObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//----------------------------------------------------------------------------//
//      Searching for greater or equal element                                //
//----------------------------------------------------------------------------//
u32 Array_FindGreatOrEqualObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//****************************************************************************//
//      Linear counting                                                       //
//****************************************************************************//

//============================================================================//
//      Bit counting                                                          //
//============================================================================//

// Unsigned integer types
u32 Array_CountBits_uint8 (const u8 array[], u32 spos, u32 epos);
u32 Array_CountBits_uint16 (const u16 array[], u32 spos, u32 epos);
u32 Array_CountBits_uint32 (const u32 array[], u32 spos, u32 epos);
u32 Array_CountBits_uint64 (const u64 array[], u32 spos, u32 epos);

// Signed integer types
u32 Array_CountBits_sint8 (const s8 array[], u32 spos, u32 epos);
u32 Array_CountBits_sint16 (const s16 array[], u32 spos, u32 epos);
u32 Array_CountBits_sint32 (const s32 array[], u32 spos, u32 epos);
u32 Array_CountBits_sint64 (const s64 array[], u32 spos, u32 epos);

// Other types
u32 Array_CountBits_size (const u32 array[], u32 spos, u32 epos);
u32 Array_CountBits (const void *array, u32 spos, u32 epos);

//============================================================================//
//      Element counting                                                      //
//============================================================================//

// Unsigned integer types
u32 Array_Count_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_Count_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_Count_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_Count_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_Count_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_Count_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_Count_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_Count_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_Count_size (const u32 array[], u32 size, u32 value);

//============================================================================//
//      Object counting                                                       //
//============================================================================//
u32 Array_CountObj (const void* array[], u32 size, const void *value, Cmp func);

//****************************************************************************//
//      Binary counting                                                       //
//****************************************************************************//

//============================================================================//
//      Element counting                                                      //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u32 Array_CountAsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_CountAsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_CountAsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_CountAsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_CountAsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_CountAsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_CountAsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_CountAsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_CountAsc_size (const u32 array[], u32 size, u32 value);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u32 Array_CountDsc_uint8 (const u8 array[], u32 size, u8 value);
u32 Array_CountDsc_uint16 (const u16 array[], u32 size, u16 value);
u32 Array_CountDsc_uint32 (const u32 array[], u32 size, u32 value);
u32 Array_CountDsc_uint64 (const u64 array[], u32 size, u64 value);

// Signed integer types
u32 Array_CountDsc_sint8 (const s8 array[], u32 size, s8 value);
u32 Array_CountDsc_sint16 (const s16 array[], u32 size, s16 value);
u32 Array_CountDsc_sint32 (const s32 array[], u32 size, s32 value);
u32 Array_CountDsc_sint64 (const s64 array[], u32 size, s64 value);

// Other types
u32 Array_CountDsc_size (const u32 array[], u32 size, u32 value);

//============================================================================//
//      Object counting                                                       //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u32 Array_CountObjAsc (const void* array[], u32 size, const void *value, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u32 Array_CountObjDsc (const void* array[], u32 size, const void *value, Cmp func);

//****************************************************************************//
//      Replacing                                                             //
//****************************************************************************//

//============================================================================//
//      Element replacing                                                     //
//============================================================================//

// Unsigned integer types
void Array_Replace_uint8 (u8 array[], u32 size, u8 pattern, u8 value);
void Array_Replace_uint16 (u16 array[], u32 size, u16 pattern, u16 value);
void Array_Replace_uint32 (u32 array[], u32 size, u32 pattern, u32 value);
void Array_Replace_uint64 (u64 array[], u32 size, u64 pattern, u64 value);

// Signed integer types
void Array_Replace_sint8 (s8 array[], u32 size, s8 pattern, s8 value);
void Array_Replace_sint16 (s16 array[], u32 size, s16 pattern, s16 value);
void Array_Replace_sint32 (s32 array[], u32 size, s32 pattern, s32 value);
void Array_Replace_sint64 (s64 array[], u32 size, s64 pattern, s64 value);

// Other types
void Array_Replace_size (u32 array[], u32 size, u32 pattern, u32 value);

//============================================================================//
//      Object replacing                                                      //
//============================================================================//
void Array_ReplaceObj (const void* array[], u32 size, const void *pattern, const void *value, Cmp func);

//****************************************************************************//
//      Order reversing                                                       //
//****************************************************************************//

//============================================================================//
//      Regular array reversing                                               //
//============================================================================//

// Unsigned integer types
void Array_Reverse_uint8 (u8 array[], u32 size);
void Array_Reverse_uint16 (u16 array[], u32 size);
void Array_Reverse_uint32 (u32 array[], u32 size);
void Array_Reverse_uint64 (u64 array[], u32 size);

// Signed integer types
void Array_Reverse_sint8 (s8 array[], u32 size);
void Array_Reverse_sint16 (s16 array[], u32 size);
void Array_Reverse_sint32 (s32 array[], u32 size);
void Array_Reverse_sint64 (s64 array[], u32 size);

// Floating-point types
void Array_Reverse_flt32 (f32 array[], u32 size);
void Array_Reverse_flt64 (f64 array[], u32 size);

// Other types
void Array_Reverse_size (u32 array[], u32 size);

//============================================================================//
//      Object array reversing                                                //
//============================================================================//
void Array_ReverseObj (const void* array[], u32 size);

//****************************************************************************//
//      Unique values                                                         //
//****************************************************************************//

//============================================================================//
//      Unique elements                                                       //
//============================================================================//

// Unsigned integer types
u32 Array_Unique_uint8 (u8 unique[], const u8 array[], u32 size);
u32 Array_Unique_uint16 (u16 unique[], const u16 array[], u32 size);
u32 Array_Unique_uint32 (u32 unique[], const u32 array[], u32 size);
u32 Array_Unique_uint64 (u64 unique[], const u64 array[], u32 size);

// Signed integer types
u32 Array_Unique_sint8 (s8 unique[], const s8 array[], u32 size);
u32 Array_Unique_sint16 (s16 unique[], const s16 array[], u32 size);
u32 Array_Unique_sint32 (s32 unique[], const s32 array[], u32 size);
u32 Array_Unique_sint64 (s64 unique[], const s64 array[], u32 size);

// Other types
u32 Array_Unique_size (u32 unique[], const u32 array[], u32 size);

//============================================================================//
//      Unique objects                                                        //
//============================================================================//
u32 Array_UniqueObj (const void* unique[], const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Duplicate values                                                      //
//****************************************************************************//

//============================================================================//
//      Duplicate elements                                                    //
//============================================================================//

// Unsigned integer types
u32 Array_Duplicates_uint8 (u8 unique[], u32 count[], const u8 array[], u32 size);
u32 Array_Duplicates_uint16 (u16 unique[], u32 count[], const u16 array[], u32 size);
u32 Array_Duplicates_uint32 (u32 unique[], u32 count[], const u32 array[], u32 size);
u32 Array_Duplicates_uint64 (u64 unique[], u32 count[], const u64 array[], u32 size);

// Signed integer types
u32 Array_Duplicates_sint8 (s8 unique[], u32 count[], const s8 array[], u32 size);
u32 Array_Duplicates_sint16 (s16 unique[], u32 count[], const s16 array[], u32 size);
u32 Array_Duplicates_sint32 (s32 unique[], u32 count[], const s32 array[], u32 size);
u32 Array_Duplicates_sint64 (s64 unique[], u32 count[], const s64 array[], u32 size);

// Other types
u32 Array_Duplicates_size (u32 unique[], u32 count[], const u32 array[], u32 size);

//============================================================================//
//      Duplicate objects                                                     //
//============================================================================//
u32 Array_DuplicatesObj (const void* unique[], u32 count[], const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Insertion sort                                                        //
//****************************************************************************//

//============================================================================//
//      Regular array sorting                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_InsertSortAsc_uint8 (u8 array[], u32 size);
void Array_InsertSortAsc_uint16 (u16 array[], u32 size);
void Array_InsertSortAsc_uint32 (u32 array[], u32 size);
void Array_InsertSortAsc_uint64 (u64 array[], u32 size);

// Signed integer types
void Array_InsertSortAsc_sint8 (s8 array[], u32 size);
void Array_InsertSortAsc_sint16 (s16 array[], u32 size);
void Array_InsertSortAsc_sint32 (s32 array[], u32 size);
void Array_InsertSortAsc_sint64 (s64 array[], u32 size);

// Floating-point types
void Array_InsertSortAsc_flt32 (f32 array[], u32 size);
void Array_InsertSortAsc_flt64 (f64 array[], u32 size);

// Other types
void Array_InsertSortAsc_size (u32 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_InsertSortDsc_uint8 (u8 array[], u32 size);
void Array_InsertSortDsc_uint16 (u16 array[], u32 size);
void Array_InsertSortDsc_uint32 (u32 array[], u32 size);
void Array_InsertSortDsc_uint64 (u64 array[], u32 size);

// Signed integer types
void Array_InsertSortDsc_sint8 (s8 array[], u32 size);
void Array_InsertSortDsc_sint16 (s16 array[], u32 size);
void Array_InsertSortDsc_sint32 (s32 array[], u32 size);
void Array_InsertSortDsc_sint64 (s64 array[], u32 size);

// Floating-point types
void Array_InsertSortDsc_flt32 (f32 array[], u32 size);
void Array_InsertSortDsc_flt64 (f64 array[], u32 size);

// Other types
void Array_InsertSortDsc_size (u32 array[], u32 size);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_InsertSortKeyAsc_uint8 (u8 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyAsc_uint16 (u16 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyAsc_uint32 (u32 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyAsc_uint64 (u64 key[], const void* ptr[], u32 size);

// Signed integer types
void Array_InsertSortKeyAsc_sint8 (s8 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyAsc_sint16 (s16 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyAsc_sint32 (s32 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyAsc_sint64 (s64 key[], const void* ptr[], u32 size);

// Floating-point types
void Array_InsertSortKeyAsc_flt32 (f32 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyAsc_flt64 (f64 key[], const void* ptr[], u32 size);

// Other types
void Array_InsertSortKeyAsc_size (u32 key[], const void* ptr[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_InsertSortKeyDsc_uint8 (u8 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyDsc_uint16 (u16 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyDsc_uint32 (u32 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyDsc_uint64 (u64 key[], const void* ptr[], u32 size);

// Signed integer types
void Array_InsertSortKeyDsc_sint8 (s8 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyDsc_sint16 (s16 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyDsc_sint32 (s32 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyDsc_sint64 (s64 key[], const void* ptr[], u32 size);

// Floating-point types
void Array_InsertSortKeyDsc_flt32 (f32 key[], const void* ptr[], u32 size);
void Array_InsertSortKeyDsc_flt64 (f64 key[], const void* ptr[], u32 size);

// Other types
void Array_InsertSortKeyDsc_size (u32 key[], const void* ptr[], u32 size);

//============================================================================//
//      Object array sorting                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_InsertSortObjAsc (const void* array[], u32 size, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_InsertSortObjDsc (const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Quick sort                                                            //
//****************************************************************************//

//============================================================================//
//      Regular array sorting                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_QuickSortAsc_uint8 (u8 array[], u32 size);
void Array_QuickSortAsc_uint16 (u16 array[], u32 size);
void Array_QuickSortAsc_uint32 (u32 array[], u32 size);
void Array_QuickSortAsc_uint64 (u64 array[], u32 size);

// Signed integer types
void Array_QuickSortAsc_sint8 (s8 array[], u32 size);
void Array_QuickSortAsc_sint16 (s16 array[], u32 size);
void Array_QuickSortAsc_sint32 (s32 array[], u32 size);
void Array_QuickSortAsc_sint64 (s64 array[], u32 size);

// Floating-point types
void Array_QuickSortAsc_flt32 (f32 array[], u32 size);
void Array_QuickSortAsc_flt64 (f64 array[], u32 size);

// Other types
void Array_QuickSortAsc_size (u32 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_QuickSortDsc_uint8 (u8 array[], u32 size);
void Array_QuickSortDsc_uint16 (u16 array[], u32 size);
void Array_QuickSortDsc_uint32 (u32 array[], u32 size);
void Array_QuickSortDsc_uint64 (u64 array[], u32 size);

// Signed integer types
void Array_QuickSortDsc_sint8 (s8 array[], u32 size);
void Array_QuickSortDsc_sint16 (s16 array[], u32 size);
void Array_QuickSortDsc_sint32 (s32 array[], u32 size);
void Array_QuickSortDsc_sint64 (s64 array[], u32 size);

// Floating-point types
void Array_QuickSortDsc_flt32 (f32 array[], u32 size);
void Array_QuickSortDsc_flt64 (f64 array[], u32 size);

// Other types
void Array_QuickSortDsc_size (u32 array[], u32 size);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_QuickSortKeyAsc_uint8 (u8 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyAsc_uint16 (u16 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyAsc_uint32 (u32 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyAsc_uint64 (u64 key[], const void* ptr[], u32 size);

// Signed integer types
void Array_QuickSortKeyAsc_sint8 (s8 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyAsc_sint16 (s16 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyAsc_sint32 (s32 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyAsc_sint64 (s64 key[], const void* ptr[], u32 size);

// Floating-point types
void Array_QuickSortKeyAsc_flt32 (f32 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyAsc_flt64 (f64 key[], const void* ptr[], u32 size);

// Other types
void Array_QuickSortKeyAsc_size (u32 key[], const void* ptr[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_QuickSortKeyDsc_uint8 (u8 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyDsc_uint16 (u16 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyDsc_uint32 (u32 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyDsc_uint64 (u64 key[], const void* ptr[], u32 size);

// Signed integer types
void Array_QuickSortKeyDsc_sint8 (s8 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyDsc_sint16 (s16 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyDsc_sint32 (s32 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyDsc_sint64 (s64 key[], const void* ptr[], u32 size);

// Floating-point types
void Array_QuickSortKeyDsc_flt32 (f32 key[], const void* ptr[], u32 size);
void Array_QuickSortKeyDsc_flt64 (f64 key[], const void* ptr[], u32 size);

// Other types
void Array_QuickSortKeyDsc_size (u32 key[], const void* ptr[], u32 size);

//============================================================================//
//      Object array sorting                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_QuickSortObjAsc (const void* array[], u32 size, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_QuickSortObjDsc (const void* array[], u32 size, Cmp func);

//****************************************************************************//
//      Merge sort                                                            //
//****************************************************************************//

//============================================================================//
//      Regular array sorting                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_MergeSortAsc_uint8 (u8 array[], u8 temp[], u32 size);
void Array_MergeSortAsc_uint16 (u16 array[], u16 temp[], u32 size);
void Array_MergeSortAsc_uint32 (u32 array[], u32 temp[], u32 size);
void Array_MergeSortAsc_uint64 (u64 array[], u64 temp[], u32 size);

// Signed integer types
void Array_MergeSortAsc_sint8 (s8 array[], s8 temp[], u32 size);
void Array_MergeSortAsc_sint16 (s16 array[], s16 temp[], u32 size);
void Array_MergeSortAsc_sint32 (s32 array[], s32 temp[], u32 size);
void Array_MergeSortAsc_sint64 (s64 array[], s64 temp[], u32 size);

// Floating-point types
void Array_MergeSortAsc_flt32 (f32 array[], f32 temp[], u32 size);
void Array_MergeSortAsc_flt64 (f64 array[], f64 temp[], u32 size);

// Other types
void Array_MergeSortAsc_size (u32 array[], u32 temp[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_MergeSortDsc_uint8 (u8 array[], u8 temp[], u32 size);
void Array_MergeSortDsc_uint16 (u16 array[], u16 temp[], u32 size);
void Array_MergeSortDsc_uint32 (u32 array[], u32 temp[], u32 size);
void Array_MergeSortDsc_uint64 (u64 array[], u64 temp[], u32 size);

// Signed integer types
void Array_MergeSortDsc_sint8 (s8 array[], s8 temp[], u32 size);
void Array_MergeSortDsc_sint16 (s16 array[], s16 temp[], u32 size);
void Array_MergeSortDsc_sint32 (s32 array[], s32 temp[], u32 size);
void Array_MergeSortDsc_sint64 (s64 array[], s64 temp[], u32 size);

// Floating-point types
void Array_MergeSortDsc_flt32 (f32 array[], f32 temp[], u32 size);
void Array_MergeSortDsc_flt64 (f64 array[], f64 temp[], u32 size);

// Other types
void Array_MergeSortDsc_size (u32 array[], u32 temp[], u32 size);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_MergeSortKeyAsc_uint8 (u8 key[], const void* ptr[], u8 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyAsc_uint16 (u16 key[], const void* ptr[], u16 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyAsc_uint32 (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyAsc_uint64 (u64 key[], const void* ptr[], u64 tkey[], const void* tptr[], u32 size);

// Signed integer types
void Array_MergeSortKeyAsc_sint8 (s8 key[], const void* ptr[], s8 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyAsc_sint16 (s16 key[], const void* ptr[], s16 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyAsc_sint32 (s32 key[], const void* ptr[], s32 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyAsc_sint64 (s64 key[], const void* ptr[], s64 tkey[], const void* tptr[], u32 size);

// Floating-point types
void Array_MergeSortKeyAsc_flt32 (f32 key[], const void* ptr[], f32 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyAsc_flt64 (f64 key[], const void* ptr[], f64 tkey[], const void* tptr[], u32 size);

// Other types
void Array_MergeSortKeyAsc_size (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_MergeSortKeyDsc_uint8 (u8 key[], const void* ptr[], u8 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyDsc_uint16 (u16 key[], const void* ptr[], u16 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyDsc_uint32 (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyDsc_uint64 (u64 key[], const void* ptr[], u64 tkey[], const void* tptr[], u32 size);

// Signed integer types
void Array_MergeSortKeyDsc_sint8 (s8 key[], const void* ptr[], s8 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyDsc_sint16 (s16 key[], const void* ptr[], s16 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyDsc_sint32 (s32 key[], const void* ptr[], s32 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyDsc_sint64 (s64 key[], const void* ptr[], s64 tkey[], const void* tptr[], u32 size);

// Floating-point types
void Array_MergeSortKeyDsc_flt32 (f32 key[], const void* ptr[], f32 tkey[], const void* tptr[], u32 size);
void Array_MergeSortKeyDsc_flt64 (f64 key[], const void* ptr[], f64 tkey[], const void* tptr[], u32 size);

// Other types
void Array_MergeSortKeyDsc_size (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);

//============================================================================//
//      Object array sorting                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_MergeSortObjAsc (const void* array[], const void* temp[], u32 size, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_MergeSortObjDsc (const void* array[], const void* temp[], u32 size, Cmp func);

//****************************************************************************//
//      Radix sort                                                            //
//****************************************************************************//

//============================================================================//
//      Regular array sorting                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_RadixSortAsc_uint8 (u8 array[], u8 temp[], u32 size);
void Array_RadixSortAsc_uint16 (u16 array[], u16 temp[], u32 size);
void Array_RadixSortAsc_uint32 (u32 array[], u32 temp[], u32 size);
void Array_RadixSortAsc_uint64 (u64 array[], u64 temp[], u32 size);

// Signed integer types
void Array_RadixSortAsc_sint8 (s8 array[], s8 temp[], u32 size);
void Array_RadixSortAsc_sint16 (s16 array[], s16 temp[], u32 size);
void Array_RadixSortAsc_sint32 (s32 array[], s32 temp[], u32 size);
void Array_RadixSortAsc_sint64 (s64 array[], s64 temp[], u32 size);

// Floating-point types
void Array_RadixSortAsc_flt32 (f32 array[], f32 temp[], u32 size);
void Array_RadixSortAsc_flt64 (f64 array[], f64 temp[], u32 size);

// Other types
void Array_RadixSortAsc_size (u32 array[], u32 temp[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_RadixSortDsc_uint8 (u8 array[], u8 temp[], u32 size);
void Array_RadixSortDsc_uint16 (u16 array[], u16 temp[], u32 size);
void Array_RadixSortDsc_uint32 (u32 array[], u32 temp[], u32 size);
void Array_RadixSortDsc_uint64 (u64 array[], u64 temp[], u32 size);

// Signed integer types
void Array_RadixSortDsc_sint8 (s8 array[], s8 temp[], u32 size);
void Array_RadixSortDsc_sint16 (s16 array[], s16 temp[], u32 size);
void Array_RadixSortDsc_sint32 (s32 array[], s32 temp[], u32 size);
void Array_RadixSortDsc_sint64 (s64 array[], s64 temp[], u32 size);

// Floating-point types
void Array_RadixSortDsc_flt32 (f32 array[], f32 temp[], u32 size);
void Array_RadixSortDsc_flt64 (f64 array[], f64 temp[], u32 size);

// Other types
void Array_RadixSortDsc_size (u32 array[], u32 temp[], u32 size);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_RadixSortKeyAsc_uint8 (u8 key[], const void* ptr[], u8 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyAsc_uint16 (u16 key[], const void* ptr[], u16 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyAsc_uint32 (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyAsc_uint64 (u64 key[], const void* ptr[], u64 tkey[], const void* tptr[], u32 size);

// Signed integer types
void Array_RadixSortKeyAsc_sint8 (s8 key[], const void* ptr[], s8 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyAsc_sint16 (s16 key[], const void* ptr[], s16 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyAsc_sint32 (s32 key[], const void* ptr[], s32 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyAsc_sint64 (s64 key[], const void* ptr[], s64 tkey[], const void* tptr[], u32 size);

// Floating-point types
void Array_RadixSortKeyAsc_flt32 (f32 key[], const void* ptr[], f32 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyAsc_flt64 (f64 key[], const void* ptr[], f64 tkey[], const void* tptr[], u32 size);

// Other types
void Array_RadixSortKeyAsc_size (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_RadixSortKeyDsc_uint8 (u8 key[], const void* ptr[], u8 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyDsc_uint16 (u16 key[], const void* ptr[], u16 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyDsc_uint32 (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyDsc_uint64 (u64 key[], const void* ptr[], u64 tkey[], const void* tptr[], u32 size);

// Signed integer types
void Array_RadixSortKeyDsc_sint8 (s8 key[], const void* ptr[], s8 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyDsc_sint16 (s16 key[], const void* ptr[], s16 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyDsc_sint32 (s32 key[], const void* ptr[], s32 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyDsc_sint64 (s64 key[], const void* ptr[], s64 tkey[], const void* tptr[], u32 size);

// Floating-point types
void Array_RadixSortKeyDsc_flt32 (f32 key[], const void* ptr[], f32 tkey[], const void* tptr[], u32 size);
void Array_RadixSortKeyDsc_flt64 (f64 key[], const void* ptr[], f64 tkey[], const void* tptr[], u32 size);

// Other types
void Array_RadixSortKeyDsc_size (u32 key[], const void* ptr[], u32 tkey[], const void* tptr[], u32 size);

//****************************************************************************//
//      Merging of sorted arrays                                              //
//****************************************************************************//

//============================================================================//
//      Regular array merging                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_MergeAsc_uint8 (u8 target[], const u8 source1[], u32 size1, const u8 source2[], u32 size2);
void Array_MergeAsc_uint16 (u16 target[], const u16 source1[], u32 size1, const u16 source2[], u32 size2);
void Array_MergeAsc_uint32 (u32 target[], const u32 source1[], u32 size1, const u32 source2[], u32 size2);
void Array_MergeAsc_uint64 (u64 target[], const u64 source1[], u32 size1, const u64 source2[], u32 size2);

// Signed integer types
void Array_MergeAsc_sint8 (s8 target[], const s8 source1[], u32 size1, const s8 source2[], u32 size2);
void Array_MergeAsc_sint16 (s16 target[], const s16 source1[], u32 size1, const s16 source2[], u32 size2);
void Array_MergeAsc_sint32 (s32 target[], const s32 source1[], u32 size1, const s32 source2[], u32 size2);
void Array_MergeAsc_sint64 (s64 target[], const s64 source1[], u32 size1, const s64 source2[], u32 size2);

// Floating-point types
void Array_MergeAsc_flt32 (f32 target[], const f32 source1[], u32 size1, const f32 source2[], u32 size2);
void Array_MergeAsc_flt64 (f64 target[], const f64 source1[], u32 size1, const f64 source2[], u32 size2);

// Other types
void Array_MergeAsc_size (u32 target[], const u32 source1[], u32 size1, const u32 source2[], u32 size2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_MergeDsc_uint8 (u8 target[], const u8 source1[], u32 size1, const u8 source2[], u32 size2);
void Array_MergeDsc_uint16 (u16 target[], const u16 source1[], u32 size1, const u16 source2[], u32 size2);
void Array_MergeDsc_uint32 (u32 target[], const u32 source1[], u32 size1, const u32 source2[], u32 size2);
void Array_MergeDsc_uint64 (u64 target[], const u64 source1[], u32 size1, const u64 source2[], u32 size2);

// Signed integer types
void Array_MergeDsc_sint8 (s8 target[], const s8 source1[], u32 size1, const s8 source2[], u32 size2);
void Array_MergeDsc_sint16 (s16 target[], const s16 source1[], u32 size1, const s16 source2[], u32 size2);
void Array_MergeDsc_sint32 (s32 target[], const s32 source1[], u32 size1, const s32 source2[], u32 size2);
void Array_MergeDsc_sint64 (s64 target[], const s64 source1[], u32 size1, const s64 source2[], u32 size2);

// Floating-point types
void Array_MergeDsc_flt32 (f32 target[], const f32 source1[], u32 size1, const f32 source2[], u32 size2);
void Array_MergeDsc_flt64 (f64 target[], const f64 source1[], u32 size1, const f64 source2[], u32 size2);

// Other types
void Array_MergeDsc_size (u32 target[], const u32 source1[], u32 size1, const u32 source2[], u32 size2);

//============================================================================//
//      Key array merging                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_MergeKeyAsc_uint8 (u8 tkey[], const void* tptr[], const u8 skey1[], const void* sptr1[], u32 size1, const u8 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyAsc_uint16 (u16 tkey[], const void* tptr[], const u16 skey1[], const void* sptr1[], u32 size1, const u16 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyAsc_uint32 (u32 tkey[], const void* tptr[], const u32 skey1[], const void* sptr1[], u32 size1, const u32 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyAsc_uint64 (u64 tkey[], const void* tptr[], const u64 skey1[], const void* sptr1[], u32 size1, const u64 skey2[], const void* sptr2[], u32 size2);

// Signed integer types
void Array_MergeKeyAsc_sint8 (s8 tkey[], const void* tptr[], const s8 skey1[], const void* sptr1[], u32 size1, const s8 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyAsc_sint16 (s16 tkey[], const void* tptr[], const s16 skey1[], const void* sptr1[], u32 size1, const s16 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyAsc_sint32 (s32 tkey[], const void* tptr[], const s32 skey1[], const void* sptr1[], u32 size1, const s32 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyAsc_sint64 (s64 tkey[], const void* tptr[], const s64 skey1[], const void* sptr1[], u32 size1, const s64 skey2[], const void* sptr2[], u32 size2);

// Floating-point types
void Array_MergeKeyAsc_flt32 (f32 tkey[], const void* tptr[], const f32 skey1[], const void* sptr1[], u32 size1, const f32 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyAsc_flt64 (f64 tkey[], const void* tptr[], const f64 skey1[], const void* sptr1[], u32 size1, const f64 skey2[], const void* sptr2[], u32 size2);

// Other types
void Array_MergeKeyAsc_size (u32 tkey[], const void* tptr[], const u32 skey1[], const void* sptr1[], u32 size1, const u32 skey2[], const void* sptr2[], u32 size2);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
void Array_MergeKeyDsc_uint8 (u8 tkey[], const void* tptr[], const u8 skey1[], const void* sptr1[], u32 size1, const u8 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyDsc_uint16 (u16 tkey[], const void* tptr[], const u16 skey1[], const void* sptr1[], u32 size1, const u16 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyDsc_uint32 (u32 tkey[], const void* tptr[], const u32 skey1[], const void* sptr1[], u32 size1, const u32 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyDsc_uint64 (u64 tkey[], const void* tptr[], const u64 skey1[], const void* sptr1[], u32 size1, const u64 skey2[], const void* sptr2[], u32 size2);

// Signed integer types
void Array_MergeKeyDsc_sint8 (s8 tkey[], const void* tptr[], const s8 skey1[], const void* sptr1[], u32 size1, const s8 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyDsc_sint16 (s16 tkey[], const void* tptr[], const s16 skey1[], const void* sptr1[], u32 size1, const s16 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyDsc_sint32 (s32 tkey[], const void* tptr[], const s32 skey1[], const void* sptr1[], u32 size1, const s32 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyDsc_sint64 (s64 tkey[], const void* tptr[], const s64 skey1[], const void* sptr1[], u32 size1, const s64 skey2[], const void* sptr2[], u32 size2);

// Floating-point types
void Array_MergeKeyDsc_flt32 (f32 tkey[], const void* tptr[], const f32 skey1[], const void* sptr1[], u32 size1, const f32 skey2[], const void* sptr2[], u32 size2);
void Array_MergeKeyDsc_flt64 (f64 tkey[], const void* tptr[], const f64 skey1[], const void* sptr1[], u32 size1, const f64 skey2[], const void* sptr2[], u32 size2);

// Other types
void Array_MergeKeyDsc_size (u32 tkey[], const void* tptr[], const u32 skey1[], const void* sptr1[], u32 size1, const u32 skey2[], const void* sptr2[], u32 size2);

//============================================================================//
//      Object array merging                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_MergeObjAsc (const void* target[], const void* source1[], u32 size1, const void* source2[], u32 size2, Cmp func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Array_MergeObjDsc (const void* target[], const void* source1[], u32 size1, const void* source2[], u32 size2, Cmp func);

//****************************************************************************//
//      Comparison of arrays                                                  //
//****************************************************************************//

//============================================================================//
//      Regular array comparison                                              //
//============================================================================//

// Unsigned integer types
s64 Array_Compare_uint8 (const u8 array1[], const u8 array2[], u32 size);
s64 Array_Compare_uint16 (const u16 array1[], const u16 array2[], u32 size);
s64 Array_Compare_uint32 (const u32 array1[], const u32 array2[], u32 size);
s64 Array_Compare_uint64 (const u64 array1[], const u64 array2[], u32 size);

// Signed integer types
s64 Array_Compare_sint8 (const s8 array1[], const s8 array2[], u32 size);
s64 Array_Compare_sint16 (const s16 array1[], const s16 array2[], u32 size);
s64 Array_Compare_sint32 (const s32 array1[], const s32 array2[], u32 size);
s64 Array_Compare_sint64 (const s64 array1[], const s64 array2[], u32 size);

// Floating-point types
s64 Array_Compare_flt32 (const f32 array1[], const f32 array2[], u32 size);
s64 Array_Compare_flt64 (const f64 array1[], const f64 array2[], u32 size);

// Other types
s64 Array_Compare_size (const u32 array1[], const u32 array2[], u32 size);
s64 Array_Compare (const void *array1, const void *array2, u32 size);

//============================================================================//
//      Object array comparison                                               //
//============================================================================//
s64 Array_CompareObj (const void* array1[], const void* array2[], u32 size, Cmp func);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

//============================================================================//
//      Check for differences                                                 //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Regular array check                                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u32 Array_CheckDiff_uint8 (const u8 array1[], const u8 array2[], u32 size);
u32 Array_CheckDiff_uint16 (const u16 array1[], const u16 array2[], u32 size);
u32 Array_CheckDiff_uint32 (const u32 array1[], const u32 array2[], u32 size);
u32 Array_CheckDiff_uint64 (const u64 array1[], const u64 array2[], u32 size);

// Signed integer types
u32 Array_CheckDiff_sint8 (const s8 array1[], const s8 array2[], u32 size);
u32 Array_CheckDiff_sint16 (const s16 array1[], const s16 array2[], u32 size);
u32 Array_CheckDiff_sint32 (const s32 array1[], const s32 array2[], u32 size);
u32 Array_CheckDiff_sint64 (const s64 array1[], const s64 array2[], u32 size);

// Floating-point types
u32 Array_CheckDiff_flt32 (const f32 array1[], const f32 array2[], u32 size);
u32 Array_CheckDiff_flt64 (const f64 array1[], const f64 array2[], u32 size);

// Other types
u32 Array_CheckDiff_size (const u32 array1[], const u32 array2[], u32 size);
u32 Array_CheckDiff (const void *array1, const void *array2, u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Object array check                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u32 Array_CheckDiffObj (const void* array1[], const void* array2[], u32 size, Cmp func);

//============================================================================//
//      Check for duplicate values                                            //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Regular array check                                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Unsigned integer types
u32 Array_CheckDup_uint8 (const u8 array[], u32 size);
u32 Array_CheckDup_uint16 (const u16 array[], u32 size);
u32 Array_CheckDup_uint32 (const u32 array[], u32 size);
u32 Array_CheckDup_uint64 (const u64 array[], u32 size);

// Signed integer types
u32 Array_CheckDup_sint8 (const s8 array[], u32 size);
u32 Array_CheckDup_sint16 (const s16 array[], u32 size);
u32 Array_CheckDup_sint32 (const s32 array[], u32 size);
u32 Array_CheckDup_sint64 (const s64 array[], u32 size);

// Floating-point types
u32 Array_CheckDup_flt32 (const f32 array[], u32 size);
u32 Array_CheckDup_flt64 (const f64 array[], u32 size);

// Other types
u32 Array_CheckDup_size (const u32 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Object array check                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u32 Array_CheckDupObj (const void* array[], u32 size, Cmp func);

//============================================================================//
//      Check for sort order                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Regular array check                                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Check for ascending sort order                                        //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_CheckSortAsc_uint8 (const u8 array[], u32 size);
u32 Array_CheckSortAsc_uint16 (const u16 array[], u32 size);
u32 Array_CheckSortAsc_uint32 (const u32 array[], u32 size);
u32 Array_CheckSortAsc_uint64 (const u64 array[], u32 size);

// Signed integer types
u32 Array_CheckSortAsc_sint8 (const s8 array[], u32 size);
u32 Array_CheckSortAsc_sint16 (const s16 array[], u32 size);
u32 Array_CheckSortAsc_sint32 (const s32 array[], u32 size);
u32 Array_CheckSortAsc_sint64 (const s64 array[], u32 size);

// Floating-point types
u32 Array_CheckSortAsc_flt32 (const f32 array[], u32 size);
u32 Array_CheckSortAsc_flt64 (const f64 array[], u32 size);

// Other types
u32 Array_CheckSortAsc_size (const u32 array[], u32 size);

//----------------------------------------------------------------------------//
//      Check for descending sort order                                       //
//----------------------------------------------------------------------------//

// Unsigned integer types
u32 Array_CheckSortDsc_uint8 (const u8 array[], u32 size);
u32 Array_CheckSortDsc_uint16 (const u16 array[], u32 size);
u32 Array_CheckSortDsc_uint32 (const u32 array[], u32 size);
u32 Array_CheckSortDsc_uint64 (const u64 array[], u32 size);

// Signed integer types
u32 Array_CheckSortDsc_sint8 (const s8 array[], u32 size);
u32 Array_CheckSortDsc_sint16 (const s16 array[], u32 size);
u32 Array_CheckSortDsc_sint32 (const s32 array[], u32 size);
u32 Array_CheckSortDsc_sint64 (const s64 array[], u32 size);

// Floating-point types
u32 Array_CheckSortDsc_flt32 (const f32 array[], u32 size);
u32 Array_CheckSortDsc_flt64 (const f64 array[], u32 size);

// Other types
u32 Array_CheckSortDsc_size (const u32 array[], u32 size);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Object array check                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Check for ascending sort order                                        //
//----------------------------------------------------------------------------//
u32 Array_CheckSortObjAsc (const void* array[], u32 size, Cmp func);

//----------------------------------------------------------------------------//
//      Check for descending sort order                                       //
//----------------------------------------------------------------------------//
u32 Array_CheckSortObjDsc (const void* array[], u32 size, Cmp func);

//============================================================================//
//      Check for infinite values                                             //
//============================================================================//
u32 Array_CheckInf_flt32 (const f32 array[], u32 size);
u32 Array_CheckInf_flt64 (const f64 array[], u32 size);

//============================================================================//
//      Check for NaN values                                                  //
//============================================================================//
u32 Array_CheckNaN_flt32 (const f32 array[], u32 size);
u32 Array_CheckNaN_flt64 (const f64 array[], u32 size);

//============================================================================//
//      Check for overlap                                                     //
//============================================================================//

// Unsigned integer types
bool Array_Overlap_uint8 (const u8 array1[], u32 size1, const u8 array2[], u32 size2);
bool Array_Overlap_uint16 (const u16 array1[], u32 size1, const u16 array2[], u32 size2);
bool Array_Overlap_uint32 (const u32 array1[], u32 size1, const u32 array2[], u32 size2);
bool Array_Overlap_uint64 (const u64 array1[], u32 size1, const u64 array2[], u32 size2);

// Signed integer types
bool Array_Overlap_sint8 (const s8 array1[], u32 size1, const s8 array2[], u32 size2);
bool Array_Overlap_sint16 (const s16 array1[], u32 size1, const s16 array2[], u32 size2);
bool Array_Overlap_sint32 (const s32 array1[], u32 size1, const s32 array2[], u32 size2);
bool Array_Overlap_sint64 (const s64 array1[], u32 size1, const s64 array2[], u32 size2);

// Floating-point types
bool Array_Overlap_flt32 (const f32 array1[], u32 size1, const f32 array2[], u32 size2);
bool Array_Overlap_flt64 (const f64 array1[], u32 size1, const f64 array2[], u32 size2);

// Other types
bool Array_Overlap_size (const u32 array1[], u32 size1, const u32 array2[], u32 size2);
bool Array_Overlap (const void *array1, u32 size1, const void *array2, u32 size2);

//****************************************************************************//
//      Array hashing                                                         //
//****************************************************************************//

//============================================================================//
//      32-bit hash functions                                                 //
//============================================================================//

// Unsigned integer types
u32 Array_Hash32_uint8 (const u8 array[], u32 size);
u32 Array_Hash32_uint16 (const u16 array[], u32 size);
u32 Array_Hash32_uint32 (const u32 array[], u32 size);
u32 Array_Hash32_uint64 (const u64 array[], u32 size);

// Unsigned integer types
u32 Array_Hash32_sint8 (const s8 array[], u32 size);
u32 Array_Hash32_sint16 (const s16 array[], u32 size);
u32 Array_Hash32_sint32 (const s32 array[], u32 size);
u32 Array_Hash32_sint64 (const s64 array[], u32 size);

// Floating-point types
u32 Array_Hash32_flt32 (const f32 array[], u32 size);
u32 Array_Hash32_flt64 (const f64 array[], u32 size);

// Other types
u32 Array_Hash32_size (const u32 array[], u32 size);
u32 Array_Hash32 (const void *array, u32 size);

//============================================================================//
//      64-bit hash functions                                                 //
//============================================================================//

// Unsigned integer types
u64 Array_Hash64_uint8 (const u8 array[], u32 size);
u64 Array_Hash64_uint16 (const u16 array[], u32 size);
u64 Array_Hash64_uint32 (const u32 array[], u32 size);
u64 Array_Hash64_uint64 (const u64 array[], u32 size);

// Unsigned integer types
u64 Array_Hash64_sint8 (const s8 array[], u32 size);
u64 Array_Hash64_sint16 (const s16 array[], u32 size);
u64 Array_Hash64_sint32 (const s32 array[], u32 size);
u64 Array_Hash64_sint64 (const s64 array[], u32 size);

// Floating-point types
u64 Array_Hash64_flt32 (const f32 array[], u32 size);
u64 Array_Hash64_flt64 (const f64 array[], u32 size);

// Other types
u64 Array_Hash64_size (const u32 array[], u32 size);
u64 Array_Hash64 (const void *array, u32 size);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
