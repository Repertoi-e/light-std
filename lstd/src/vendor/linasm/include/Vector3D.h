/*                                                                    Vector3D.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                     FUNCTIONS TO OPERATE WITH 3D VECTORS                     #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Types.h>

//****************************************************************************//
//      3D vector structure (32-bit)                                          //
//****************************************************************************//
struct v3D32_t
{
	f32	x;		// X coordinate
	f32	y;		// Y coordinate
	f32	z;		// Z coordinate
};

//****************************************************************************//
//      3D vector structure (64-bit)                                          //
//****************************************************************************//
struct v3D64_t
{
	f64	x;		// X coordinate
	f64	y;		// Y coordinate
	f64	z;		// Z coordinate
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
class Vector3D
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
static void Normalize (v3D32_t *vector);
static void Normalize (v3D64_t *vector);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Reflection of vector                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Reflection through the origin
static void ReflectOrigin (v3D32_t *vector);
static void ReflectOrigin (v3D64_t *vector);

// Reflection through the X axis
static void ReflectX (v3D32_t *vector);
static void ReflectX (v3D64_t *vector);

// Reflection through the Y axis
static void ReflectY (v3D32_t *vector);
static void ReflectY (v3D64_t *vector);

// Reflection through the Z axis
static void ReflectZ (v3D32_t *vector);
static void ReflectZ (v3D64_t *vector);

// Reflection through the YZ plane
static void ReflectYZ (v3D32_t *vector);
static void ReflectYZ (v3D64_t *vector);

// Reflection through the XZ plane
static void ReflectXZ (v3D32_t *vector);
static void ReflectXZ (v3D64_t *vector);

// Reflection through the XY plane
static void ReflectXY (v3D32_t *vector);
static void ReflectXY (v3D64_t *vector);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

// Addition of vectors
static void Add (v3D32_t *target, const v3D32_t *source);
static void Add (v3D64_t *target, const v3D64_t *source);

// Subtraction of vectors
static void Sub (v3D32_t *target, const v3D32_t *source);
static void Sub (v3D64_t *target, const v3D64_t *source);

// Multiplication by scalar value
static void Mul (v3D32_t *vector, f32 value);
static void Mul (v3D64_t *vector, f64 value);

// Division by scalar value
static void Div (v3D32_t *vector, f32 value);
static void Div (v3D64_t *vector, f64 value);

//****************************************************************************//
//      Rotation of vector                                                    //
//****************************************************************************//

// Rotation around the X axis
static void RotateX (v3D32_t *vector, f32 cos, f32 sin);
static void RotateX (v3D64_t *vector, f64 cos, f64 sin);

// Rotation around the Y axis
static void RotateY (v3D32_t *vector, f32 cos, f32 sin);
static void RotateY (v3D64_t *vector, f64 cos, f64 sin);

// Rotation around the Z axis
static void RotateZ (v3D32_t *vector, f32 cos, f32 sin);
static void RotateZ (v3D64_t *vector, f64 cos, f64 sin);

//****************************************************************************//
//      Shearing of vector                                                    //
//****************************************************************************//

// Shearing parallel to the YZ plane
static void ShearYZ (v3D32_t *vector, f32 value1, f32 value2);
static void ShearYZ (v3D64_t *vector, f64 value1, f64 value2);

// Shearing parallel to the XZ plane
static void ShearXZ (v3D32_t *vector, f32 value1, f32 value2);
static void ShearXZ (v3D64_t *vector, f64 value1, f64 value2);

// Shearing parallel to the XY plane
static void ShearXY (v3D32_t *vector, f32 value1, f32 value2);
static void ShearXY (v3D64_t *vector, f64 value1, f64 value2);

//****************************************************************************//
//      Scaling of vector                                                     //
//****************************************************************************//
static void Scale (v3D32_t *target, const v3D32_t *source);
static void Scale (v3D64_t *target, const v3D64_t *source);

//****************************************************************************//
//      Products                                                              //
//****************************************************************************//

// Vector product
static void VectorProduct (v3D32_t *target, const v3D32_t *source);
static void VectorProduct (v3D64_t *target, const v3D64_t *source);

// Scalar product
static f32 ScalarProduct (const v3D32_t *target, const v3D32_t *source);
static f64 ScalarProduct (const v3D64_t *target, const v3D64_t *source);

// Triple product
static f32 TripleProduct (const v3D32_t *target, const v3D32_t *source1, const v3D32_t *source2);
static f64 TripleProduct (const v3D64_t *target, const v3D64_t *source1, const v3D64_t *source2);

//****************************************************************************//
//      Absolute value                                                        //
//****************************************************************************//
static f32 Abs (const v3D32_t *vector);
static f64 Abs (const v3D64_t *vector);

//****************************************************************************//
//      Cosine value of angle between the vectors                             //
//****************************************************************************//
static f32 Cos (const v3D32_t *target, const v3D32_t *source);
static f64 Cos (const v3D64_t *target, const v3D64_t *source);

//****************************************************************************//
//      Projection of the vector to another vector                            //
//****************************************************************************//
static f32 Projection (const v3D32_t *target, const v3D32_t *source);
static f64 Projection (const v3D64_t *target, const v3D64_t *source);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

// Check for zero vector
static bool IsZero (const v3D32_t *vector);
static bool IsZero (const v3D64_t *vector);

// Check for equality of the vectors
static bool IsEqual (const v3D32_t *target, const v3D32_t *source);
static bool IsEqual (const v3D64_t *target, const v3D64_t *source);

// Check for negativity of the vectors
static bool IsNeg (const v3D32_t *target, const v3D32_t *source);
static bool IsNeg (const v3D64_t *target, const v3D64_t *source);

// Check for collinearity of the vectors
static bool IsCollinear (const v3D32_t *target, const v3D32_t *source);
static bool IsCollinear (const v3D64_t *target, const v3D64_t *source);

// Check for orthogonality of the vectors
static bool IsOrthogonal (const v3D32_t *target, const v3D32_t *source);
static bool IsOrthogonal (const v3D64_t *target, const v3D64_t *source);

// Check for coplanarity of the vectors
static bool IsCoplanar (const v3D32_t *target, const v3D32_t *source1, const v3D32_t *source2);
static bool IsCoplanar (const v3D64_t *target, const v3D64_t *source1, const v3D64_t *source2);
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
void Vector3D_Normalize_flt32 (struct v3D32_t *vector);
void Vector3D_Normalize_flt64 (struct v3D64_t *vector);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Reflection of vector                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Reflection through the origin
void Vector3D_ReflectOrigin_flt32 (struct v3D32_t *vector);
void Vector3D_ReflectOrigin_flt64 (struct v3D64_t *vector);

// Reflection through the X axis
void Vector3D_ReflectX_flt32 (struct v3D32_t *vector);
void Vector3D_ReflectX_flt64 (struct v3D64_t *vector);

// Reflection through the Y axis
void Vector3D_ReflectY_flt32 (struct v3D32_t *vector);
void Vector3D_ReflectY_flt64 (struct v3D64_t *vector);

// Reflection through the Z axis
void Vector3D_ReflectZ_flt32 (struct v3D32_t *vector);
void Vector3D_ReflectZ_flt64 (struct v3D64_t *vector);

// Reflection through the YZ plane
void Vector3D_ReflectYZ_flt32 (struct v3D32_t *vector);
void Vector3D_ReflectYZ_flt64 (struct v3D64_t *vector);

// Reflection through the XZ plane
void Vector3D_ReflectXZ_flt32 (struct v3D32_t *vector);
void Vector3D_ReflectXZ_flt64 (struct v3D64_t *vector);

// Reflection through the XY plane
void Vector3D_ReflectXY_flt32 (struct v3D32_t *vector);
void Vector3D_ReflectXY_flt64 (struct v3D64_t *vector);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

// Addition of vectors
void Vector3D_Add_flt32 (struct v3D32_t *target, const struct v3D32_t *source);
void Vector3D_Add_flt64 (struct v3D64_t *target, const struct v3D64_t *source);

// Subtraction of vectors
void Vector3D_Sub_flt32 (struct v3D32_t *target, const struct v3D32_t *source);
void Vector3D_Sub_flt64 (struct v3D64_t *target, const struct v3D64_t *source);

// Multiplication by scalar value
void Vector3D_Mul_flt32 (struct v3D32_t *vector, f32 value);
void Vector3D_Mul_flt64 (struct v3D64_t *vector, f64 value);

// Division by scalar value
void Vector3D_Div_flt32 (struct v3D32_t *vector, f32 value);
void Vector3D_Div_flt64 (struct v3D64_t *vector, f64 value);

//****************************************************************************//
//      Rotation of vector                                                    //
//****************************************************************************//

// Rotation around the X axis
void Vector3D_RotateX_flt32 (struct v3D32_t *vector, f32 cos, f32 sin);
void Vector3D_RotateX_flt64 (struct v3D64_t *vector, f64 cos, f64 sin);

// Rotation around the Y axis
void Vector3D_RotateY_flt32 (struct v3D32_t *vector, f32 cos, f32 sin);
void Vector3D_RotateY_flt64 (struct v3D64_t *vector, f64 cos, f64 sin);

// Rotation around the Z axis
void Vector3D_RotateZ_flt32 (struct v3D32_t *vector, f32 cos, f32 sin);
void Vector3D_RotateZ_flt64 (struct v3D64_t *vector, f64 cos, f64 sin);

//****************************************************************************//
//      Shearing of vector                                                    //
//****************************************************************************//

// Shearing parallel to the YZ plane
void Vector3D_ShearYZ_flt32 (struct v3D32_t *vector, f32 value1, f32 value2);
void Vector3D_ShearYZ_flt64 (struct v3D64_t *vector, f64 value1, f64 value2);

// Shearing parallel to the XZ plane
void Vector3D_ShearXZ_flt32 (struct v3D32_t *vector, f32 value1, f32 value2);
void Vector3D_ShearXZ_flt64 (struct v3D64_t *vector, f64 value1, f64 value2);

// Shearing parallel to the XY plane
void Vector3D_ShearXY_flt32 (struct v3D32_t *vector, f32 value1, f32 value2);
void Vector3D_ShearXY_flt64 (struct v3D64_t *vector, f64 value1, f64 value2);

//****************************************************************************//
//      Scaling of vector                                                     //
//****************************************************************************//
void Vector3D_Scale_flt32 (struct v3D32_t *target, const struct v3D32_t *source);
void Vector3D_Scale_flt64 (struct v3D64_t *target, const struct v3D64_t *source);

//****************************************************************************//
//      Products                                                              //
//****************************************************************************//

// Vector product
void Vector3D_VectorProduct_flt32 (struct v3D32_t *target, const struct v3D32_t *source);
void Vector3D_VectorProduct_flt64 (struct v3D64_t *target, const struct v3D64_t *source);

// Scalar product
f32 Vector3D_ScalarProduct_flt32 (const struct v3D32_t *target, const struct v3D32_t *source);
f64 Vector3D_ScalarProduct_flt64 (const struct v3D64_t *target, const struct v3D64_t *source);

// Triple product
f32 Vector3D_TripleProduct_flt32 (const struct v3D32_t *target, const struct v3D32_t *source1, const struct v3D32_t *source2);
f64 Vector3D_TripleProduct_flt64 (const struct v3D64_t *target, const struct v3D64_t *source1, const struct v3D64_t *source2);

//****************************************************************************//
//      Absolute value                                                        //
//****************************************************************************//
f32 Vector3D_Abs_flt32 (const struct v3D32_t *vector);
f64 Vector3D_Abs_flt64 (const struct v3D64_t *vector);

//****************************************************************************//
//      Cosine value of angle between the vectors                             //
//****************************************************************************//
f32 Vector3D_Cos_flt32 (const struct v3D32_t *target, const struct v3D32_t *source);
f64 Vector3D_Cos_flt64 (const struct v3D64_t *target, const struct v3D64_t *source);

//****************************************************************************//
//      Projection of the vector to another vector                            //
//****************************************************************************//
f32 Vector3D_Projection_flt32 (const struct v3D32_t *target, const struct v3D32_t *source);
f64 Vector3D_Projection_flt64 (const struct v3D64_t *target, const struct v3D64_t *source);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

// Check for zero vector
bool Vector3D_IsZero_flt32 (const struct v3D32_t *vector);
bool Vector3D_IsZero_flt64 (const struct v3D64_t *vector);

// Check for equality of the vectors
bool Vector3D_IsEqual_flt32 (const struct v3D32_t *target, const struct v3D32_t *source);
bool Vector3D_IsEqual_flt64 (const struct v3D64_t *target, const struct v3D64_t *source);

// Check for negativity of the vectors
bool Vector3D_IsNeg_flt32 (const struct v3D32_t *target, const struct v3D32_t *source);
bool Vector3D_IsNeg_flt64 (const struct v3D64_t *target, const struct v3D64_t *source);

// Check for collinearity of the vectors
bool Vector3D_IsCollinear_flt32 (const struct v3D32_t *target, const struct v3D32_t *source);
bool Vector3D_IsCollinear_flt64 (const struct v3D64_t *target, const struct v3D64_t *source);

// Check for orthogonality of the vectors
bool Vector3D_IsOrthogonal_flt32 (const struct v3D32_t *target, const struct v3D32_t *source);
bool Vector3D_IsOrthogonal_flt64 (const struct v3D64_t *target, const struct v3D64_t *source);

// Check for coplanarity of the vectors
bool Vector3D_IsCoplanar_flt32 (const struct v3D32_t *target, const struct v3D32_t *source1, const struct v3D32_t *source2);
bool Vector3D_IsCoplanar_flt64 (const struct v3D64_t *target, const struct v3D64_t *source1, const struct v3D64_t *source2);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
