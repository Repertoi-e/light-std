/*                                                                      Window.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                WINDOW FUNCTIONS FOR DIGITAL SIGNAL PROCESSING                #
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
class Window
{
public:

//****************************************************************************//
//      High-resolution windows (low dynamic range)                           //
//****************************************************************************//

// Sine window
static void Sine (f32 window[], u32 size);
static void Sine (f64 window[], u32 size);

// Hamming window
static void Hamming (f32 window[], u32 size);
static void Hamming (f64 window[], u32 size);

// Blackman window
static void Blackman (f32 window[], u32 size);
static void Blackman (f64 window[], u32 size);

//****************************************************************************//
//      Low-resolution windows (high dynamic range)                           //
//****************************************************************************//

// Blackman-Nuttall window
static void BlackmanNuttall (f32 window[], u32 size);
static void BlackmanNuttall (f64 window[], u32 size);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      High-resolution windows (low dynamic range)                           //
//****************************************************************************//

// Sine window
void Window_Sine_flt32 (f32 window[], u32 size);
void Window_Sine_flt64 (f64 window[], u32 size);

// Hamming window
void Window_Hamming_flt32 (f32 window[], u32 size);
void Window_Hamming_flt64 (f64 window[], u32 size);

// Blackman window
void Window_Blackman_flt32 (f32 window[], u32 size);
void Window_Blackman_flt64 (f64 window[], u32 size);

//****************************************************************************//
//      Low-resolution windows (high dynamic range)                           //
//****************************************************************************//

// Blackman-Nuttall window
void Window_BlackmanNuttall_flt32 (f32 window[], u32 size);
void Window_BlackmanNuttall_flt64 (f64 window[], u32 size);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
