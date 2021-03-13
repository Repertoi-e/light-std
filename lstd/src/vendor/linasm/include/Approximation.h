/*                                                               Approximation.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                 APPROXIMATION ALGORITHMS FOR EMPIRICAL DATA                  #
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
class Approximation
{
public:

//****************************************************************************//
//      Approximation by power function                                       //
//****************************************************************************//
static bool Power (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Power (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

//****************************************************************************//
//      Approximation by elementary functions                                 //
//****************************************************************************//

// Linear approximation
static bool Linear (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Linear (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Square law approximation
static bool Square (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Square (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Cube law approximation
static bool Cube (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Cube (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic approximation
static bool Hyperbolic (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Hyperbolic (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse square law approximation
static bool InverseSquare (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool InverseSquare (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse cube law approximation
static bool InverseCube (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool InverseCube (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Square root approximation
static bool Sqrt (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Sqrt (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Exponential approximation
static bool Exp (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Exp (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Logarithmic approximation
static bool Log (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Log (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic sine approximation
static bool SinH (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool SinH (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic cosine approximation
static bool CosH (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool CosH (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse hyperbolic sine approximation
static bool ArcSinH (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool ArcSinH (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse hyperbolic cosine approximation
static bool ArcCosH (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
static bool ArcCosH (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

//****************************************************************************//
//      Approximation by polynomial functions                                 //
//****************************************************************************//

// Polynomial approximation
static bool Polynomial (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Polynomial (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Square root approximation
static bool PolySqrt (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool PolySqrt (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Exponential approximation
static bool PolyExp (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool PolyExp (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Logarithmic approximation
static bool PolyLog (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool PolyLog (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic sine approximation
static bool PolySinH (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool PolySinH (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic cosine approximation
static bool PolyCosH (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool PolyCosH (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse hyperbolic sine approximation
static bool PolyArcSinH (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool PolyArcSinH (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse hyperbolic cosine approximation
static bool PolyArcCosH (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool PolyArcCosH (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

//****************************************************************************//
//      Approximation by rational functions                                   //
//****************************************************************************//

// Rational function approximation
static bool Rational (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Rational (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Subrational function approximation
static bool Subrational (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
static bool Subrational (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Approximation by power function                                       //
//****************************************************************************//
bool Approximation_Power_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Power_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

//****************************************************************************//
//      Approximation by elementary functions                                 //
//****************************************************************************//

// Linear approximation
bool Approximation_Linear_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Linear_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Square law approximation
bool Approximation_Square_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Square_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Cube law approximation
bool Approximation_Cube_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Cube_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic approximation
bool Approximation_Hyperbolic_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Hyperbolic_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse square law approximation
bool Approximation_InverseSquare_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_InverseSquare_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse cube law approximation
bool Approximation_InverseCube_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_InverseCube_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Square root approximation
bool Approximation_Sqrt_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Sqrt_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Exponential approximation
bool Approximation_Exp_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Exp_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Logarithmic approximation
bool Approximation_Log_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Log_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic sine approximation
bool Approximation_SinH_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_SinH_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic cosine approximation
bool Approximation_CosH_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_CosH_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse hyperbolic sine approximation
bool Approximation_ArcSinH_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_ArcSinH_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse hyperbolic cosine approximation
bool Approximation_ArcCosH_flt32 (f32 coeff[], f32 matrix[], f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_ArcCosH_flt64 (f64 coeff[], f64 matrix[], f64 args[], f64 vals[], f64 temp[], u32 size);

//****************************************************************************//
//      Approximation by polynomial functions                                 //
//****************************************************************************//

// Polynomial approximation
bool Approximation_Polynomial_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Polynomial_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Square root approximation
bool Approximation_PolySqrt_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_PolySqrt_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Exponential approximation
bool Approximation_PolyExp_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_PolyExp_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Logarithmic approximation
bool Approximation_PolyLog_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_PolyLog_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic sine approximation
bool Approximation_PolySinH_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_PolySinH_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Hyperbolic cosine approximation
bool Approximation_PolyCosH_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_PolyCosH_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse hyperbolic sine approximation
bool Approximation_PolyArcSinH_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_PolyArcSinH_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Inverse hyperbolic cosine approximation
bool Approximation_PolyArcCosH_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_PolyArcCosH_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

//****************************************************************************//
//      Approximation by rational functions                                   //
//****************************************************************************//

// Rational function approximation
bool Approximation_Rational_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Rational_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

// Subrational function approximation
bool Approximation_Subrational_flt32 (f32 coeff[], f32 matrix[], u32 degree, f32 args[], f32 vals[], f32 temp[], u32 size);
bool Approximation_Subrational_flt64 (f64 coeff[], f64 matrix[], u32 degree, f64 args[], f64 vals[], f64 temp[], u32 size);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
