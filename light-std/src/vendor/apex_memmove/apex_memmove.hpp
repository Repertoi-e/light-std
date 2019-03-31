#pragma once

//	apex_memmove written by Trevor Herselman in 2014

#include "../../lstd/common.hpp"

LSTD_BEGIN_NAMESPACE

extern void *(*copy_memory)(void *dest, const void *src, size_t num);
extern void *(*move_memory)(void *dest, const void *src, size_t num);

LSTD_END_NAMESPACE