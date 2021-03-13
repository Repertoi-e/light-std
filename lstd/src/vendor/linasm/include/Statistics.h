/*                                                                  Statistics.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                      WIDELY USED STATISTICAL FUNCTIONS                       #
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
class Statistics
{
public:

//****************************************************************************//
//      Measures of location                                                  //
//****************************************************************************//

//============================================================================//
//      Mean                                                                  //
//============================================================================//
static f32 Mean (const f32 array[], u32 size);
static f64 Mean (const f64 array[], u32 size);

//============================================================================//
//      Median                                                                //
//============================================================================//

// Unsigned integer types
static u8 Median (u8 array[], u32 size);
static u16 Median (u16 array[], u32 size);
static u32 Median (u32 array[], u32 size);
static u64 Median (u64 array[], u32 size);

// Signed integer types
static s8 Median (s8 array[], u32 size);
static s16 Median (s16 array[], u32 size);
static s32 Median (s32 array[], u32 size);
static s64 Median (s64 array[], u32 size);

// Floating-point types
static f32 Median (f32 array[], u32 size);
static f64 Median (f64 array[], u32 size);

// Other types
static u32 Median (u32 array[], u32 size);

//============================================================================//
//      Lower quartile                                                        //
//============================================================================//

// Unsigned integer types
static u8 LowerQuartile (u8 array[], u32 size);
static u16 LowerQuartile (u16 array[], u32 size);
static u32 LowerQuartile (u32 array[], u32 size);
static u64 LowerQuartile (u64 array[], u32 size);

// Signed integer types
static s8 LowerQuartile (s8 array[], u32 size);
static s16 LowerQuartile (s16 array[], u32 size);
static s32 LowerQuartile (s32 array[], u32 size);
static s64 LowerQuartile (s64 array[], u32 size);

// Floating-point types
static f32 LowerQuartile (f32 array[], u32 size);
static f64 LowerQuartile (f64 array[], u32 size);

// Other types
static u32 LowerQuartile (u32 array[], u32 size);

//============================================================================//
//      Upper quartile                                                        //
//============================================================================//

// Unsigned integer types
static u8 UpperQuartile (u8 array[], u32 size);
static u16 UpperQuartile (u16 array[], u32 size);
static u32 UpperQuartile (u32 array[], u32 size);
static u64 UpperQuartile (u64 array[], u32 size);

// Signed integer types
static s8 UpperQuartile (s8 array[], u32 size);
static s16 UpperQuartile (s16 array[], u32 size);
static s32 UpperQuartile (s32 array[], u32 size);
static s64 UpperQuartile (s64 array[], u32 size);

// Floating-point types
static f32 UpperQuartile (f32 array[], u32 size);
static f64 UpperQuartile (f64 array[], u32 size);

// Other types
static u32 UpperQuartile (u32 array[], u32 size);

//============================================================================//
//      Mid-range                                                             //
//============================================================================//
static f32 Midrange (const f32 array[], u32 size);
static f64 Midrange (const f64 array[], u32 size);

//****************************************************************************//
//      Measures of variability                                               //
//****************************************************************************//

//============================================================================//
//      Variance                                                              //
//============================================================================//
static f32 Variance (const f32 array[], u32 size, f32 mean);
static f64 Variance (const f64 array[], u32 size, f64 mean);

//============================================================================//
//      Standard deviation                                                    //
//============================================================================//
static f32 StandardDeviation (const f32 array[], u32 size, f32 mean);
static f64 StandardDeviation (const f64 array[], u32 size, f64 mean);

//============================================================================//
//      Absolute deviation                                                    //
//============================================================================//
static f32 AbsoluteDeviation (const f32 array[], u32 size, f32 mean);
static f64 AbsoluteDeviation (const f64 array[], u32 size, f64 mean);

//============================================================================//
//      Interquartile range                                                   //
//============================================================================//

// Unsigned integer types
static u8 InterquartileRange (u8 array[], u32 size);
static u16 InterquartileRange (u16 array[], u32 size);
static u32 InterquartileRange (u32 array[], u32 size);
static u64 InterquartileRange (u64 array[], u32 size);

// Signed integer types
static u8 InterquartileRange (s8 array[], u32 size);
static u16 InterquartileRange (s16 array[], u32 size);
static u32 InterquartileRange (s32 array[], u32 size);
static u64 InterquartileRange (s64 array[], u32 size);

// Floating-point types
static f32 InterquartileRange (f32 array[], u32 size);
static f64 InterquartileRange (f64 array[], u32 size);

// Other types
static u32 InterquartileRange (u32 array[], u32 size);

//============================================================================//
//      Range                                                                 //
//============================================================================//
static f32 Range (const f32 array[], u32 size);
static f64 Range (const f64 array[], u32 size);

