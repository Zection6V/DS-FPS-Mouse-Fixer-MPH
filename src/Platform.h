#pragma once

#ifndef UNICODE
#  define UNICODE
#endif
#ifndef _UNICODE
#  define _UNICODE
#endif
#if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN 1
#endif

#include <windows.h>
#include <winuser.h>

#if defined(_MSC_VER)
#  define DS_FORCEINLINE __forceinline
#elif defined(__GNUC__)
#  define DS_FORCEINLINE inline __attribute__((always_inline))
#else
#  define DS_FORCEINLINE inline
#endif
