/*                                                                      Matrix.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                              MATRIX OPERATIONS                               #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Complex.h>

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
class Matrix
{
public:

//****************************************************************************//
//      Initialization                                                        //
//****************************************************************************//

// Initialization of rectangle matrix
static void InitRectangle (f32 matrix[], u32 rows, u32 cols, f32 value);
static void InitRectangle (f64 matrix[], u32 rows, u32 cols, f64 value);
static void InitRectangle (cmplx32_t matrix[], u32 rows, u32 cols, cmplx32_t value);
static void InitRectangle (cmplx64_t matrix[], u32 rows, u32 cols, cmplx64_t value);

// Initialization of diagonal matrix
static void InitDiagonal (f32 matrix[], u32 order, f32 value);
static void InitDiagonal (f64 matrix[], u32 order, f64 value);
static void InitDiagonal (cmplx32_t matrix[], u32 order, cmplx32_t value);
static void InitDiagonal (cmplx64_t matrix[], u32 order, cmplx64_t value);

//****************************************************************************//
//      Copying                                                               //
//****************************************************************************//
static void Copy (f32 target[], const f32 source[], u32 rows, u32 cols);
static void Copy (f64 target[], const f64 source[], u32 rows, u32 cols);
static void Copy (cmplx32_t target[], const cmplx32_t source[], u32 rows, u32 cols);
static void Copy (cmplx64_t target[], const cmplx64_t source[], u32 rows, u32 cols);

//****************************************************************************//
//      Arithmetic operations                                                 //
//****************************************************************************//

//============================================================================//
//      Unary operations                                                      //
//============================================================================//

// Negative matrix
static void Neg (f32 matrix[], u32 rows, u32 cols);
static void Neg (f64 matrix[], u32 rows, u32 cols);
static void Neg (cmplx32_t matrix[], u32 rows, u32 cols);
static void Neg (cmplx64_t matrix[], u32 rows, u32 cols);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

// Addition
static void Add (f32 target[], const f32 source[], u32 rows, u32 cols);
static void Add (f64 target[], const f64 source[], u32 rows, u32 cols);
static void Add (cmplx32_t target[], const cmplx32_t source[], u32 rows, u32 cols);
static void Add (cmplx64_t target[], const cmplx64_t source[], u32 rows, u32 cols);

// Subtraction
static void Sub (f32 target[], const f32 source[], u32 rows, u32 cols);
static void Sub (f64 target[], const f64 source[], u32 rows, u32 cols);
static void Sub (cmplx32_t target[], const cmplx32_t source[], u32 rows, u32 cols);
static void Sub (cmplx64_t target[], const cmplx64_t source[], u32 rows, u32 cols);

// Multiplication by scalar value
static void Mul (f32 matrix[], u32 rows, u32 cols, f32 value);
static void Mul (f64 matrix[], u32 rows, u32 cols, f64 value);
static void Mul (cmplx32_t matrix[], u32 rows, u32 cols, cmplx32_t value);
static void Mul (cmplx64_t matrix[], u32 rows, u32 cols, cmplx64_t value);

// Division by scalar value
static void Div (f32 matrix[], u32 rows, u32 cols, f32 value);
static void Div (f64 matrix[], u32 rows, u32 cols, f64 value);
static void Div (cmplx32_t matrix[], u32 rows, u32 cols, cmplx32_t value);
static void Div (cmplx64_t matrix[], u32 rows, u32 cols, cmplx64_t value);

// Multiplication by matrix
static void MulMatrix (f32 target[], const f32 source1[], const f32 source2[], u32 rows, u32 cols, u32 k);
static void MulMatrix (f64 target[], const f64 source1[], const f64 source2[], u32 rows, u32 cols, u32 k);
static void MulMatrix (cmplx32_t target[], const cmplx32_t source1[], const cmplx32_t source2[], u32 rows, u32 cols, u32 k);
static void MulMatrix (cmplx64_t target[], const cmplx64_t source1[], const cmplx64_t source2[], u32 rows, u32 cols, u32 k);

//****************************************************************************//
//      Transposition                                                         //
//****************************************************************************//
static void Transpose (f32 target[], const f32 source[], u32 rows, u32 cols);
static void Transpose (f64 target[], const f64 source[], u32 rows, u32 cols);
static void Transpose (cmplx32_t target[], const cmplx32_t source[], u32 rows, u32 cols);
static void Transpose (cmplx64_t target[], const cmplx64_t source[], u32 rows, u32 cols);

//****************************************************************************//
//      Inverse matrix                                                        //
//****************************************************************************//

