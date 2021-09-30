#pragma once

#if !defined LSTD_DONT_DEFINE_STD
#define va_start __crt_va_start
#define va_arg __crt_va_arg
#define va_end __crt_va_end
#define va_copy(destination, source) ((destination) = (source))
#endif
