/*                                                                       Angle.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                    ANGLES CONVERSION FROM DIFFERENT UNITS                    #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Types.h>
# include	<Math.h>

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
class Angle
{
public:

//****************************************************************************//
//      Conversion radians to degrees                                         //
//****************************************************************************//
inline static f32 RadToDeg (f32 angle)
{
	return angle * (static_cast <f32> (180.0) / static_cast <f32> (M_PI));
}
inline static f64 RadToDeg (f64 angle)
{
	return angle * (static_cast <f64> (180.0) / static_cast <f64> (M_PI));
}

//****************************************************************************//
//      Conversion radians to gradians                                        //
//****************************************************************************//
inline static f32 RadToGrad (f32 angle)
{
	return angle * (static_cast <f32> (200.0) / static_cast <f32> (M_PI));
}
inline static f64 RadToGrad (f64 angle)
{
	return angle * (static_cast <f64> (200.0) / static_cast <f64> (M_PI));
}

//****************************************************************************//
//      Conversion degrees to radians                                         //
//****************************************************************************//
inline static f32 DegToRad (f32 angle)
{
	return angle * (static_cast <f32> (M_PI) / static_cast <f32> (180.0));
}
inline static f64 DegToRad (f64 angle)
{
	return angle * (static_cast <f64> (M_PI) / static_cast <f64> (180.0));
}

//****************************************************************************//
//      Conversion degrees to gradians                                        //
//****************************************************************************//
inline static f32 DegToGrad (f32 angle)
{
	return angle * (static_cast <f32> (200.0) / static_cast <f32> (180.0));
}
inline static f64 DegToGrad (f64 angle)
{
	return angle * (static_cast <f64> (200.0) / static_cast <f64> (180.0));
}

//****************************************************************************//
//      Conversion gradians to radians                                        //
//****************************************************************************//
inline static f32 GradToRad (f32 angle)
{
	return angle * (static_cast <f32> (M_PI) / static_cast <f32> (200.0));
}
inline static f64 GradToRad (f64 angle)
{
	return angle * (static_cast <f64> (M_PI) / static_cast <f64> (200.0));
}

//****************************************************************************//
//      Conversion gradians to degrees                                        //
//****************************************************************************//
inline static f32 GradToDeg (f32 angle)
{
	return angle * (static_cast <f32> (180.0) / static_cast <f32> (200.0));
}
inline static f64 GradToDeg (f64 angle)
{
	return angle * (static_cast <f64> (180.0) / static_cast <f64> (200.0));
}
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Conversion radians to degrees                                         //
//****************************************************************************//
inline f32 Angle_RadToDeg_flt32 (f32 angle)
{
	return angle * ((f32) 180.0 / (f32) M_PI);
}
inline f64 Angle_RadToDeg_flt64 (f64 angle)
{
	return angle * ((f64) 180.0 / (f64) M_PI);
}

//****************************************************************************//
//      Conversion radians to gradians                                        //
//****************************************************************************//
inline f32 Angle_RadToGrad_flt32 (f32 angle)
{
	return angle * ((f32) 200.0 / (f32) M_PI);
}
inline f64 Angle_RadToGrad_flt64 (f64 angle)
{
	return angle * ((f64) 200.0 / (f64) M_PI);
}

//****************************************************************************//
//      Conversion degrees to radians                                         //
//****************************************************************************//
inline f32 Angle_DegToRad_flt32 (f32 angle)
{
	return angle * ((f32) M_PI / (f32) 180.0);
}
inline f64 Angle_DegToRad_flt64 (f64 angle)
{
	return angle * ((f64) M_PI / (f64) 180.0);
}

//****************************************************************************//
//      Conversion degrees to gradians                                        //
//****************************************************************************//
inline f32 Angle_DegToGrad_flt32 (f32 angle)
{
	return angle * ((f32) 200.0 / (f32) 180.0);
}
inline f64 Angle_DegToGrad_flt64 (f64 angle)
{
	return angle * ((f64) 200.0 / (f64) 180.0);
}

//****************************************************************************//
//      Conversion gradians to radians                                        //
//****************************************************************************//
inline f32 Angle_GradToRad_flt32 (f32 angle)
{
	return angle * ((f32) M_PI / (f32) 200.0);
}
inline f64 Angle_GradToRad_flt64 (f64 angle)
{
	return angle * ((f64) M_PI / (f64) 200.0);
}

//****************************************************************************//
//      Conversion gradians to degrees                                        //
//****************************************************************************//
inline f32 Angle_GradToDeg_flt32 (f32 angle)
{
	return angle * ((f32) 180.0 / (f32) 200.0);
}
inline f64 Angle_GradToDeg_flt64 (f64 angle)
{
	return angle * ((f64) 180.0 / (f64) 200.0);
}
# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