// Inverse matrix through upper triangular matrix
static bool InverseUp (f32 target[], f32 source[], u32 order);
static bool InverseUp (f64 target[], f64 source[], u32 order);
static bool InverseUp (cmplx32_t target[], cmplx32_t source[], u32 order);
static bool InverseUp (cmplx64_t target[], cmplx64_t source[], u32 order);

// Inverse matrix through lower triangular matrix
static bool InverseLow (f32 target[], f32 source[], u32 order);
static bool InverseLow (f64 target[], f64 source[], u32 order);
static bool InverseLow (cmplx32_t target[], cmplx32_t source[], u32 order);
static bool InverseLow (cmplx64_t target[], cmplx64_t source[], u32 order);

//****************************************************************************//
//      Determinant                                                           //
//****************************************************************************//

//============================================================================//
//      Determinant of diagonal matrix                                        //
//============================================================================//
static f32 DeterminantDiagonal (const f32 matrix[], u32 order);
static f64 DeterminantDiagonal (const f64 matrix[], u32 order);
static cmplx32_t DeterminantDiagonal (const cmplx32_t matrix[], u32 order);
static cmplx64_t DeterminantDiagonal (const cmplx64_t matrix[], u32 order);

//============================================================================//
//      Determinant of triangular matrix                                      //
//============================================================================//
static f32 DeterminantTriangular (const f32 matrix[], u32 order);
static f64 DeterminantTriangular (const f64 matrix[], u32 order);
static cmplx32_t DeterminantTriangular (const cmplx32_t matrix[], u32 order);
static cmplx64_t DeterminantTriangular (const cmplx64_t matrix[], u32 order);

//============================================================================//
//      Determinant of square matrix                                          //
//============================================================================//

// Determinant of square matrix through upper triangular matrix
static f32 DeterminantSquareUp (f32 matrix[], u32 order);
static f64 DeterminantSquareUp (f64 matrix[], u32 order);
static cmplx32_t DeterminantSquareUp (cmplx32_t matrix[], u32 order);
static cmplx64_t DeterminantSquareUp (cmplx64_t matrix[], u32 order);

// Determinant of square matrix through lower triangular matrix
static f32 DeterminantSquareLow (f32 matrix[], u32 order);
static f64 DeterminantSquareLow (f64 matrix[], u32 order);
static cmplx32_t DeterminantSquareLow (cmplx32_t matrix[], u32 order);
static cmplx64_t DeterminantSquareLow (cmplx64_t matrix[], u32 order);

//****************************************************************************//
//      Solving of linear system                                              //
//****************************************************************************//

//============================================================================//
//      Gauss elimination                                                     //
//============================================================================//

// Gauss elimination to upper triangular matrix
static bool GaussUp (f32 coefficients[], f32 matrix[], u32 order);
static bool GaussUp (f64 coefficients[], f64 matrix[], u32 order);
static bool GaussUp (cmplx32_t coefficients[], cmplx32_t matrix[], u32 order);
static bool GaussUp (cmplx64_t coefficients[], cmplx64_t matrix[], u32 order);

// Gauss elimination to lower triangular matrix
static bool GaussLow (f32 coefficients[], f32 matrix[], u32 order);
static bool GaussLow (f64 coefficients[], f64 matrix[], u32 order);
static bool GaussLow (cmplx32_t coefficients[], cmplx32_t matrix[], u32 order);
static bool GaussLow (cmplx64_t coefficients[], cmplx64_t matrix[], u32 order);

//============================================================================//
//      Cholesky decomposition                                                //
//============================================================================//

// Cholesky decomposition to upper triangular matrix
static bool CholeskyUp (f32 coefficients[], f32 matrix[], u32 order);
static bool CholeskyUp (f64 coefficients[], f64 matrix[], u32 order);

// Cholesky decomposition to lower triangular matrix
static bool CholeskyLow (f32 coefficients[], f32 matrix[], u32 order);
static bool CholeskyLow (f64 coefficients[], f64 matrix[], u32 order);
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

// Initialization of rectangle matrix
void Matrix_InitRectangle_flt32 (f32 matrix[], u32 rows, u32 cols, f32 value);
void Matrix_InitRectangle_flt64 (f64 matrix[], u32 rows, u32 cols, f64 value);
void Matrix_InitRectangle_cmplx32 (cmplx32_t matrix[], u32 rows, u32 cols, cmplx32_t value);
void Matrix_InitRectangle_cmplx64 (cmplx64_t matrix[], u32 rows, u32 cols, cmplx64_t value);

// Initialization of diagonal matrix
void Matrix_InitDiagonal_flt32 (f32 matrix[], u32 order, f32 value);
void Matrix_InitDiagonal_flt64 (f64 matrix[], u32 order, f64 value);
void Matrix_InitDiagonal_cmplx32 (cmplx32_t matrix[], u32 order, cmplx32_t value);
void Matrix_InitDiagonal_cmplx64 (cmplx64_t matrix[], u32 order, cmplx64_t value);

