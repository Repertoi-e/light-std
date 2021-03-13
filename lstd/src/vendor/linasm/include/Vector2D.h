/*                                                                    Vector2D.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                     FUNCTIONS TO OPERATE WITH 2D VECTORS                     #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Types.h>

//****************************************************************************//
//      2D vector structure (32-bit)                                          //
//****************************************************************************//
struct v2D32_t
{
	f32	x;		// X coordinate
	f32	y;		// Y coordinate
};

//****************************************************************************//
//      2D vector structure (64-bit)                                          //
//****************************************************************************//
struct v2D64_t
{
	f64	x;		// X coordinate
	f64	y;		// Y coordinate
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
class Vector2D
{
public:

//****************************************************************************//
//      Arithmetic operations                                                 //
//****************************************************************************//

//============================================================================//
//      Unary operations                                                      //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Normalization (direction cosines)                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void Normalize (v2D32_t *vector);
static void Normalize (v2D64_t *vector);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Reflection of vector                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Reflection through the origin
static void ReflectOrigin (v2D32_t *vector);
static void ReflectOrigin (v2D64_t *vector);

// Reflection through the X axis
static void ReflectX (v2D32_t *vector);
static void ReflectX (v2D64_t *vector);

// Reflection through the Y axis
static void ReflectY (v2D32_t *vector);
static void ReflectY (v2D64_t *vector);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

// Addition of vectors
static void Add (v2D32_t *target, const v2D32_t *source);
static void Add (v2D64_t *target, const v2D64_t *source);

// Subtraction of vectors
static void Sub (v2D32_t *target, const v2D32_t *source);
static void Sub (v2D64_t *target, const v2D64_t *source);

// Multiplication by scalar value
static void Mul (v2D32_t *vector, f32 value);
static void Mul (v2D64_t *vector, f64 value);

// Division by scalar value
static void Div (v2D32_t *vector, f32 value);
static void Div (v2D64_t *vector, f64 value);

//****************************************************************************//
//      Rotation of vector                                                    //
//****************************************************************************//
static void Rotate (v2D32_t *vector, f32 cos, f32 sin);
static void Rotate (v2D64_t *vector, f64 cos, f64 sin;

//****************************************************************************//
//      Shearing of vector                                                    //
//****************************************************************************//

// Shearing parallel to the X axis
static void ShearX (v2D32_t *vector, f32 value);
static void ShearX (v2D64_t *vector, f64 value);

// Shearing parallel to the Y axis
static void ShearY (v2D32_t *vector, f32 value);
static void ShearY (v2D64_t *vector, f64 value);

//****************************************************************************//
//      Scaling of vector                                                     //
//****************************************************************************//
static void Scale (v2D32_t *target, const v2D32_t *source);
static void Scale (v2D64_t *target, const v2D64_t *source);

//****************************************************************************//
//      Scalar product                                                        //
//****************************************************************************//
static f32 ScalarProduct (const v2D32_t *target, const v2D32_t *source);
static f64 ScalarProduct (const v2D64_t *target, const v2D64_t *source);

//****************************************************************************//
//      Absolute value                                                        //
//****************************************************************************//
static f32 Abs (const v2D32_t *vector);
static f64 Abs (const v2D64_t *vector);

//****************************************************************************//
//      Cosine value of angle between the vectors                             //
//****************************************************************************//
static f32 Cos (const v2D32_t *target, const v2D32_t *source);
static f64 Cos (const v2D64_t *target, const v2D64_t *source);

//****************************************************************************//
//      Projection of the vector to another vector                            //
//****************************************************************************//
static f32 Projection (const v2D32_t *target, const v2D32_t *source);
static f64 Projection (const v2D64_t *target, const v2D64_t *source);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

// Check for zero vector
static bool IsZero (const v2D32_t *vector);
static bool IsZero (const v2D64_t *vector);

// Check for equality of the vectors
static bool IsEqual (const v2D32_t *target, const v2D32_t *source);
static bool IsEqual (const v2D64_t *target, const v2D64_t *source);

// Check for negativity of the vectors
static bool IsNeg (const v2D32_t *target, const v2D32_t *source);
static bool IsNeg (const v2D64_t *target, const v2D64_t *source);

// Check for collinearity of the vectors
static bool IsCollinear (const v2D32_t *target, const v2D32_t *source);
static bool IsCollinear (const v2D64_t *target, const v2D64_t *source);

// Check for orthogonality of the vectors
static bool IsOrthogonal (const v2D32_t *target, const v2D32_t *source);
static bool IsOrthogonal (const v2D64_t *target, const v2D64_t *source);
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Normalization (direction cosines)                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void Vector2D_Normalize_flt32 (struct v2D32_t *vector);
void Vector2D_Normalize_flt64 (struct v2D64_t *vector);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Reflection of vector                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Reflection through the origin
void Vector2D_ReflectOrigin_flt32 (struct v2D32_t *vector);
void Vector2D_ReflectOrigin_flt64 (struct v2D64_t *vector);

// Reflection through the X axis
void Vector2D_ReflectX_flt32 (struct v2D32_t *vector);
void Vector2D_ReflectX_flt64 (struct v2D64_t *vector);

// Reflection through the Y axis
void Vector2D_ReflectY_flt32 (struct v2D32_t *vector);
void Vector2D_ReflectY_flt64 (struct v2D64_t *vector);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

// Addition of vectors
void Vector2D_Add_flt32 (struct v2D32_t *target, const struct v2D32_t *source);
void Vector2D_Add_flt64 (struct v2D64_t *target, const struct v2D64_t *source);

// Subtraction of vectors
void Vector2D_Sub_flt32 (struct v2D32_t *target, const struct v2D32_t *source);
void Vector2D_Sub_flt64 (struct v2D64_t *target, const struct v2D64_t *source);

// Multiplication by scalar value
void Vector2D_Mul_flt32 (struct v2D32_t *vector, f32 value);
void Vector2D_Mul_flt64 (struct v2D64_t *vector, f64 value);

// Division by scalar value
void Vector2D_Div_flt32 (struct v2D32_t *vector, f32 value);
void Vector2D_Div_flt64 (struct v2D64_t *vector, f64 value);

//****************************************************************************//
//      Rotation of vector                                                    //
//****************************************************************************//
void Vector2D_Rotate_flt32 (struct v2D32_t *vector, f32 cos, f32 sin);
void Vector2D_Rotate_flt64 (struct v2D64_t *vector, f64 cos, f64 sin);

//****************************************************************************//
//      Shearing of vector                                                    //
//****************************************************************************//

// Shearing parallel to the X axis
void Vector2D_ShearX_flt32 (struct v2D32_t *vector, f32 value);
void Vector2D_ShearX_flt64 (struct v2D64_t *vector, f64 value);

// Shearing parallel to the Y axis
void Vector2D_ShearY_flt32 (struct v2D32_t *vector, f32 value);
void Vector2D_ShearY_flt64 (struct v2D64_t *vector, f64 value);

//****************************************************************************//
//      Scaling of vector                                                     //
//****************************************************************************//
void Vector2D_Scale_flt32 (struct v2D32_t *target, const struct v2D32_t *source);
void Vector2D_Scale_flt64 (struct v2D64_t *target, const struct v2D64_t *source);

//****************************************************************************//
//      Scalar product                                                        //
//****************************************************************************//
f32 Vector2D_ScalarProduct_flt32 (const struct v2D32_t *target, const struct v2D32_t *source);
f64 Vector2D_ScalarProduct_flt64 (const struct v2D64_t *target, const struct v2D64_t *source);

//****************************************************************************//
//      Absolute value                                                        //
//****************************************************************************//
f32 Vector2D_Abs_flt32 (const struct v2D32_t *vector);
f64 Vector2D_Abs_flt64 (const struct v2D64_t *vector);

//****************************************************************************//
//      Cosine value of angle between the vectors                             //
//****************************************************************************//
f32 Vector2D_Cos_flt32 (const struct v2D32_t *target, const struct v2D32_t *source);
f64 Vector2D_Cos_flt64 (const struct v2D64_t *target, const struct v2D64_t *source);

//****************************************************************************//
//      Projection of the vector to another vector                            //
//****************************************************************************//
f32 Vector2D_Projection_flt32 (const struct v2D32_t *target, const struct v2D32_t *source);
f64 Vector2D_Projection_flt64 (const struct v2D64_t *target, const struct v2D64_t *source);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

// Check for zero vector
bool Vector2D_IsZero_flt32 (const struct v2D32_t *vector);
bool Vector2D_IsZero_flt64 (const struct v2D64_t *vector);

// Check for equality of the vectors
bool Vector2D_IsEqual_flt32 (const struct v2D32_t *target, const struct v2D32_t *source);
bool Vector2D_IsEqual_flt64 (const struct v2D64_t *target, const struct v2D64_t *source);

// Check for negativity of the vectors
bool Vector2D_IsNeg_flt32 (const struct v2D32_t *target, const struct v2D32_t *source);
bool Vector2D_IsNeg_flt64 (const struct v2D64_t *target, const struct v2D64_t *source);

// Check for collinearity of the vectors
bool Vector2D_IsCollinear_flt32 (const struct v2D32_t *target, const struct v2D32_t *source);
bool Vector2D_IsCollinear_flt64 (const struct v2D64_t *target, const struct v2D64_t *source);

// Check for orthogonality of the vectors
bool Vector2D_IsOrthogonal_flt32 (const struct v2D32_t *target, const struct v2D32_t *source);
bool Vector2D_IsOrthogonal_flt64 (const struct v2D64_t *target, const struct v2D64_t *source);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