//****************************************************************************//
//      Measures of shape                                                     //
//****************************************************************************//

// Skewness
static f32 Skewness (const f32 array[], u32 size, f32 mean);
static f64 Skewness (const f64 array[], u32 size, f64 mean);

// Kurtosis
static f32 Kurtosis (const f32 array[], u32 size, f32 mean);
static f64 Kurtosis (const f64 array[], u32 size, f64 mean);

//****************************************************************************//
//      Covariance                                                            //
//****************************************************************************//
static f32 Covariance (const f32 arr1[], const f32 arr2[], u32 size, f32 mean1, f32 mean2);
static f64 Covariance (const f64 arr1[], const f64 arr2[], u32 size, f64 mean1, f64 mean2);

//****************************************************************************//
//      Correlation                                                           //
//****************************************************************************//

// Pearson correlation
static f32 PearsonCorrelation (const f32 arr1[], const f32 arr2[], u32 size, f32 mean1, f32 mean2);
static f64 PearsonCorrelation (const f64 arr1[], const f64 arr2[], u32 size, f64 mean1, f64 mean2);

// Fechner correlation
static f32 FechnerCorrelation (const f32 arr1[], const f32 arr2[], u32 size, f32 mean1, f32 mean2);
static f64 FechnerCorrelation (const f64 arr1[], const f64 arr2[], u32 size, f64 mean1, f64 mean2);

// Spearman correlation
static f32 SpearmanCorrelation (f32 arr1[], u32 rank1[], f32 arr2[], u32 rank2[], f32 tarr[], u32 trank[], u32 size);
static f64 SpearmanCorrelation (f64 arr1[], u32 rank1[], f64 arr2[], u32 rank2[], f64 tarr[], u32 trank[], u32 size);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Measures of location                                                  //
//****************************************************************************//

//============================================================================//
//      Mean                                                                  //
//============================================================================//
f32 Statistics_Mean_flt32 (const f32 array[], u32 size);
f64 Statistics_Mean_flt64 (const f64 array[], u32 size);

//============================================================================//
//      Median                                                                //
//============================================================================//

// Unsigned integer types
u8 Statistics_Median_uint8 (u8 array[], u32 size);
u16 Statistics_Median_uint16 (u16 array[], u32 size);
u32 Statistics_Median_uint32 (u32 array[], u32 size);
u64 Statistics_Median_uint64 (u64 array[], u32 size);

// Signed integer types
s8 Statistics_Median_sint8 (s8 array[], u32 size);
s16 Statistics_Median_sint16 (s16 array[], u32 size);
s32 Statistics_Median_sint32 (s32 array[], u32 size);
s64 Statistics_Median_sint64 (s64 array[], u32 size);

// Floating-point types
f32 Statistics_Median_flt32 (f32 array[], u32 size);
f64 Statistics_Median_flt64 (f64 array[], u32 size);

// Other types
u32 Statistics_Median_size (u32 array[], u32 size);

//============================================================================//
//      Lower quartile                                                        //
//============================================================================//

// Unsigned integer types
u8 Statistics_LowerQuartile_uint8 (u8 array[], u32 size);
u16 Statistics_LowerQuartile_uint16 (u16 array[], u32 size);
u32 Statistics_LowerQuartile_uint32 (u32 array[], u32 size);
u64 Statistics_LowerQuartile_uint64 (u64 array[], u32 size);

// Signed integer types
s8 Statistics_LowerQuartile_sint8 (s8 array[], u32 size);
s16 Statistics_LowerQuartile_sint16 (s16 array[], u32 size);
s32 Statistics_LowerQuartile_sint32 (s32 array[], u32 size);
s64 Statistics_LowerQuartile_sint64 (s64 array[], u32 size);

// Floating-point types
f32 Statistics_LowerQuartile_flt32 (f32 array[], u32 size);
f64 Statistics_LowerQuartile_flt64 (f64 array[], u32 size);

// Other types
u32 Statistics_LowerQuartile_size (u32 array[], u32 size);

//============================================================================//
//      Upper quartile                                                        //
//============================================================================//

// Unsigned integer types
u8 Statistics_UpperQuartile_uint8 (u8 array[], u32 size);
u16 Statistics_UpperQuartile_uint16 (u16 array[], u32 size);
u32 Statistics_UpperQuartile_uint32 (u32 array[], u32 size);
u64 Statistics_UpperQuartile_uint64 (u64 array[], u32 size);

// Signed integer types
s8 Statistics_UpperQuartile_sint8 (s8 array[], u32 size);
s16 Statistics_UpperQuartile_sint16 (s16 array[], u32 size);
s32 Statistics_UpperQuartile_sint32 (s32 array[], u32 size);
s64 Statistics_UpperQuartile_sint64 (s64 array[], u32 size);

