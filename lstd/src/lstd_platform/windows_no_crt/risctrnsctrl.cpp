#include <eh.h>
#include <ehdata.h>
#include <ehdata4.h>

#define RENAME_EH_EXTERN_HYBRID(x) x
#define RENAME_BASE_PTD(x) x

// Taken from "vcruntime_internal.h":
typedef struct RENAME_BASE_PTD(__vcrt_ptd) {
    // C++ Exception Handling (EH) state
    unsigned long _NLG_dwCode;       // Required by NLG routines
    unexpected_handler _unexpected;  // unexpected() routine
    void *_translator;               // S.E. translator
    void *_purecall;                 // called when pure virtual happens
    void *_curexception;             // current exception
    void *_curcontext;               // current exception context
    int _ProcessingThrow;            // for uncaught_exception
    void *_curexcspec;               // for handling exceptions thrown from std::unexpected
    int _cxxReThrow;                 // true if it's a rethrown C++ exception

#if defined _M_X64 || defined _M_ARM || defined _M_ARM64 || defined _M_HYBRID
    void *_pExitContext;
    void *_pUnwindContext;
    void *_pFrameInfoChain;
    uintptr_t _ImageBase;
    uintptr_t _ThrowImageBase;
    void *_pForeignException;
    int _CatchStateInParent;  // Used to link together the catch funclet with the parent. During dispatch contains state associated
                              // with catch in the parent. During unwind represents the current unwind state that is resumed to
                              // during collided unwind and used to look for handlers of the throwing dtor.
#elif defined _M_IX86
    void *_pFrameInfoChain;
#endif

} RENAME_BASE_PTD(__vcrt_ptd);

__vcrt_ptd *__cdecl __vcrt_getptd(void);
#define _ImageBase (RENAME_BASE_PTD(__vcrt_getptd)()->_ImageBase)

//
// Prototype for the internal handler
//
// ehhelpers.h
template <class T>
EXCEPTION_DISPOSITION __InternalCxxFrameHandler(
    EHExceptionRecord *pExcept,
    EHRegistrationNode *pRN,
    CONTEXT *pContext,
    DispatcherContext *pDC,
    typename T::FuncInfo *pFuncInfo,
    int CatchDepth,
    EHRegistrationNode *pMarkerRN,
    BOOLEAN recursive);

extern "C" DECLSPEC_GUARD_SUPPRESS EXCEPTION_DISPOSITION __cdecl RENAME_EH_EXTERN_HYBRID(__CxxFrameHandler4)(
    EHExceptionRecord *pExcept,  // Information for this exception
    EHRegistrationNode RN,       // Dynamic information for this frame
    CONTEXT *pContext,           // Context info
    DispatcherContext *pDC       // More dynamic info for this frame
) {
    FH4::FuncInfo4 FuncInfo;
    EXCEPTION_DISPOSITION result;
    EHRegistrationNode EstablisherFrame = RN;

    _ImageBase = pDC->ImageBase;
#ifdef _ThrowImageBase
    _ThrowImageBase = (uintptr_t) pExcept->params.pThrowImageBase;
#endif
    PBYTE buffer = (PBYTE)(_ImageBase + *(PULONG) pDC->HandlerData);

    FH4::DecompFuncInfo(buffer, FuncInfo, pDC->ImageBase, pDC->FunctionEntry->BeginAddress);

    result = __InternalCxxFrameHandler<RENAME_EH_EXTERN(__FrameHandler4)>(pExcept, &EstablisherFrame, pContext, pDC, &FuncInfo, 0, nullptr, FALSE);
    return result;
}