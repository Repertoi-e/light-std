/*                                                                         FHT.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                         FAST HARTLEY TRANSFORMATION                          #
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
class FHT
{
public:

//****************************************************************************//
//      Sine and cosine table                                                 //
//****************************************************************************//
static void SinCosTable (f32 table[], u8 exp);
static void SinCosTable (f64 table[], u8 exp);

//****************************************************************************//
//      Hartley transformation                                                //
//****************************************************************************//

// Time domain to frequency domain
static void Spectrum (f32 array[], const f32 table[], u8 exp);
static void Spectrum (f64 array[], const f64 table[], u8 exp);

// Frequency domain to time domain
static void Impulse (f32 array[], const f32 table[], u8 exp);
static void Impulse (f64 array[], const f64 table[], u8 exp);

//****************************************************************************//
//      Spectrum                                                              //
//****************************************************************************//

// Energy spectrum
static void EnergySpectrum (f32 energy[], const f32 spectrum[], u8 exp);
static void EnergySpectrum (f64 energy[], const f64 spectrum[], u8 exp);

// Magnitude spectrum
static void MagnitudeSpectrum (f32 magnitude[], const f32 spectrum[], u8 exp);
static void MagnitudeSpectrum (f64 magnitude[], const f64 spectrum[], u8 exp);

// Real spectrum
static void RealSpectrum (f32 magnitude[], f32 phase[], const f32 spectrum[], u8 exp);
static void RealSpectrum (f64 magnitude[], f64 phase[], const f64 spectrum[], u8 exp);

//****************************************************************************//
//      Signal manipulations                                                  //
//****************************************************************************//

// Signal reflection
static void Reflect (f32 array[], u8 exp);
static void Reflect (f64 array[], u8 exp);

// Signal derivative
static void Derivative (f32 array[], u8 exp);
static void Derivative (f64 array[], u8 exp);

// Signal antiderivative
static void AntiDerivative (f32 array[], f32 shift, u8 exp);
static void AntiDerivative (f64 array[], f64 shift, u8 exp);

// Signal time and frequency shift
static void Shift (f32 array[], f32 shift, u8 exp);
static void Shift (f64 array[], f64 shift, u8 exp);

// Magnitude scaling
static void Scale (f32 array[], f32 scale, u8 exp);
static void Scale (f64 array[], f64 scale, u8 exp);

// Autocorrelation of signal
static void AutoCorrelation (f32 array[], f32 scale, u8 exp);
static void AutoCorrelation (f64 array[], f64 scale, u8 exp);

// Cross-correlation of signals
static void CrossCorrelation (f32 target[], const f32 source[], f32 scale, u8 exp);
static void CrossCorrelation (f64 target[], const f64 source[], f64 scale, u8 exp);

// Convolution of signals
static void Convolution (f32 target[], const f32 source[], f32 scale, u8 exp);
static void Convolution (f64 target[], const f64 source[], f64 scale, u8 exp);

// Mixing signals
static void Mix (f32 target[], const f32 source[], f32 tscale, f32 sscale, u8 exp);
static void Mix (f64 target[], const f64 source[], f64 tscale, f64 sscale, u8 exp);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Sine and cosine table                                                 //
//****************************************************************************//
void FHT_SinCosTable_flt32 (f32 table[], u8 exp);
void FHT_SinCosTable_flt64 (f64 table[], u8 exp);

//****************************************************************************//
//      Hartley transformation                                                //
//****************************************************************************//

// Time domain to frequency domain
void FHT_Spectrum_flt32 (f32 array[], const f32 table[], u8 exp);
void FHT_Spectrum_flt64 (f64 array[], const f64 table[], u8 exp);

// Frequency domain to time domain
void FHT_Impulse_flt32 (f32 array[], const f32 table[], u8 exp);
void FHT_Impulse_flt64 (f64 array[], const f64 table[], u8 exp);

//****************************************************************************//
//      Spectrum                                                              //
//****************************************************************************//

// Energy spectrum
void FHT_EnergySpectrum_flt32 (f32 energy[], const f32 spectrum[], u8 exp);
void FHT_EnergySpectrum_flt64 (f64 energy[], const f64 spectrum[], u8 exp);

// Magnitude spectrum
void FHT_MagnitudeSpectrum_flt32 (f32 magnitude[], const f32 spectrum[], u8 exp);
void FHT_MagnitudeSpectrum_flt64 (f64 magnitude[], const f64 spectrum[], u8 exp);

// Real spectrum
void FHT_RealSpectrum_flt32 (f32 magnitude[], f32 phase[], const f32 spectrum[], u8 exp);
void FHT_RealSpectrum_flt64 (f64 magnitude[], f64 phase[], const f64 spectrum[], u8 exp);

//****************************************************************************//
//      Signal manipulations                                                  //
//****************************************************************************//

// Signal reflection
void FHT_Reflect_flt32 (f32 array[], u8 exp);
void FHT_Reflect_flt64 (f64 array[], u8 exp);

// Signal derivative
void FHT_Derivative_flt32 (f32 array[], u8 exp);
void FHT_Derivative_flt64 (f64 array[], u8 exp);

// Signal antiderivative
void FHT_AntiDerivative_flt32 (f32 array[], f32 shift, u8 exp);
void FHT_AntiDerivative_flt64 (f64 array[], f64 shift, u8 exp);

// Signal time and frequency shift
void FHT_Shift_flt32 (f32 array[], f32 shift, u8 exp);
void FHT_Shift_flt64 (f64 array[], f64 shift, u8 exp);

// Magnitude scaling
void FHT_Scale_flt32 (f32 array[], f32 scale, u8 exp);
void FHT_Scale_flt64 (f64 array[], f64 scale, u8 exp);

// Autocorrelation of signal
void FHT_AutoCorrelation_flt32 (f32 array[], f32 scale, u8 exp);
void FHT_AutoCorrelation_flt64 (f64 array[], f64 scale, u8 exp);

// Cross-correlation of signals
void FHT_CrossCorrelation_flt32 (f32 target[], const f32 source[], f32 scale, u8 exp);
void FHT_CrossCorrelation_flt64 (f64 target[], const f64 source[], f64 scale, u8 exp);

// Convolution of signals
void FHT_Convolution_flt32 (f32 target[], const f32 source[], f32 scale, u8 exp);
void FHT_Convolution_flt64 (f64 target[], const f64 source[], f64 scale, u8 exp);

// Mixing signals
void FHT_Mix_flt32 (f32 target[], const f32 source[], f32 tscale, f32 sscale, u8 exp);
void FHT_Mix_flt64 (f64 target[], const f64 source[], f64 tscale, f64 sscale, u8 exp);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