// Floating-point types
f32 Statistics_UpperQuartile_flt32 (f32 array[], u32 size);
f64 Statistics_UpperQuartile_flt64 (f64 array[], u32 size);

// Other types
u32 Statistics_UpperQuartile_size (u32 array[], u32 size);

//============================================================================//
//      Mid-range                                                             //
//============================================================================//
f32 Statistics_Midrange_flt32 (const f32 array[], u32 size);
f64 Statistics_Midrange_flt64 (const f64 array[], u32 size);

//****************************************************************************//
//      Measures of variability                                               //
//****************************************************************************//

//============================================================================//
//      Variance                                                              //
//============================================================================//
f32 Statistics_Variance_flt32 (const f32 array[], u32 size, f32 mean);
f64 Statistics_Variance_flt64 (const f64 array[], u32 size, f64 mean);

//============================================================================//
//      Standard deviation                                                    //
//============================================================================//
f32 Statistics_StandardDeviation_flt32 (const f32 array[], u32 size, f32 mean);
f64 Statistics_StandardDeviation_flt64 (const f64 array[], u32 size, f64 mean);

//============================================================================//
//      Absolute deviation                                                    //
//============================================================================//
f32 Statistics_AbsoluteDeviation_flt32 (const f32 array[], u32 size, f32 mean);
f64 Statistics_AbsoluteDeviation_flt64 (const f64 array[], u32 size, f64 mean);

//============================================================================//
//      Interquartile range                                                   //
//============================================================================//

// Unsigned integer types
u8 Statistics_InterquartileRange_uint8 (u8 array[], u32 size);
u16 Statistics_InterquartileRange_uint16 (u16 array[], u32 size);
u32 Statistics_InterquartileRange_uint32 (u32 array[], u32 size);
u64 Statistics_InterquartileRange_uint64 (u64 array[], u32 size);

// Signed integer types
u8 Statistics_InterquartileRange_sint8 (s8 array[], u32 size);
u16 Statistics_InterquartileRange_sint16 (s16 array[], u32 size);
u32 Statistics_InterquartileRange_sint32 (s32 array[], u32 size);
u64 Statistics_InterquartileRange_sint64 (s64 array[], u32 size);

// Floating-point types
f32 Statistics_InterquartileRange_flt32 (f32 array[], u32 size);
f64 Statistics_InterquartileRange_flt64 (f64 array[], u32 size);

// Other types
u32 Statistics_InterquartileRange_size (u32 array[], u32 size);

//============================================================================//
//      Range                                                                 //
//============================================================================//
f32 Statistics_Range_flt32 (const f32 array[], u32 size);
f64 Statistics_Range_flt64 (const f64 array[], u32 size);

//****************************************************************************//
//      Measures of shape                                                     //
//****************************************************************************//

// Skewness
f32 Statistics_Skewness_flt32 (const f32 array[], u32 size, f32 mean);
f64 Statistics_Skewness_flt64 (const f64 array[], u32 size, f64 mean);

// Kurtosis
f32 Statistics_Kurtosis_flt32 (const f32 array[], u32 size, f32 mean);
f64 Statistics_Kurtosis_flt64 (const f64 array[], u32 size, f64 mean);

//****************************************************************************//
//      Covariance                                                            //
//****************************************************************************//
f32 Statistics_Covariance_flt32 (const f32 arr1[], const f32 arr2[], u32 size, f32 mean1, f32 mean2);
f64 Statistics_Covariance_flt64 (const f64 arr1[], const f64 arr2[], u32 size, f64 mean1, f64 mean2);

//****************************************************************************//
//      Correlation                                                           //
//****************************************************************************//

// Pearson correlation
f32 Statistics_PearsonCorrelation_flt32 (const f32 arr1[], const f32 arr2[], u32 size, f32 mean1, f32 mean2);
f64 Statistics_PearsonCorrelation_flt64 (const f64 arr1[], const f64 arr2[], u32 size, f64 mean1, f64 mean2);

// Fechner correlation
f32 Statistics_FechnerCorrelation_flt32 (const f32 arr1[], const f32 arr2[], u32 size, f32 mean1, f32 mean2);
f64 Statistics_FechnerCorrelation_flt64 (const f64 arr1[], const f64 arr2[], u32 size, f64 mean1, f64 mean2);

// Spearman correlation
f32 Statistics_SpearmanCorrelation_flt32 (f32 arr1[], u32 rank1[], f32 arr2[], u32 rank2[], f32 tarr[], u32 trank[], u32 size);
f64 Statistics_SpearmanCorrelation_flt64 (f64 arr1[], u32 rank1[], f64 arr2[], u32 rank2[], f64 tarr[], u32 trank[], u32 size);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
