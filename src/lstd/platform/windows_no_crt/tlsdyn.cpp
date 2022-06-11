#include "common.h"
#include "lstd/platform/windows.h"

import lstd.os;
import lstd.context;

//
// These are taken from vcruntime/utility.cpp:
//

extern "C" IMAGE_DOS_HEADER __ImageBase;

// * Tests whether a PE image is located at the given image base.  Returns true if
// * the given image base potentially points to a loaded PE image; false otherwise.
static bool __cdecl is_potentially_valid_image_base(void *const image_base) noexcept {
    if (!image_base) {
        return false;
    }

    auto const dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(image_base);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        return false;
    }

    auto const nt_header_address = reinterpret_cast<PBYTE>(dos_header) + dos_header->e_lfanew;
    auto const nt_header         = reinterpret_cast<PIMAGE_NT_HEADERS64>(nt_header_address);
    if (nt_header->Signature != IMAGE_NT_SIGNATURE) {
        return false;
    }

    auto const optional_header = &nt_header->OptionalHeader;
    if (optional_header->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return false;
    }

    return true;
}

using uintptr_t = u64;

// * Given an RVA, finds the PE section in the pointed-to image that includes the
// * RVA.  Returns null if no such section exists or the section is not found.
static PIMAGE_SECTION_HEADER __cdecl find_pe_section(unsigned char *const image_base, uintptr_t const rva) noexcept {
    auto const dos_header        = reinterpret_cast<PIMAGE_DOS_HEADER>(image_base);
    auto const nt_header_address = reinterpret_cast<PBYTE>(dos_header) + dos_header->e_lfanew;
    auto const nt_header         = reinterpret_cast<PIMAGE_NT_HEADERS64>(nt_header_address);

    // * Find the section holding the RVA.  We make no assumptions here about the
    // * sort order of the section descriptors, though they always appear to be
    // * sorted by ascending section RVA.
    PIMAGE_SECTION_HEADER const first_section = IMAGE_FIRST_SECTION(nt_header);
    PIMAGE_SECTION_HEADER const last_section  = first_section + nt_header->FileHeader.NumberOfSections;
    for (auto it = first_section; it != last_section; ++it) {
        if (rva >= it->VirtualAddress && rva < it->VirtualAddress + it->Misc.VirtualSize) {
            return it;
        }
    }

    return nullptr;
}

extern "C" {

// * Tests whether a target address is located within the current PE image (the
// * image located at __ImageBase), that the target address is located in a proper
// * section of the image, and that the section in which it is located is not
// * writable.
bool __cdecl __scrt_is_nonwritable_in_current_image(void const *const target) {
    auto const target_address = reinterpret_cast<unsigned char const *>(target);
    auto const image_base     = reinterpret_cast<unsigned char *>(&__ImageBase);

    __try {
        // * Make sure __ImageBase is the address of a valid PE image.  This is
        // * likely an unnecessary check, since we should be executing in a
        // * normal image, but it is fast, this routine is rarely called, and the
        // * normal call is for security purposes.  If we don't have a PE image,
        // * return failure:
        if (!is_potentially_valid_image_base(image_base)) {
            return false;
        }

        // * Convert the target address to an RVA within the image and find the
        // * corresponding PE section.  Return failure if the target address is
        // * not found within the current image:
        uintptr_t const rva_target                 = target_address - image_base;
        PIMAGE_SECTION_HEADER const section_header = find_pe_section(image_base, rva_target);
        if (!section_header) {
            return false;
        }

        // * Check the section characteristics to see if the target address is
        // * located within a writable section, returning a failure if it is:
        if (section_header->Characteristics & IMAGE_SCN_MEM_WRITE) {
            return false;
        }

        return true;
    } __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION) {
        // * If any of the above operations failed, assume that we do not have a
        // * valid nonwritable address in the current image:
        return false;
    }
}

//
// Code taken from:
//

/***
*tlsdyn.cpp - Thread Local Storage dynamic initialization run-time support module
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Provide the TLS equivalent of DllMainCRTStartup, to be called by the
*       OS when a thread begins or ends.  On thread start, walk the list of
*       pointers to initialization routines for dynamically-initialized
*       __declspec(thread) variables.  On thread stop, walk the list of
*       destructors for __declspec(thread) variables, as registered at
*       initialization time.
*
****/

/*
 * Start and end sections for array of TLS callBacks to initialize individual
 * dynamically initialized __declspec(thread) items.  The actual array of
 * callbacks is constructed using .CRT$XDA, .CRT$XDC, .CRT$XDL, .CRT$XDU,
 * and .CRT$XDZ similar to the way global static initializers are done
 * for C++.  The C++ compiler will inject function pointers into .CRT$XD[CLU]
 * for each dynamically initialized __declspec(thread) variable, as well as
 * injecting a /include:__dyn_tls_init directive into the enclosing .obj,
 * to force inclusion of this support object.
 */