//****************************************************************************//
//      Copying                                                               //
//****************************************************************************//
void Matrix_Copy_flt32 (f32 target[], const f32 source[], u32 rows, u32 cols);
void Matrix_Copy_flt64 (f64 target[], const f64 source[], u32 rows, u32 cols);
void Matrix_Copy_cmplx32 (cmplx32_t target[], const cmplx32_t source[], u32 rows, u32 cols);
void Matrix_Copy_cmplx64 (cmplx64_t target[], const cmplx64_t source[], u32 rows, u32 cols);

//****************************************************************************//
//      Arithmetic operations                                                 //
//****************************************************************************//

//============================================================================//
//      Unary operations                                                      //
//============================================================================//

// Negative matrix
void Matrix_Neg_flt32 (f32 matrix[], u32 rows, u32 cols);
void Matrix_Neg_flt64 (f64 matrix[], u32 rows, u32 cols);
void Matrix_Neg_cmplx32 (cmplx32_t matrix[], u32 rows, u32 cols);
void Matrix_Neg_cmplx64 (cmplx64_t matrix[], u32 rows, u32 cols);

//============================================================================//
//      Binary operations                                                     //
//============================================================================//

// Addition
void Matrix_Add_flt32 (f32 target[], const f32 source[], u32 rows, u32 cols);
void Matrix_Add_flt64 (f64 target[], const f64 source[], u32 rows, u32 cols);
void Matrix_Add_cmplx32 (cmplx32_t target[], const cmplx32_t source[], u32 rows, u32 cols);
void Matrix_Add_cmplx64 (cmplx64_t target[], const cmplx64_t source[], u32 rows, u32 cols);

// Subtraction
void Matrix_Sub_flt32 (f32 target[], const f32 source[], u32 rows, u32 cols);
void Matrix_Sub_flt64 (f64 target[], const f64 source[], u32 rows, u32 cols);
void Matrix_Sub_cmplx32 (cmplx32_t target[], const cmplx32_t source[], u32 rows, u32 cols);
void Matrix_Sub_cmplx64 (cmplx64_t target[], const cmplx64_t source[], u32 rows, u32 cols);

// Multiplication by scalar value
void Matrix_Mul_flt32 (f32 matrix[], u32 rows, u32 cols, f32 value);
void Matrix_Mul_flt64 (f64 matrix[], u32 rows, u32 cols, f64 value);
void Matrix_Mul_cmplx32 (cmplx32_t matrix[], u32 rows, u32 cols, cmplx32_t value);
void Matrix_Mul_cmplx64 (cmplx64_t matrix[], u32 rows, u32 cols, cmplx64_t value);

// Division by scalar value
void Matrix_Div_flt32 (f32 matrix[], u32 rows, u32 cols, f32 value);
void Matrix_Div_flt64 (f64 matrix[], u32 rows, u32 cols, f64 value);
void Matrix_Div_cmplx32 (cmplx32_t matrix[], u32 rows, u32 cols, cmplx32_t value);
void Matrix_Div_cmplx64 (cmplx64_t matrix[], u32 rows, u32 cols, cmplx64_t value);

// Multiplication by matrix
void Matrix_MulMatrix_flt32 (f32 target[], const f32 source1[], const f32 source2[], u32 rows, u32 cols, u32 k);
void Matrix_MulMatrix_flt64 (f64 target[], const f64 source1[], const f64 source2[], u32 rows, u32 cols, u32 k);
void Matrix_MulMatrix_cmplx32 (cmplx32_t target[], const cmplx32_t source1[], const cmplx32_t source2[], u32 rows, u32 cols, u32 k);
void Matrix_MulMatrix_cmplx64 (cmplx64_t target[], const cmplx64_t source1[], const cmplx64_t source2[], u32 rows, u32 cols, u32 k);

//****************************************************************************//
//      Transposition                                                         //
//****************************************************************************//
void Matrix_Transpose_flt32 (f32 target[], const f32 source[], u32 rows, u32 cols);
void Matrix_Transpose_flt64 (f64 target[], const f64 source[], u32 rows, u32 cols);
void Matrix_Transpose_cmplx32 (cmplx32_t target[], const cmplx32_t source[], u32 rows, u32 cols);
void Matrix_Transpose_cmplx64 (cmplx64_t target[], const cmplx64_t source[], u32 rows, u32 cols);

//****************************************************************************//
//      Inverse matrix                                                        //
//****************************************************************************//

