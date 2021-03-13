/*                                                                      String.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                            SAFE STRING FUNCTIONS                             #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2016, Jack Black #
################################################################################
*/
# pragma	once
# include	<Types.h>

//****************************************************************************//
//      String compare function prototypes                                    //
//****************************************************************************//
typedef	s64 (*CmpChar8) (const utf8 string1[], const utf8 string2[]);
typedef	s64 (*CmpChar16) (const utf16 string1[], const utf16 string2[]);
typedef	s64 (*CmpChar32) (const utf32 string1[], const utf32 string2[]);

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
class String
{
public:

//****************************************************************************//
//      String length                                                         //
//****************************************************************************//
static u32 Len (const utf8 string[]);
static u32 Len (const utf16 string[]);
static u32 Len (const utf32 string[]);

//****************************************************************************//
//      Copying                                                               //
//****************************************************************************//

// Copying of string to string
static u32 Copy (utf8 target[], u32 maxlen, const utf8 source[]);
static u32 Copy (utf16 target[], u32 maxlen, const utf16 source[]);
static u32 Copy (utf32 target[], u32 maxlen, const utf32 source[]);

// Copying of characters sequence to string
static u32 CopyN (utf8 target[], u32 maxlen, const utf8 source[], u32 size);
static u32 CopyN (utf16 target[], u32 maxlen, const utf16 source[], u32 size);
static u32 CopyN (utf32 target[], u32 maxlen, const utf32 source[], u32 size);

//****************************************************************************//
//      Concatenating                                                         //
//****************************************************************************//

// Concatenating of string to string
static u32 Cat (utf8 target[], u32 maxlen, const utf8 source[]);
static u32 Cat (utf16 target[], u32 maxlen, const utf16 source[]);
static u32 Cat (utf32 target[], u32 maxlen, const utf32 source[]);

// Concatenating of characters sequence to string
static u32 CatN (utf8 target[], u32 maxlen, const utf8 source[], u32 size);
static u32 CatN (utf16 target[], u32 maxlen, const utf16 source[], u32 size);
static u32 CatN (utf32 target[], u32 maxlen, const utf32 source[], u32 size);

//****************************************************************************//
//      String comparison                                                     //
//****************************************************************************//

// Comparison of strings
static s64 Compare (const utf8 string1[], const utf8 string2[]);
static s64 Compare (const utf16 string1[], const utf16 string2[]);
static s64 Compare (const utf32 string1[], const utf32 string2[]);

// Comparison of characters sequences
static s64 CompareN (const utf8 string1[], const utf8 string2[], u32 size);
static s64 CompareN (const utf16 string1[], const utf16 string2[], u32 size);
static s64 CompareN (const utf32 string1[], const utf32 string2[], u32 size);

//****************************************************************************//
//      Symbol search                                                         //
//****************************************************************************//

//============================================================================//
//      Searching for single symbol                                           //
//============================================================================//

// Forward direction search
static u32 FindSymbolFwd (const utf8 string[], utf8 symbol);
static u32 FindSymbolFwd (const utf16 string[], utf16 symbol);
static u32 FindSymbolFwd (const utf32 string[], utf32 symbol);

// Backward direction search
static u32 FindSymbolBwd (const utf8 string[], utf8 symbol);
static u32 FindSymbolBwd (const utf16 string[], utf16 symbol);
static u32 FindSymbolBwd (const utf32 string[], utf32 symbol);

//============================================================================//
//      Searching for symbols set                                             //
//============================================================================//

// Forward direction search
static u32 FindSymbolsFwd (const utf8 string[], const utf8 symbols[]);
static u32 FindSymbolsFwd (const utf16 string[], const utf16 symbols[]);
static u32 FindSymbolsFwd (const utf32 string[], const utf32 symbols[]);

// Backward direction search
static u32 FindSymbolsBwd (const utf8 string[], const utf8 symbols[]);
static u32 FindSymbolsBwd (const utf16 string[], const utf16 symbols[]);
static u32 FindSymbolsBwd (const utf32 string[], const utf32 symbols[]);

//****************************************************************************//
//      Substring search                                                      //
//****************************************************************************//

//============================================================================//
//      Searching string for pattern                                          //
//============================================================================//

// Forward direction search
static u32 FindSubStringFwd (const utf8 string[], const utf8 pattern[]);
static u32 FindSubStringFwd (const utf16 string[], const utf16 pattern[]);
static u32 FindSubStringFwd (const utf32 string[], const utf32 pattern[]);

// Backward direction search
static u32 FindSubStringBwd (const utf8 string[], const utf8 pattern[]);
static u32 FindSubStringBwd (const utf16 string[], const utf16 pattern[]);
static u32 FindSubStringBwd (const utf32 string[], const utf32 pattern[]);

//============================================================================//
//      Searching characters sequence for pattern                             //
//============================================================================//

// Forward direction search
static u32 FindSubStringNFwd (const utf8 string[], u32 size, const utf8 pattern[]);
static u32 FindSubStringNFwd (const utf16 string[], u32 size, const utf16 pattern[]);
static u32 FindSubStringNFwd (const utf32 string[], u32 size, const utf32 pattern[]);

// Backward direction search
static u32 FindSubStringNBwd (const utf8 string[], u32 size, const utf8 pattern[]);
static u32 FindSubStringNBwd (const utf16 string[], u32 size, const utf16 pattern[]);
static u32 FindSubStringNBwd (const utf32 string[], u32 size, const utf32 pattern[]);

//****************************************************************************//
//      String search                                                         //
//****************************************************************************//

//============================================================================//
//      Linear search                                                         //
//============================================================================//

// Forward direction search
static u32 FindFwd (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindFwd (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindFwd (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Backward direction search
static u32 FindBwd (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindBwd (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindBwd (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//============================================================================//
//      Binary search                                                         //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Searching for first equal string
static u32 FindFirstEqualAsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindFirstEqualAsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindFirstEqualAsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for last equal string
static u32 FindLastEqualAsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindLastEqualAsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindLastEqualAsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for greater string
static u32 FindGreatAsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindGreatAsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindGreatAsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for greater or equal string
static u32 FindGreatOrEqualAsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindGreatOrEqualAsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindGreatOrEqualAsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for less string
static u32 FindLessAsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindLessAsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindLessAsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for less or equal string
static u32 FindLessOrEqualAsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindLessOrEqualAsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindLessOrEqualAsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Searching for first equal string
static u32 FindFirstEqualDsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindFirstEqualDsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindFirstEqualDsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for last equal string
static u32 FindLastEqualDsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindLastEqualDsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindLastEqualDsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for less string
static u32 FindLessDsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindLessDsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindLessDsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for less or equal string
static u32 FindLessOrEqualDsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindLessOrEqualDsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindLessOrEqualDsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for greater string
static u32 FindGreatDsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindGreatDsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindGreatDsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for greater or equal string
static u32 FindGreatOrEqualDsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 FindGreatOrEqualDsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 FindGreatOrEqualDsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//****************************************************************************//
//      Counting                                                              //
//****************************************************************************//

//============================================================================//
//      Symbol counting                                                       //
//============================================================================//

// Single symbol counting
static u32 CountSymbol (const utf8 string[], utf8 symbol);
static u32 CountSymbol (const utf16 string[], utf16 symbol);
static u32 CountSymbol (const utf32 string[], utf32 symbol);

// Symbols set counting
static u32 CountSymbols (const utf8 string[], const utf8 symbols[]);
static u32 CountSymbols (const utf16 string[], const utf16 symbols[]);
static u32 CountSymbols (const utf32 string[], const utf32 symbols[]);

//============================================================================//
//      String counting                                                       //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Linear counting                                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static u32 CountString (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 CountString (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 CountString (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Binary counting                                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Ascending sort order                                                  //
//----------------------------------------------------------------------------//
static u32 CountStringAsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 CountStringAsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 CountStringAsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//----------------------------------------------------------------------------//
//      Descending sort order                                                 //
//----------------------------------------------------------------------------//
static u32 CountStringDsc (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
static u32 CountStringDsc (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
static u32 CountStringDsc (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//****************************************************************************//
//      Replacing                                                             //
//****************************************************************************//

// Symbol replacing
static void ReplaceSymbol (utf8 string[], utf8 symbol, utf8 value);
static void ReplaceSymbol (utf16 string[], utf16 symbol, utf16 value);
static void ReplaceSymbol (utf32 string[], utf32 symbol, utf32 value);

// String replacing
static void ReplaceString (const utf8* array[], u32 size, const utf8 string[], const utf8 value[], CmpChar8 func);
static void ReplaceString (const utf16* array[], u32 size, const utf16 string[], const utf16 value[], CmpChar16 func);
static void ReplaceString (const utf32* array[], u32 size, const utf32 string[], const utf32 value[], CmpChar32 func);

//****************************************************************************//
//      Order reversing                                                       //
//****************************************************************************//
static void Reverse (const utf8* array[], u32 size);
static void Reverse (const utf16* array[], u32 size);
static void Reverse (const utf32* array[], u32 size);

//****************************************************************************//
//      Unique values                                                         //
//****************************************************************************//
static u32 Unique (const utf8* unique[], const utf8* array[], u32 size, CmpChar8 func);
static u32 Unique (const utf16* unique[], const utf16* array[], u32 size, CmpChar16 func);
static u32 Unique (const utf32* unique[], const utf32* array[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Duplicate values                                                      //
//****************************************************************************//
static u32 Duplicates (const utf8* unique[], u32 count[], const utf8* array[], u32 size, CmpChar8 func);
static u32 Duplicates (const utf16* unique[], u32 count[], const utf16* array[], u32 size, CmpChar16 func);
static u32 Duplicates (const utf32* unique[], u32 count[], const utf32* array[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Insertion sort                                                        //
//****************************************************************************//

//============================================================================//
//      String array sorting                                                  //
//============================================================================//

// Ascending sort order
static void InsertSortAsc (const utf8* array[], u32 size, CmpChar8 func);
static void InsertSortAsc (const utf16* array[], u32 size, CmpChar16 func);
static void InsertSortAsc (const utf32* array[], u32 size, CmpChar32 func);

// Descending sort order
static void InsertSortDsc (const utf8* array[], u32 size, CmpChar8 func);
static void InsertSortDsc (const utf16* array[], u32 size, CmpChar16 func);
static void InsertSortDsc (const utf32* array[], u32 size, CmpChar32 func);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

// Ascending sort order
static void InsertSortKeyAsc (const utf8* key[], const void* ptr[], u32 size, CmpChar8 func);
static void InsertSortKeyAsc (const utf16* key[], const void* ptr[], u32 size, CmpChar16 func);
static void InsertSortKeyAsc (const utf32* key[], const void* ptr[], u32 size, CmpChar32 func);

// Descending sort order
static void InsertSortKeyDsc (const utf8* key[], const void* ptr[], u32 size, CmpChar8 func);
static void InsertSortKeyDsc (const utf16* key[], const void* ptr[], u32 size, CmpChar16 func);
static void InsertSortKeyDsc (const utf32* key[], const void* ptr[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Quick sort                                                            //
//****************************************************************************//

//============================================================================//
//      String array sorting                                                  //
//============================================================================//

// Ascending sort order
static void QuickSortAsc (const utf8* array[], u32 size, CmpChar8 func);
static void QuickSortAsc (const utf16* array[], u32 size, CmpChar16 func);
static void QuickSortAsc (const utf32* array[], u32 size, CmpChar32 func);

// Descending sort order
static void QuickSortDsc (const utf8* array[], u32 size, CmpChar8 func);
static void QuickSortDsc (const utf16* array[], u32 size, CmpChar16 func);
static void QuickSortDsc (const utf32* array[], u32 size, CmpChar32 func);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

// Ascending sort order
static void QuickSortKeyAsc (const utf8* key[], const void* ptr[], u32 size, CmpChar8 func);
static void QuickSortKeyAsc (const utf16* key[], const void* ptr[], u32 size, CmpChar16 func);
static void QuickSortKeyAsc (const utf32* key[], const void* ptr[], u32 size, CmpChar32 func);

// Descending sort order
static void QuickSortKeyDsc (const utf8* key[], const void* ptr[], u32 size, CmpChar8 func);
static void QuickSortKeyDsc (const utf16* key[], const void* ptr[], u32 size, CmpChar16 func);
static void QuickSortKeyDsc (const utf32* key[], const void* ptr[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Merge sort                                                            //
//****************************************************************************//

//============================================================================//
//      String array sorting                                                  //
//============================================================================//

// Ascending sort order
static void MergeSortAsc (const utf8* array[], const utf8* temp[], u32 size, CmpChar8 func);
static void MergeSortAsc (const utf16* array[], const utf16* temp[], u32 size, CmpChar16 func);
static void MergeSortAsc (const utf32* array[], const utf32* temp[], u32 size, CmpChar32 func);

// Descending sort order
static void MergeSortDsc (const utf8* array[], const utf8* temp[], u32 size, CmpChar8 func);
static void MergeSortDsc (const utf16* array[], const utf16* temp[], u32 size, CmpChar16 func);
static void MergeSortDsc (const utf32* array[], const utf32* temp[], u32 size, CmpChar32 func);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

// Ascending sort order
static void MergeSortKeyAsc (const utf8* key[], const void* ptr[], const utf8* tkey[], const void* tptr[], u32 size, CmpChar8 func);
static void MergeSortKeyAsc (const utf16* key[], const void* ptr[], const utf16* tkey[], const void* tptr[], u32 size, CmpChar16 func);
static void MergeSortKeyAsc (const utf32* key[], const void* ptr[], const utf32* tkey[], const void* tptr[], u32 size, CmpChar32 func);

// Descending sort order
static void MergeSortKeyDsc (const utf8* key[], const void* ptr[], const utf8* tkey[], const void* tptr[], u32 size, CmpChar8 func);
static void MergeSortKeyDsc (const utf16* key[], const void* ptr[], const utf16* tkey[], const void* tptr[], u32 size, CmpChar16 func);
static void MergeSortKeyDsc (const utf32* key[], const void* ptr[], const utf32* tkey[], const void* tptr[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Merging of sorted strings                                             //
//****************************************************************************//

//============================================================================//
//      String array merging                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void MergeAsc (const utf8* target[], const utf8* src1[], u32 size1, const utf8* src2[], u32 size2, CmpChar8 func);
static void MergeAsc (const utf16* target[], const utf16* src1[], u32 size1, const utf16* src2[], u32 size2, CmpChar16 func);
static void MergeAsc (const utf32* target[], const utf32* src1[], u32 size1, const utf32* src2[], u32 size2, CmpChar32 func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void MergeDsc (const utf8* target[], const utf8* src1[], u32 size1, const utf8* src2[], u32 size2, CmpChar8 func);
static void MergeDsc (const utf16* target[], const utf16* src1[], u32 size1, const utf16* src2[], u32 size2, CmpChar16 func);
static void MergeDsc (const utf32* target[], const utf32* src1[], u32 size1, const utf32* src2[], u32 size2, CmpChar32 func);

//============================================================================//
//      Key array merging                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void MergeKeyAsc (const utf8* tkey[], const void* tptr[], const utf8* skey1[], const void* sptr1[], u32 size1, const utf8* skey2[], const void* sptr2[], u32 size2, CmpChar8 func);
static void MergeKeyAsc (const utf16* tkey[], const void* tptr[], const utf16* skey1[], const void* sptr1[], u32 size1, const utf16* skey2[], const void* sptr2[], u32 size2, CmpChar16 func);
static void MergeKeyAsc (const utf32* tkey[], const void* tptr[], const utf32* skey1[], const void* sptr1[], u32 size1, const utf32* skey2[], const void* sptr2[], u32 size2, CmpChar32 func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static void MergeKeyDsc (const utf8* tkey[], const void* tptr[], const utf8* skey1[], const void* sptr1[], u32 size1, const utf8* skey2[], const void* sptr2[], u32 size2, CmpChar8 func);
static void MergeKeyDsc (const utf16* tkey[], const void* tptr[], const utf16* skey1[], const void* sptr1[], u32 size1, const utf16* skey2[], const void* sptr2[], u32 size2, CmpChar16 func);
static void MergeKeyDsc (const utf32* tkey[], const void* tptr[], const utf32* skey1[], const void* sptr1[], u32 size1, const utf32* skey2[], const void* sptr2[], u32 size2, CmpChar32 func);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

//============================================================================//
//      Check for duplicate values                                            //
//============================================================================//
static u32 CheckDup (const utf8* array[], u32 size, CmpChar8 func);
static u32 CheckDup (const utf16* array[], u32 size, CmpChar16 func);
static u32 CheckDup (const utf32* array[], u32 size, CmpChar32 func);

//============================================================================//
//      Check for sort order                                                  //
//============================================================================//

// Check for ascending sort order
static u32 CheckSortAsc (const utf8* array[], u32 size, CmpChar8 func);
static u32 CheckSortAsc (const utf16* array[], u32 size, CmpChar16 func);
static u32 CheckSortAsc (const utf32* array[], u32 size, CmpChar32 func);

// Check for descending sort order
static u32 CheckSortDsc (const utf8* array[], u32 size, CmpChar8 func);
static u32 CheckSortDsc (const utf16* array[], u32 size, CmpChar16 func);
static u32 CheckSortDsc (const utf32* array[], u32 size, CmpChar32 func);

//****************************************************************************//
//      String hashing                                                        //
//****************************************************************************//

//============================================================================//
//      32-bit hash functions                                                 //
//============================================================================//
static u32 Hash32 (const utf8 string[]);
static u32 Hash32 (const utf16 string[]);
static u32 Hash32 (const utf32 string[]);

//============================================================================//
//      64-bit hash functions                                                 //
//============================================================================//
static u64 Hash64 (const utf8 string[]);
static u64 Hash64 (const utf16 string[]);
static u64 Hash64 (const utf32 string[]);
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      String length                                                         //
//****************************************************************************//
u32 String_Len_char8 (const utf8 string[]);
u32 String_Len_char16 (const utf16 string[]);
u32 String_Len_char32 (const utf32 string[]);

//****************************************************************************//
//      Copying                                                               //
//****************************************************************************//

// Copying of string to string
u32 String_Copy_char8 (utf8 target[], u32 maxlen, const utf8 source[]);
u32 String_Copy_char16 (utf16 target[], u32 maxlen, const utf16 source[]);
u32 String_Copy_char32 (utf32 target[], u32 maxlen, const utf32 source[]);

// Copying of characters sequence to string
u32 String_CopyN_char8 (utf8 target[], u32 maxlen, const utf8 source[], u32 size);
u32 String_CopyN_char16 (utf16 target[], u32 maxlen, const utf16 source[], u32 size);
u32 String_CopyN_char32 (utf32 target[], u32 maxlen, const utf32 source[], u32 size);

//****************************************************************************//
//      Concatenating                                                         //
//****************************************************************************//

// Concatenating of string to string
u32 String_Cat_char8 (utf8 target[], u32 maxlen, const utf8 source[]);
u32 String_Cat_char16 (utf16 target[], u32 maxlen, const utf16 source[]);
u32 String_Cat_char32 (utf32 target[], u32 maxlen, const utf32 source[]);

// Concatenating of characters sequence to string
u32 String_CatN_char8 (utf8 target[], u32 maxlen, const utf8 source[], u32 size);
u32 String_CatN_char16 (utf16 target[], u32 maxlen, const utf16 source[], u32 size);
u32 String_CatN_char32 (utf32 target[], u32 maxlen, const utf32 source[], u32 size);

//****************************************************************************//
//      String comparison                                                     //
//****************************************************************************//

// Comparison of strings
s64 String_Compare_char8 (const utf8 string1[], const utf8 string2[]);
s64 String_Compare_char16 (const utf16 string1[], const utf16 string2[]);
s64 String_Compare_char32 (const utf32 string1[], const utf32 string2[]);

// Comparison of characters sequences
s64 String_CompareN_char8 (const utf8 string1[], const utf8 string2[], u32 size);
s64 String_CompareN_char16 (const utf16 string1[], const utf16 string2[], u32 size);
s64 String_CompareN_char32 (const utf32 string1[], const utf32 string2[], u32 size);

//****************************************************************************//
//      Symbol search                                                         //
//****************************************************************************//

//============================================================================//
//      Searching for single symbol                                           //
//============================================================================//

// Forward direction search
u32 String_FindSymbolFwd_char8 (const utf8 string[], utf8 symbol);
u32 String_FindSymbolFwd_char16 (const utf16 string[], utf16 symbol);
u32 String_FindSymbolFwd_char32 (const utf32 string[], utf32 symbol);

// Backward direction search
u32 String_FindSymbolBwd_char8 (const utf8 string[], utf8 symbol);
u32 String_FindSymbolBwd_char16 (const utf16 string[], utf16 symbol);
u32 String_FindSymbolBwd_char32 (const utf32 string[], utf32 symbol);

//============================================================================//
//      Searching for symbols set                                             //
//============================================================================//

// Forward direction search
u32 String_FindSymbolsFwd_char8 (const utf8 string[], const utf8 symbols[]);
u32 String_FindSymbolsFwd_char16 (const utf16 string[], const utf16 symbols[]);
u32 String_FindSymbolsFwd_char32 (const utf32 string[], const utf32 symbols[]);

// Backward direction search
u32 String_FindSymbolsBwd_char8 (const utf8 string[], const utf8 symbols[]);
u32 String_FindSymbolsBwd_char16 (const utf16 string[], const utf16 symbols[]);
u32 String_FindSymbolsBwd_char32 (const utf32 string[], const utf32 symbols[]);

//****************************************************************************//
//      Substring search                                                      //
//****************************************************************************//

//============================================================================//
//      Searching string for pattern                                          //
//============================================================================//

// Forward direction search
u32 String_FindSubStringFwd_char8 (const utf8 string[], const utf8 pattern[]);
u32 String_FindSubStringFwd_char16 (const utf16 string[], const utf16 pattern[]);
u32 String_FindSubStringFwd_char32 (const utf32 string[], const utf32 pattern[]);

// Backward direction search
u32 String_FindSubStringBwd_char8 (const utf8 string[], const utf8 pattern[]);
u32 String_FindSubStringBwd_char16 (const utf16 string[], const utf16 pattern[]);
u32 String_FindSubStringBwd_char32 (const utf32 string[], const utf32 pattern[]);

//============================================================================//
//      Searching characters sequence for pattern                             //
//============================================================================//

// Forward direction search
u32 String_FindSubStringNFwd_char8 (const utf8 string[], u32 size, const utf8 pattern[]);
u32 String_FindSubStringNFwd_char16 (const utf16 string[], u32 size, const utf16 pattern[]);
u32 String_FindSubStringNFwd_char32 (const utf32 string[], u32 size, const utf32 pattern[]);

// Backward direction search
u32 String_FindSubStringNBwd_char8 (const utf8 string[], u32 size, const utf8 pattern[]);
u32 String_FindSubStringNBwd_char16 (const utf16 string[], u32 size, const utf16 pattern[]);
u32 String_FindSubStringNBwd_char32 (const utf32 string[], u32 size, const utf32 pattern[]);

//****************************************************************************//
//      String search                                                         //
//****************************************************************************//

//============================================================================//
//      Linear search                                                         //
//============================================================================//

// Forward direction search
u32 String_FindFwd_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindFwd_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindFwd_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Backward direction search
u32 String_FindBwd_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindBwd_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindBwd_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//============================================================================//
//      Binary search                                                         //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Searching for first equal string
u32 String_FindFirstEqualAsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindFirstEqualAsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindFirstEqualAsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for last equal string
u32 String_FindLastEqualAsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindLastEqualAsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindLastEqualAsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for greater string
u32 String_FindGreatAsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindGreatAsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindGreatAsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for greater or equal string
u32 String_FindGreatOrEqualAsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindGreatOrEqualAsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindGreatOrEqualAsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for less string
u32 String_FindLessAsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindLessAsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindLessAsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for less or equal string
u32 String_FindLessOrEqualAsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindLessOrEqualAsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindLessOrEqualAsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Searching for first equal string
u32 String_FindFirstEqualDsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindFirstEqualDsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindFirstEqualDsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for last equal string
u32 String_FindLastEqualDsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindLastEqualDsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindLastEqualDsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for less string
u32 String_FindLessDsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindLessDsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindLessDsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for less or equal string
u32 String_FindLessOrEqualDsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindLessOrEqualDsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindLessOrEqualDsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for greater string
u32 String_FindGreatDsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindGreatDsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindGreatDsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

// Searching for greater or equal string
u32 String_FindGreatOrEqualDsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_FindGreatOrEqualDsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_FindGreatOrEqualDsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//****************************************************************************//
//      Counting                                                              //
//****************************************************************************//

//============================================================================//
//      Symbol counting                                                       //
//============================================================================//

// Single symbol counting
u32 String_CountSymbol_char8 (const utf8 string[], utf8 symbol);
u32 String_CountSymbol_char16 (const utf16 string[], utf16 symbol);
u32 String_CountSymbol_char32 (const utf32 string[], utf32 symbol);

// Symbols set counting
u32 String_CountSymbols_char8 (const utf8 string[], const utf8 symbols[]);
u32 String_CountSymbols_char16 (const utf16 string[], const utf16 symbols[]);
u32 String_CountSymbols_char32 (const utf32 string[], const utf32 symbols[]);

//============================================================================//
//      String counting                                                       //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Linear counting                                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
u32 String_CountString_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_CountString_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_CountString_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Binary counting                                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//----------------------------------------------------------------------------//
//      Ascending sort order                                                  //
//----------------------------------------------------------------------------//
u32 String_CountStringAsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_CountStringAsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_CountStringAsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//----------------------------------------------------------------------------//
//      Descending sort order                                                 //
//----------------------------------------------------------------------------//
u32 String_CountStringDsc_char8 (const utf8* array[], u32 size, const utf8 string[], CmpChar8 func);
u32 String_CountStringDsc_char16 (const utf16* array[], u32 size, const utf16 string[], CmpChar16 func);
u32 String_CountStringDsc_char32 (const utf32* array[], u32 size, const utf32 string[], CmpChar32 func);

//****************************************************************************//
//      Replacing                                                             //
//****************************************************************************//

// Symbol replacing
void String_ReplaceSymbol_char8 (utf8 string[], utf8 symbol, utf8 value);
void String_ReplaceSymbol_char16 (utf16 string[], utf16 symbol, utf16 value);
void String_ReplaceSymbol_char32 (utf32 string[], utf32 symbol, utf32 value);

// String replacing
void String_ReplaceString_char8 (const utf8* array[], u32 size, const utf8 string[], const utf8 value[], CmpChar8 func);
void String_ReplaceString_char16 (const utf16* array[], u32 size, const utf16 string[], const utf16 value[], CmpChar16 func);
void String_ReplaceString_char32 (const utf32* array[], u32 size, const utf32 string[], const utf32 value[], CmpChar32 func);

//****************************************************************************//
//      Order reversing                                                       //
//****************************************************************************//
void String_Reverse_char8 (const utf8* array[], u32 size);
void String_Reverse_char16 (const utf16* array[], u32 size);
void String_Reverse_char32 (const utf32* array[], u32 size);

//****************************************************************************//
//      Unique values                                                         //
//****************************************************************************//
u32 String_Unique_char8 (const utf8* unique[], const utf8* array[], u32 size, CmpChar8 func);
u32 String_Unique_char16 (const utf16* unique[], const utf16* array[], u32 size, CmpChar16 func);
u32 String_Unique_char32 (const utf32* unique[], const utf32* array[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Duplicate values                                                      //
//****************************************************************************//
u32 String_Duplicates_char8 (const utf8* unique[], u32 count[], const utf8* array[], u32 size, CmpChar8 func);
u32 String_Duplicates_char16 (const utf16* unique[], u32 count[], const utf16* array[], u32 size, CmpChar16 func);
u32 String_Duplicates_char32 (const utf32* unique[], u32 count[], const utf32* array[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Insertion sort                                                        //
//****************************************************************************//

//============================================================================//
//      String array sorting                                                  //
//============================================================================//

// Ascending sort order
void String_InsertSortAsc_char8 (const utf8* array[], u32 size, CmpChar8 func);
void String_InsertSortAsc_char16 (const utf16* array[], u32 size, CmpChar16 func);
void String_InsertSortAsc_char32 (const utf32* array[], u32 size, CmpChar32 func);

// Descending sort order
void String_InsertSortDsc_char8 (const utf8* array[], u32 size, CmpChar8 func);
void String_InsertSortDsc_char16 (const utf16* array[], u32 size, CmpChar16 func);
void String_InsertSortDsc_char32 (const utf32* array[], u32 size, CmpChar32 func);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

// Ascending sort order
void String_InsertSortKeyAsc_char8 (const utf8* key[], const void* ptr[], u32 size, CmpChar8 func);
void String_InsertSortKeyAsc_char16 (const utf16* key[], const void* ptr[], u32 size, CmpChar16 func);
void String_InsertSortKeyAsc_char32 (const utf32* key[], const void* ptr[], u32 size, CmpChar32 func);

// Descending sort order
void String_InsertSortKeyDsc_char8 (const utf8* key[], const void* ptr[], u32 size, CmpChar8 func);
void String_InsertSortKeyDsc_char16 (const utf16* key[], const void* ptr[], u32 size, CmpChar16 func);
void String_InsertSortKeyDsc_char32 (const utf32* key[], const void* ptr[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Quick sort                                                            //
//****************************************************************************//

//============================================================================//
//      String array sorting                                                  //
//============================================================================//

// Ascending sort order
void String_QuickSortAsc_char8 (const utf8* array[], u32 size, CmpChar8 func);
void String_QuickSortAsc_char16 (const utf16* array[], u32 size, CmpChar16 func);
void String_QuickSortAsc_char32 (const utf32* array[], u32 size, CmpChar32 func);

// Descending sort order
void String_QuickSortDsc_char8 (const utf8* array[], u32 size, CmpChar8 func);
void String_QuickSortDsc_char16 (const utf16* array[], u32 size, CmpChar16 func);
void String_QuickSortDsc_char32 (const utf32* array[], u32 size, CmpChar32 func);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

// Ascending sort order
void String_QuickSortKeyAsc_char8 (const utf8* key[], const void* ptr[], u32 size, CmpChar8 func);
void String_QuickSortKeyAsc_char16 (const utf16* key[], const void* ptr[], u32 size, CmpChar16 func);
void String_QuickSortKeyAsc_char32 (const utf32* key[], const void* ptr[], u32 size, CmpChar32 func);

// Descending sort order
void String_QuickSortKeyDsc_char8 (const utf8* key[], const void* ptr[], u32 size, CmpChar8 func);
void String_QuickSortKeyDsc_char16 (const utf16* key[], const void* ptr[], u32 size, CmpChar16 func);
void String_QuickSortKeyDsc_char32 (const utf32* key[], const void* ptr[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Merge sort                                                            //
//****************************************************************************//

//============================================================================//
//      String array sorting                                                  //
//============================================================================//

// Ascending sort order
void String_MergeSortAsc_char8 (const utf8* array[], const utf8* temp[], u32 size, CmpChar8 func);
void String_MergeSortAsc_char16 (const utf16* array[], const utf16* temp[], u32 size, CmpChar16 func);
void String_MergeSortAsc_char32 (const utf32* array[], const utf32* temp[], u32 size, CmpChar32 func);

// Descending sort order
void String_MergeSortDsc_char8 (const utf8* array[], const utf8* temp[], u32 size, CmpChar8 func);
void String_MergeSortDsc_char16 (const utf16* array[], const utf16* temp[], u32 size, CmpChar16 func);
void String_MergeSortDsc_char32 (const utf32* array[], const utf32* temp[], u32 size, CmpChar32 func);

//============================================================================//
//      Key array sorting                                                     //
//============================================================================//

// Ascending sort order
void String_MergeSortKeyAsc_char8 (const utf8* key[], const void* ptr[], const utf8* tkey[], const void* tptr[], u32 size, CmpChar8 func);
void String_MergeSortKeyAsc_char16 (const utf16* key[], const void* ptr[], const utf16* tkey[], const void* tptr[], u32 size, CmpChar16 func);
void String_MergeSortKeyAsc_char32 (const utf32* key[], const void* ptr[], const utf32* tkey[], const void* tptr[], u32 size, CmpChar32 func);

// Descending sort order
void String_MergeSortKeyDsc_char8 (const utf8* key[], const void* ptr[], const utf8* tkey[], const void* tptr[], u32 size, CmpChar8 func);
void String_MergeSortKeyDsc_char16 (const utf16* key[], const void* ptr[], const utf16* tkey[], const void* tptr[], u32 size, CmpChar16 func);
void String_MergeSortKeyDsc_char32 (const utf32* key[], const void* ptr[], const utf32* tkey[], const void* tptr[], u32 size, CmpChar32 func);

//****************************************************************************//
//      Merging of sorted strings                                             //
//****************************************************************************//

//============================================================================//
//      String array merging                                                  //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void String_MergeAsc_char8 (const utf8* target[], const utf8* src1[], u32 size1, const utf8* src2[], u32 size2, CmpChar8 func);
void String_MergeAsc_char16 (const utf16* target[], const utf16* src1[], u32 size1, const utf16* src2[], u32 size2, CmpChar16 func);
void String_MergeAsc_char32 (const utf32* target[], const utf32* src1[], u32 size1, const utf32* src2[], u32 size2, CmpChar32 func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void String_MergeDsc_char8 (const utf8* target[], const utf8* src1[], u32 size1, const utf8* src2[], u32 size2, CmpChar8 func);
void String_MergeDsc_char16 (const utf16* target[], const utf16* src1[], u32 size1, const utf16* src2[], u32 size2, CmpChar16 func);
void String_MergeDsc_char32 (const utf32* target[], const utf32* src1[], u32 size1, const utf32* src2[], u32 size2, CmpChar32 func);

//============================================================================//
//      Key array merging                                                     //
//============================================================================//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Ascending sort order                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void String_MergeKeyAsc_char8 (const utf8* tkey[], const void* tptr[], const utf8* skey1[], const void* sptr1[], u32 size1, const utf8* skey2[], const void* sptr2[], u32 size2, CmpChar8 func);
void String_MergeKeyAsc_char16 (const utf16* tkey[], const void* tptr[], const utf16* skey1[], const void* sptr1[], u32 size1, const utf16* skey2[], const void* sptr2[], u32 size2, CmpChar16 func);
void String_MergeKeyAsc_char32 (const utf32* tkey[], const void* tptr[], const utf32* skey1[], const void* sptr1[], u32 size1, const utf32* skey2[], const void* sptr2[], u32 size2, CmpChar32 func);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//      Descending sort order                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void String_MergeKeyDsc_char8 (const utf8* tkey[], const void* tptr[], const utf8* skey1[], const void* sptr1[], u32 size1, const utf8* skey2[], const void* sptr2[], u32 size2, CmpChar8 func);
void String_MergeKeyDsc_char16 (const utf16* tkey[], const void* tptr[], const utf16* skey1[], const void* sptr1[], u32 size1, const utf16* skey2[], const void* sptr2[], u32 size2, CmpChar16 func);
void String_MergeKeyDsc_char32 (const utf32* tkey[], const void* tptr[], const utf32* skey1[], const void* sptr1[], u32 size1, const utf32* skey2[], const void* sptr2[], u32 size2, CmpChar32 func);

//****************************************************************************//
//      Checks                                                                //
//****************************************************************************//

//============================================================================//
//      Check for duplicate values                                            //
//============================================================================//
u32 String_CheckDup_char8 (const utf8* array[], u32 size, CmpChar8 func);
u32 String_CheckDup_char16 (const utf16* array[], u32 size, CmpChar16 func);
u32 String_CheckDup_char32 (const utf32* array[], u32 size, CmpChar32 func);

//============================================================================//
//      Check for sort order                                                  //
//============================================================================//

// Check for ascending sort order
u32 String_CheckSortAsc_char8 (const utf8* array[], u32 size, CmpChar8 func);
u32 String_CheckSortAsc_char16 (const utf16* array[], u32 size, CmpChar16 func);
u32 String_CheckSortAsc_char32 (const utf32* array[], u32 size, CmpChar32 func);

// Check for descending sort order
u32 String_CheckSortDsc_char8 (const utf8* array[], u32 size, CmpChar8 func);
u32 String_CheckSortDsc_char16 (const utf16* array[], u32 size, CmpChar16 func);
u32 String_CheckSortDsc_char32 (const utf32* array[], u32 size, CmpChar32 func);

//****************************************************************************//
//      String hashing                                                        //
//****************************************************************************//

//============================================================================//
//      32-bit hash functions                                                 //
//============================================================================//
u32 String_Hash32_char8 (const utf8 string[]);
u32 String_Hash32_char16 (const utf16 string[]);
u32 String_Hash32_char32 (const utf32 string[]);

//============================================================================//
//      64-bit hash functions                                                 //
//============================================================================//
u64 String_Hash64_char8 (const utf8 string[]);
u64 String_Hash64_char16 (const utf16 string[]);
u64 String_Hash64_char32 (const utf32 string[]);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
