//
// Modified version of:
//
// std_exception.cpp
//
//       Copyright (c) Microsoft Corporation. All rights reserved.
//
// Definitions of the std::exception implementation functions.

#include <vcruntime_exception.h>

#include "lstd/memory/string.h"

using namespace LSTD_NAMESPACE;

_VCRTIMP extern "C" void __cdecl __std_exception_copy(_In_ __std_exception_data const *const from, _Out_ __std_exception_data *const to) {
    assert(to->_What == null && to->_DoFree == false);

    if (!from->_DoFree || !from->_What) {
        to->_What = from->_What;
        to->_DoFree = false;
        return;
    }

    size_t buffer_count = c_string_length(from->_What) + 1;

    char *buffer = allocate_array<char>(buffer_count);
    copy_memory(buffer, from->_What, buffer_count);

    to->_What = buffer;
    to->_DoFree = true;
}

_VCRTIMP extern "C" void __cdecl __std_exception_destroy(_In_ __std_exception_data *const data) {
    if (data->_DoFree) {
        ::LSTD_NAMESPACE::free((char *) data->_What);  // @Constcast
    }

    data->_DoFree = false;
    data->_What = null;
}
