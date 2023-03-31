#include "lstd/os/windows/api.h"

extern "C" {
// This is needed for SEH exceptions
EXCEPTION_DISPOSITION
__C_specific_handler(struct _EXCEPTION_RECORD *ExceptionRecord,
                     void *EstablisherFrame, struct _CONTEXT *ContextRecord,
                     struct _DISPATCHER_CONTEXT *DispatcherContext) {
  typedef EXCEPTION_DISPOSITION Function(struct _EXCEPTION_RECORD *, void *,
                                         struct _CONTEXT *,
                                         _DISPATCHER_CONTEXT *);
  static Function *FunctionPtr;

  if (!FunctionPtr) {
    HMODULE Library = LoadLibraryW(L"msvcrt.dll");
    FunctionPtr = (Function *)GetProcAddress(Library, "__C_specific_handler");
  }

  return FunctionPtr(ExceptionRecord, EstablisherFrame, ContextRecord,
                     DispatcherContext);
}
}
