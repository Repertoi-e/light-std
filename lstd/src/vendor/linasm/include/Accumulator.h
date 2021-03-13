/*                                                                 Accumulator.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                             ACCUMULATING BUFFER                              #
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
//****************************************************************************//
//      Accumulating buffer class                                             //
//****************************************************************************//
class Accumulator
{
private:
	void	*buffer;	// Pointer to memory buffer
	u32	capacity;	// Capacity of the buffer (auto extended if required)
	u32	size;		// Current buffer size

public:

//****************************************************************************//
//      Constructor                                                           //
//****************************************************************************//
Accumulator (u32 capacity);

//****************************************************************************//
//      Copy constructor                                                      //
//****************************************************************************//
Accumulator (const Accumulator &source);

//****************************************************************************//
//      Destructor                                                            //
//****************************************************************************//
~Accumulator (void);

//****************************************************************************//
//      Accumulator functions                                                 //
//****************************************************************************//
void* Reserve (u32 size);
bool Fill (u32 size);
const void* Data (void) const;
void Clear (void);

//****************************************************************************//
//      Accumulator properties                                                //
//****************************************************************************//
u32 Capacity (void) const;
u32 Size (void) const;
bool IsEmpty (void) const;
bool IsInit (void) const;
};
# else
/*
################################################################################
#       C prototypes                                                           #
################################################################################
*/
//****************************************************************************//
//      Accumulating buffer structure                                         //
//****************************************************************************//
struct Accumulator
{
	void	*buffer;	// Pointer to memory buffer
	u32	capacity;	// Capacity of the buffer (auto extended if required)
	u32	size;		// Current buffer size
};

//****************************************************************************//
//      Init accumulator structure                                            //
//****************************************************************************//
void Accumulator_InitAccumulator (struct Accumulator *accumulator, u32 capacity);

//****************************************************************************//
//      Copy accumulator structure                                            //
//****************************************************************************//
void Accumulator_CopyAccumulator (struct Accumulator *accumulator, const struct Accumulator *source);

//****************************************************************************//
//      Free accumulator structure                                            //
//****************************************************************************//
void Accumulator_FreeAccumulator (struct Accumulator *accumulator);

//****************************************************************************//
//      Accumulator functions                                                 //
//****************************************************************************//
void* Accumulator_Reserve (struct Accumulator *accumulator, u32 size);
bool Accumulator_Fill (struct Accumulator *accumulator, u32 size);
const void* Accumulator_Data (const struct Accumulator *accumulator);
void Accumulator_Clear (struct Accumulator *accumulator);

//****************************************************************************//
//      Accumulator properties                                                //
//****************************************************************************//
u32 Accumulator_Capacity (const struct Accumulator *accumulator);
u32 Accumulator_Size (const struct Accumulator *accumulator);
bool Accumulator_IsEmpty (const struct Accumulator *accumulator);
bool Accumulator_IsInit (const struct Accumulator *accumulator);

# endif
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