static _CRTALLOC(".CRT$XDA") _PVFV __xd_a = nullptr;

static _CRTALLOC(".CRT$XDZ") _PVFV __xd_z = nullptr;

// TRANSITION, toolset update
#pragma warning(push)
#pragma warning(disable : 5030)  // attribute '%s' is not recognized
/*
 * References to __tls_guard are generated by the compiler to detect if
 * dynamic initialization of TLS variables have run. This flag must be
 * set to 'true' before running the TLS initializers below.
 *
 * Note: [[msvc::no_tls_guard]] is not required here as it is known to
 * be statically initialized in this TU and all uses here will be unguarded
 * because of this. It is included for completion only but would be
 * necessary if a declaration is moved to a header and this variable
 * referenced from other files.
 */
[[msvc::no_tls_guard]] __declspec(thread) bool __tls_guard = false;
#pragma warning(pop)

/*
 * __dyn_tls_init - dynamically initialize __declspec(thread) variables
 *
 * Purpose:
 *      When any thread starts up, walk the array of function pointers found
 *      in sections .CRT$XD*, calling each non-NULL entry to dynamically
 *      initialize that thread's copy of a __declspec(thread) variable.
 *
 * Entry:
 *      This is called directly from the Windows loader code, with dwReason
 *      equal to one of DLL_PROCESS_ATTACH/DETACH or DLL_THREAD_ATTACH_DETACH.
 *
 * Exit:
 *      Returns TRUE always, though the loader code ignores the result.
 *
 * Notes:
 *      Only calls the initializers on DLL_THREAD_ATTACH and not
 *      DLL_PROCESS_ATTACH.  That's because the DLL_PROCESS_ATTACH call happens
 *      too early, before the CRT has been initialized in, e.g.,
 *      DllMainCRTStartup or mainCRTStartup.  Instead, the CRT init code will
 *      call here directly with a DLL_THREAD_ATTACH to initialize any
 *      __declspec(thread) variables in the primary thread at process startup.
 */

void WINAPI __dyn_tls_init(PVOID, DWORD dwReason, LPVOID) noexcept  // terminate on any C++ exception that leaves a
// namespace-scope thread-local initializer
// N4830 [basic.start.dynamic]/7
{
    if (dwReason != DLL_THREAD_ATTACH || __tls_guard == true)
        return;

    /*
     * Guard against repeated initialization by setting the tls guard tested
     * by the compiler before we run any initializers.
     */
    __tls_guard = true;

    /* prefast assumes we are overflowing __xd_a */
#pragma warning(push)
#pragma warning(disable : 26000)
    for (_PVFV *pfunc = &__xd_a + 1; pfunc != &__xd_z; ++pfunc) {
        if (*pfunc)
            (*pfunc)();
    }
#pragma warning(pop)

    // :ThreadsContext:
    //
    // We don't guarantee a valid context for threads.
    //
    // LSTD_NAMESPACE::platform_init_context();
    //
    // The reason for this decision: we can't know the parent of the thread
    // so we can't know which Context to copy.
    //
    // If you use lstd's API for creating a thread then we CAN know,
    // so in that case we provide a valid Context. However if you create
    // a thread with the raw OS API, then there is no way (as far as I know)
    // to get the parent thread. In that case we let it be zero filled
    // and let the user copy the Context manually.
    //
    // If in DEBUG then we fill it with a special value to catch bugs more
    // easily when reading values from the invalid context.
    //

    extern void *MainContext;
    if ((void *) &Context != MainContext) {
        LSTD_NAMESPACE::memset((byte *) &Context, DEAD_LAND_FILL, sizeof(Context));
    }
}

/*
 * Define an initialized callback function pointer, so CRT startup code knows
 * we have dynamically initialized __declspec(thread) variables that need to
 * be initialized at process startup for the primary thread.
 */

extern const PIMAGE_TLS_CALLBACK __dyn_tls_init_callback = __dyn_tls_init;

/*
 * Enter a callback function pointer into the .CRT$XL* array, which is the
 * callback array pointed to by the IMAGE_TLS_DIRECTORY in the PE header, so
 * the OS knows we want to be notified on each thread startup/shutdown.
 */

static _CRTALLOC(".CRT$XLC") PIMAGE_TLS_CALLBACK __xl_c = __dyn_tls_init;

/*
 * Helper function invoked by the compiler for on-demand initialization of
 * TLS variables when the initializers have not run because a DLL is dynamically
 * loaded after the thread(s) have started.
 */
void __cdecl __dyn_tls_on_demand_init() noexcept {
    __dyn_tls_init(nullptr, DLL_THREAD_ATTACH, nullptr);
}

}  // extern "C"