// Inverse matrix through upper triangular matrix
bool Matrix_InverseUp_flt32 (f32 target[], f32 source[], u32 order);
bool Matrix_InverseUp_flt64 (f64 target[], f64 source[], u32 order);
bool Matrix_InverseUp_cmplx32 (cmplx32_t target[], cmplx32_t source[], u32 order);
bool Matrix_InverseUp_cmplx64 (cmplx64_t target[], cmplx64_t source[], u32 order);

// Inverse matrix through lower triangular matrix
bool Matrix_InverseLow_flt32 (f32 target[], f32 source[], u32 order);
bool Matrix_InverseLow_flt64 (f64 target[], f64 source[], u32 order);
bool Matrix_InverseLow_cmplx32 (cmplx32_t target[], cmplx32_t source[], u32 order);
bool Matrix_InverseLow_cmplx64 (cmplx64_t target[], cmplx64_t source[], u32 order);

//****************************************************************************//
//      Determinant                                                           //
//****************************************************************************//

//============================================================================//
//      Determinant of diagonal matrix                                        //
//============================================================================//
f32 Matrix_DeterminantDiagonal_flt32 (const f32 matrix[], u32 order);
f64 Matrix_DeterminantDiagonal_flt64 (const f64 matrix[], u32 order);
cmplx32_t Matrix_DeterminantDiagonal_cmplx32 (const cmplx32_t matrix[], u32 order);
cmplx64_t Matrix_DeterminantDiagonal_cmplx64 (const cmplx64_t matrix[], u32 order);

//============================================================================//
//      Determinant of triangular matrix                                      //
//============================================================================//
f32 Matrix_DeterminantTriangular_flt32 (const f32 matrix[], u32 order);
f64 Matrix_DeterminantTriangular_flt64 (const f64 matrix[], u32 order);
cmplx32_t Matrix_DeterminantTriangular_cmplx32 (const cmplx32_t matrix[], u32 order);
cmplx64_t Matrix_DeterminantTriangular_cmplx64 (const cmplx64_t matrix[], u32 order);

//============================================================================//
//      Determinant of square matrix                                          //
//============================================================================//

// Determinant of square matrix through upper triangular matrix
f32 Matrix_DeterminantSquareUp_flt32 (f32 matrix[], u32 order);
f64 Matrix_DeterminantSquareUp_flt64 (f64 matrix[], u32 order);
cmplx32_t Matrix_DeterminantSquareUp_cmplx32 (cmplx32_t matrix[], u32 order);
cmplx64_t Matrix_DeterminantSquareUp_cmplx64 (cmplx64_t matrix[], u32 order);

// Determinant of square matrix through lower triangular matrix
f32 Matrix_DeterminantSquareLow_flt32 (f32 matrix[], u32 order);
f64 Matrix_DeterminantSquareLow_flt64 (f64 matrix[], u32 order);
cmplx32_t Matrix_DeterminantSquareLow_cmplx32 (cmplx32_t matrix[], u32 order);
cmplx64_t Matrix_DeterminantSquareLow_cmplx64 (cmplx64_t matrix[], u32 order);

//****************************************************************************//
//      Solving of linear system                                              //
//****************************************************************************//

//============================================================================//
//      Gauss elimination                                                     //
//============================================================================//

// Gauss elimination to upper triangular matrix
bool Matrix_GaussUp_flt32 (f32 coefficients[], f32 matrix[], u32 order);
bool Matrix_GaussUp_flt64 (f64 coefficients[], f64 matrix[], u32 order);
bool Matrix_GaussUp_cmplx32 (cmplx32_t coefficients[], cmplx32_t matrix[], u32 order);
bool Matrix_GaussUp_cmplx64 (cmplx64_t coefficients[], cmplx64_t matrix[], u32 order);

// Gauss elimination to lower triangular matrix
bool Matrix_GaussLow_flt32 (f32 coefficients[], f32 matrix[], u32 order);
bool Matrix_GaussLow_flt64 (f64 coefficients[], f64 matrix[], u32 order);
bool Matrix_GaussLow_cmplx32 (cmplx32_t coefficients[], cmplx32_t matrix[], u32 order);
bool Matrix_GaussLow_cmplx64 (cmplx64_t coefficients[], cmplx64_t matrix[], u32 order);

//============================================================================//
//      Cholesky decomposition                                                //
//============================================================================//

// Cholesky decomposition to upper triangular matrix
bool Matrix_CholeskyUp_flt32 (f32 coefficients[], f32 matrix[], u32 order);
bool Matrix_CholeskyUp_flt64 (f64 coefficients[], f64 matrix[], u32 order);

// Cholesky decomposition to lower triangular matrix
bool Matrix_CholeskyLow_flt32 (f32 coefficients[], f32 matrix[], u32 order);
bool Matrix_CholeskyLow_flt64 (f64 coefficients[], f64 matrix[], u32 order);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
