/*                                                                     Complex.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                           COMPLEX NUMBER FUNCTIONS                           #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Types.h>

//****************************************************************************//
//      Complex number structure (32-bit)                                     //
//****************************************************************************//
struct cmplx32_t
{
	f32	re;		// Real part
	f32	im;		// Imaginary part
};

//****************************************************************************//
//      Complex number structure (64-bit)                                     //
//****************************************************************************//
struct cmplx64_t
{
	f64	re;		// Real part
	f64	im;		// Imaginary part
};

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
class Complex
{
public:

//****************************************************************************//
//      Arithmetic operations                                                 //
//****************************************************************************//

//============================================================================//
//      Unary operations                                                      //
//============================================================================//

// Negative value
static void Neg (cmplx32_t *number);
static void Neg (cmplx64_t *number);

// Complex conjugate value
static void Conj (cmplx32_t *number);
static void Conj (cmplx64_t *number);

// Square root
static void Sqrt (cmplx32_t *number);
static void Sqrt (cmplx64_t *number);

// Square value
static void Sqr (cmplx32_t *number);
static void Sqr (cmplx64_t *number);

// Inverse value
static void Inverse (cmplx32_t *number);
static void Inverse (cmplx64_t *number);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

// Addition
static void Add (cmplx32_t *target, const cmplx32_t *source);
static void Add (cmplx64_t *target, const cmplx64_t *source);

// Subtraction
static void Sub (cmplx32_t *target, const cmplx32_t *source);
static void Sub (cmplx64_t *target, const cmplx64_t *source);

// Multiplication
static void Mul (cmplx32_t *target, const cmplx32_t *source);
static void Mul (cmplx64_t *target, const cmplx64_t *source);

// Division
static void Div (cmplx32_t *target, const cmplx32_t *source);
static void Div (cmplx64_t *target, const cmplx64_t *source);

//****************************************************************************//
//      Complex number properties                                             //
//****************************************************************************//

// Magnitude value
static f32 Magnitude (const cmplx32_t *number);
static f64 Magnitude (const cmplx64_t *number);

// Argument value
static f32 Argument (const cmplx32_t *number);
static f64 Argument (const cmplx64_t *number);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

// Check for zero number
static bool IsZero (const cmplx32_t *number);
static bool IsZero (const cmplx64_t *number);

// Check for equality of the numbers
static bool IsEqual (const cmplx32_t *target, const cmplx32_t *source);
static bool IsEqual (const cmplx64_t *target, const cmplx64_t *source);

// Check for negativity of the numbers
static bool IsNeg (const cmplx32_t *target, const cmplx32_t *source);
static bool IsNeg (const cmplx64_t *target, const cmplx64_t *source);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Arithmetic operations                                                 //
//****************************************************************************//

//============================================================================//
//      Unary operations                                                      //
//============================================================================//

// Negative value
void Complex_Neg_flt32 (struct cmplx32_t *number);
void Complex_Neg_flt64 (struct cmplx64_t *number);

// Complex conjugate value
void Complex_Conj_flt32 (struct cmplx32_t *number);
void Complex_Conj_flt64 (struct cmplx64_t *number);

// Square root
void Complex_Sqrt_flt32 (struct cmplx32_t *number);
void Complex_Sqrt_flt64 (struct cmplx64_t *number);

// Square value
void Complex_Sqr_flt32 (struct cmplx32_t *number);
void Complex_Sqr_flt64 (struct cmplx64_t *number);

// Inverse value
void Complex_Inverse_flt32 (struct cmplx32_t *number);
void Complex_Inverse_flt64 (struct cmplx64_t *number);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

// Addition
void Complex_Add_flt32 (struct cmplx32_t *target, const struct cmplx32_t *source);
void Complex_Add_flt64 (struct cmplx64_t *target, const struct cmplx64_t *source);

// Subtraction
void Complex_Sub_flt32 (struct cmplx32_t *target, const struct cmplx32_t *source);
void Complex_Sub_flt64 (struct cmplx64_t *target, const struct cmplx64_t *source);

// Multiplication
void Complex_Mul_flt32 (struct cmplx32_t *target, const struct cmplx32_t *source);
void Complex_Mul_flt64 (struct cmplx64_t *target, const struct cmplx64_t *source);

// Division
void Complex_Div_flt32 (struct cmplx32_t *target, const struct cmplx32_t *source);
void Complex_Div_flt64 (struct cmplx64_t *target, const struct cmplx64_t *source);

//****************************************************************************//
//      Complex number properties                                             //
//****************************************************************************//

// Magnitude value
f32 Complex_Magnitude_flt32 (const struct cmplx32_t *number);
f64 Complex_Magnitude_flt64 (const struct cmplx64_t *number);

// Argument value
f32 Complex_Argument_flt32 (const struct cmplx32_t *number);
f64 Complex_Argument_flt64 (const struct cmplx64_t *number);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

// Check for zero number
bool Complex_IsZero_flt32 (const struct cmplx32_t *number);
bool Complex_IsZero_flt64 (const struct cmplx64_t *number);

// Check for equality of the numbers
bool Complex_IsEqual_flt32 (const struct cmplx32_t *target, const struct cmplx32_t *source);
bool Complex_IsEqual_flt64 (const struct cmplx64_t *target, const struct cmplx64_t *source);

// Check for negativity of the numbers
bool Complex_IsNeg_flt32 (const struct cmplx32_t *target, const struct cmplx32_t *source);
bool Complex_IsNeg_flt64 (const struct cmplx64_t *target, const struct cmplx64_t *source);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
