/*                                                                      Filter.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                   FINITE IMPULSE RESPONSE DIGITAL FILTERS                    #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Types.h>

//****************************************************************************//
//      Frequency range                                                       //
//****************************************************************************//
# define	FREQ_MIN	0.0				// Min frequency
# define	FREQ_MAX	0.5				// Max frequency

//****************************************************************************//
//      Window functions                                                      //
//****************************************************************************//
enum window_t
{
	// No window functions
	WIN_NONE,							// Rectangle window (or no window)

	// High-resolution window functions
	WIN_SINE,							// Sine window
	WIN_HAMMING,						// Hamming window
	WIN_BLACKMAN,						// Blackman window

	// Low-resolution window functions
	WIN_BLACKMAN_NUTTALL				// Blackman-Nuttall window
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
class Filter
{
public:

//****************************************************************************//
//      Linear filters                                                        //
//****************************************************************************//

//============================================================================//
//      Band-pass filter                                                      //
//============================================================================//
static bool BandPass (f32 filter[], u32 size, f32 lowfreq, f32 highfreq, window_t window);
static bool BandPass (f64 filter[], u32 size, f64 lowfreq, f64 highfreq, window_t window);

//============================================================================//
//      Band-stop filter                                                      //
//============================================================================//
static bool BandStop (f32 filter[], u32 size, f32 lowfreq, f32 highfreq, window_t window);
static bool BandStop (f64 filter[], u32 size, f64 lowfreq, f64 highfreq, window_t window);

//============================================================================//
//      Hilbert filter                                                        //
//============================================================================//
static bool Hilbert (f32 filter[], u32 size, f32 lowfreq, f32 highfreq, window_t window);
static bool Hilbert (f64 filter[], u32 size, f64 lowfreq, f64 highfreq, window_t window);

//============================================================================//
//      Differential filter                                                   //
//============================================================================//
static bool Diff (f32 filter[], u32 size, f32 lowfreq, f32 highfreq, window_t window);
static bool Diff (f64 filter[], u32 size, f64 lowfreq, f64 highfreq, window_t window);

//============================================================================//
//      Moving average filter                                                 //
//============================================================================//
static bool Average (f32 response[], const f32 data[], u32 size, u32 order);
static bool Average (f64 response[], const f64 data[], u32 size, u32 order);

//****************************************************************************//
//      Nonlinear filters                                                     //
//****************************************************************************//

//============================================================================//
//      Median filter                                                         //
//============================================================================//

// Unsigned integer types
static bool Median (u8 response[], const u8 data[], u32 temp[], u32 size, u32 order);
static bool Median (u16 response[], const u16 data[], u32 temp[], u32 size, u32 order);
static bool Median (u32 response[], const u32 data[], u32 temp[], u32 size, u32 order);
static bool Median (u64 response[], const u64 data[], u32 temp[], u32 size, u32 order);

// Signed integer types
static bool Median (s8 response[], const s8 data[], u32 temp[], u32 size, u32 order);
static bool Median (s16 response[], const s16 data[], u32 temp[], u32 size, u32 order);
static bool Median (s32 response[], const s32 data[], u32 temp[], u32 size, u32 order);
static bool Median (s64 response[], const s64 data[], u32 temp[], u32 size, u32 order);

// Floating-point types
static bool Median (f32 response[], const f32 data[], u32 temp[], u32 size, u32 order);
static bool Median (f64 response[], const f64 data[], u32 temp[], u32 size, u32 order);

//============================================================================//
//      Min filter                                                            //
//============================================================================//

// Unsigned integer types
static bool Min (u8 response[], const u8 data[], u32 temp[], u32 size, u32 order);
static bool Min (u16 response[], const u16 data[], u32 temp[], u32 size, u32 order);
static bool Min (u32 response[], const u32 data[], u32 temp[], u32 size, u32 order);
static bool Min (u64 response[], const u64 data[], u32 temp[], u32 size, u32 order);

// Signed integer types
static bool Min (s8 response[], const s8 data[], u32 temp[], u32 size, u32 order);
static bool Min (s16 response[], const s16 data[], u32 temp[], u32 size, u32 order);
static bool Min (s32 response[], const s32 data[], u32 temp[], u32 size, u32 order);
static bool Min (s64 response[], const s64 data[], u32 temp[], u32 size, u32 order);

// Floating-point types
static bool Min (f32 response[], const f32 data[], u32 temp[], u32 size, u32 order);
static bool Min (f64 response[], const f64 data[], u32 temp[], u32 size, u32 order);

//============================================================================//
//      Max filter                                                            //
//============================================================================//

// Unsigned integer types
static bool Max (u8 response[], const u8 data[], u32 temp[], u32 size, u32 order);
static bool Max (u16 response[], const u16 data[], u32 temp[], u32 size, u32 order);
static bool Max (u32 response[], const u32 data[], u32 temp[], u32 size, u32 order);
static bool Max (u64 response[], const u64 data[], u32 temp[], u32 size, u32 order);

// Signed integer types
static bool Max (s8 response[], const s8 data[], u32 temp[], u32 size, u32 order);
static bool Max (s16 response[], const s16 data[], u32 temp[], u32 size, u32 order);
static bool Max (s32 response[], const s32 data[], u32 temp[], u32 size, u32 order);
static bool Max (s64 response[], const s64 data[], u32 temp[], u32 size, u32 order);

// Floating-point types
static bool Max (f32 response[], const f32 data[], u32 temp[], u32 size, u32 order);
static bool Max (f64 response[], const f64 data[], u32 temp[], u32 size, u32 order);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Linear filters                                                        //
//****************************************************************************//

//============================================================================//
//      Band-pass filter                                                      //
//============================================================================//
bool Filter_BandPass_flt32 (f32 filter[], u32 size, f32 lowfreq, f32 highfreq, enum window_t window);
bool Filter_BandPass_flt64 (f64 filter[], u32 size, f64 lowfreq, f64 highfreq, enum window_t window);

//============================================================================//
//      Band-stop filter                                                      //
//============================================================================//
bool Filter_BandStop_flt32 (f32 filter[], u32 size, f32 lowfreq, f32 highfreq, enum window_t window);
bool Filter_BandStop_flt64 (f64 filter[], u32 size, f64 lowfreq, f64 highfreq, enum window_t window);

//============================================================================//
//      Hilbert filter                                                        //
//============================================================================//
bool Filter_Hilbert_flt32 (f32 filter[], u32 size, f32 lowfreq, f32 highfreq, enum window_t window);
bool Filter_Hilbert_flt64 (f64 filter[], u32 size, f64 lowfreq, f64 highfreq, enum window_t window);

//============================================================================//
//      Differential filter                                                   //
//============================================================================//
bool Filter_Diff_flt32 (f32 filter[], u32 size, f32 lowfreq, f32 highfreq, enum window_t window);
bool Filter_Diff_flt64 (f64 filter[], u32 size, f64 lowfreq, f64 highfreq, enum window_t window);

//============================================================================//
//      Moving average filter                                                 //
//============================================================================//
bool Filter_Average_flt32 (f32 response[], const f32 data[], u32 size, u32 order);
bool Filter_Average_flt64 (f64 response[], const f64 data[], u32 size, u32 order);

//****************************************************************************//
//      Nonlinear filters                                                     //
//****************************************************************************//

//============================================================================//
//      Median filter                                                         //
//============================================================================//

// Unsigned integer types
bool Filter_Median_uint8 (u8 response[], const u8 data[], u32 temp[], u32 size, u32 order);
bool Filter_Median_uint16 (u16 response[], const u16 data[], u32 temp[], u32 size, u32 order);
bool Filter_Median_uint32 (u32 response[], const u32 data[], u32 temp[], u32 size, u32 order);
bool Filter_Median_uint64 (u64 response[], const u64 data[], u32 temp[], u32 size, u32 order);

// Signed integer types
bool Filter_Median_sint8 (s8 response[], const s8 data[], u32 temp[], u32 size, u32 order);
bool Filter_Median_sint16 (s16 response[], const s16 data[], u32 temp[], u32 size, u32 order);
bool Filter_Median_sint32 (s32 response[], const s32 data[], u32 temp[], u32 size, u32 order);
bool Filter_Median_sint64 (s64 response[], const s64 data[], u32 temp[], u32 size, u32 order);

// Floating-point types
bool Filter_Median_flt32 (f32 response[], const f32 data[], u32 temp[], u32 size, u32 order);
bool Filter_Median_flt64 (f64 response[], const f64 data[], u32 temp[], u32 size, u32 order);

//============================================================================//
//      Min filter                                                            //
//============================================================================//

// Unsigned integer types
bool Filter_Min_uint8 (u8 response[], const u8 data[], u32 temp[], u32 size, u32 order);
bool Filter_Min_uint16 (u16 response[], const u16 data[], u32 temp[], u32 size, u32 order);
bool Filter_Min_uint32 (u32 response[], const u32 data[], u32 temp[], u32 size, u32 order);
bool Filter_Min_uint64 (u64 response[], const u64 data[], u32 temp[], u32 size, u32 order);

// Signed integer types
bool Filter_Min_sint8 (s8 response[], const s8 data[], u32 temp[], u32 size, u32 order);
bool Filter_Min_sint16 (s16 response[], const s16 data[], u32 temp[], u32 size, u32 order);
bool Filter_Min_sint32 (s32 response[], const s32 data[], u32 temp[], u32 size, u32 order);
bool Filter_Min_sint64 (s64 response[], const s64 data[], u32 temp[], u32 size, u32 order);

// Floating-point types
bool Filter_Min_flt32 (f32 response[], const f32 data[], u32 temp[], u32 size, u32 order);
bool Filter_Min_flt64 (f64 response[], const f64 data[], u32 temp[], u32 size, u32 order);

//============================================================================//
//      Max filter                                                            //
//============================================================================//

// Unsigned integer types
bool Filter_Max_uint8 (u8 response[], const u8 data[], u32 temp[], u32 size, u32 order);
bool Filter_Max_uint16 (u16 response[], const u16 data[], u32 temp[], u32 size, u32 order);
bool Filter_Max_uint32 (u32 response[], const u32 data[], u32 temp[], u32 size, u32 order);
bool Filter_Max_uint64 (u64 response[], const u64 data[], u32 temp[], u32 size, u32 order);

// Signed integer types
bool Filter_Max_sint8 (s8 response[], const s8 data[], u32 temp[], u32 size, u32 order);
bool Filter_Max_sint16 (s16 response[], const s16 data[], u32 temp[], u32 size, u32 order);
bool Filter_Max_sint32 (s32 response[], const s32 data[], u32 temp[], u32 size, u32 order);
bool Filter_Max_sint64 (s64 response[], const s64 data[], u32 temp[], u32 size, u32 order);

// Floating-point types
bool Filter_Max_flt32 (f32 response[], const f32 data[], u32 temp[], u32 size, u32 order);
bool Filter_Max_flt64 (f64 response[], const f64 data[], u32 temp[], u32 size, u32 order);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
