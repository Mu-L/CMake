// SPDX-License-Identifier: 0BSD

///////////////////////////////////////////////////////////////////////////////
//
/// \file       sysdefs.h
/// \brief      Common includes, definitions, system-specific things etc.
///
/// This file is used also by the lzma command line tool, that's why this
/// file is separate from common.h.
//
//  Author:     Lasse Collin
//
///////////////////////////////////////////////////////////////////////////////

#ifndef LZMA_SYSDEFS_H
#define LZMA_SYSDEFS_H

#if defined(_MSC_VER)
# pragma warning(push,1)
# pragma warning(disable: 4028) /* formal parameter different from decl */
# pragma warning(disable: 4142) /* benign redefinition of type */
# pragma warning(disable: 4761) /* integral size mismatch in argument */
#endif

//////////////
// Includes //
//////////////

#include "config.h"

// This #define ensures that C99 and POSIX compliant stdio functions are
// available with MinGW-w64 (both 32-bit and 64-bit). Modern MinGW-w64 adds
// this automatically, for example, when the compiler is in C99 (or later)
// mode when building against msvcrt.dll. It still doesn't hurt to be explicit
// that we always want this and #define this unconditionally.
//
// With Universal CRT (UCRT) this is less important because UCRT contains
// C99-compatible stdio functions. It's still nice to #define this as UCRT
// doesn't support the POSIX thousand separator flag in printf (like "%'u").
#ifdef __MINGW32__
#	define __USE_MINGW_ANSI_STDIO 1
#endif

// size_t and NULL
#include <stddef.h>

#ifdef HAVE_INTTYPES_H
#	include <inttypes.h>
#endif

// C99 says that inttypes.h always includes stdint.h, but some systems
// don't do that, and require including stdint.h separately.
#ifdef HAVE_STDINT_H
#	include <stdint.h>
#endif

// Some pre-C99 systems have SIZE_MAX in limits.h instead of stdint.h. The
// limits are also used to figure out some macros missing from pre-C99 systems.
#include <limits.h>


#if defined(_MSC_VER) && (_MSC_VER < 1310)
#  define UINT64_C(n) n ## ui64
#endif


// Be more compatible with systems that have non-conforming inttypes.h.
// We assume that int is 32-bit and that long is either 32-bit or 64-bit.
// Full Autoconf test could be more correct, but this should work well enough.
// Note that this duplicates some code from lzma.h, but this is better since
// we can work without inttypes.h thanks to Autoconf tests.
#ifndef UINT32_C
#	if UINT_MAX != 4294967295U
#		error UINT32_C is not defined and unsigned int is not 32-bit.
#	endif
#	define UINT32_C(n) n ## U
#endif
#ifndef UINT32_MAX
#	define UINT32_MAX UINT32_C(4294967295)
#endif
#ifndef PRIu32
#	define PRIu32 "u"
#endif
#ifndef PRIx32
#	define PRIx32 "x"
#endif
#ifndef PRIX32
#	define PRIX32 "X"
#endif

#if ULONG_MAX == 4294967295UL
#	ifndef UINT64_C
#		define UINT64_C(n) n ## ULL
#	endif
#	ifndef PRIu64
#		define PRIu64 "llu"
#	endif
#	ifndef PRIx64
#		define PRIx64 "llx"
#	endif
#	ifndef PRIX64
#		define PRIX64 "llX"
#	endif
#else
#	ifndef UINT64_C
#		define UINT64_C(n) n ## UL
#	endif
#	ifndef PRIu64
#		define PRIu64 "lu"
#	endif
#	ifndef PRIx64
#		define PRIx64 "lx"
#	endif
#	ifndef PRIX64
#		define PRIX64 "lX"
#	endif
#endif
#ifndef UINT64_MAX
#	define UINT64_MAX UINT64_C(18446744073709551615)
#endif

// Incorrect(?) SIZE_MAX:
//   - Interix headers typedef size_t to unsigned long,
//     but a few lines later define SIZE_MAX to INT32_MAX.
//   - SCO OpenServer (x86) headers typedef size_t to unsigned int
//     but define SIZE_MAX to INT32_MAX.
#if defined(__INTERIX) || defined(_SCO_DS)
#	undef SIZE_MAX
#endif

// The code currently assumes that size_t is either 32-bit or 64-bit.
#ifndef SIZE_MAX
#	if SIZEOF_SIZE_T == 4
#		define SIZE_MAX UINT32_MAX
#	elif SIZEOF_SIZE_T == 8
#		define SIZE_MAX UINT64_MAX
#	else
#		error size_t is not 32-bit or 64-bit
#	endif
#endif
#if SIZE_MAX != UINT32_MAX && SIZE_MAX != UINT64_MAX
#	error size_t is not 32-bit or 64-bit
#endif

#include <stdlib.h>
#include <assert.h>

// Pre-C99 systems lack stdbool.h. All the code in XZ Utils must be written
// so that it works with fake bool type, for example:
//
//    bool foo = (flags & 0x100) != 0;
//    bool bar = !!(flags & 0x100);
//
// This works with the real C99 bool but breaks with fake bool:
//
//    bool baz = (flags & 0x100);
//
#ifdef HAVE_STDBOOL_H
#	include <stdbool.h>
#else
#	if ! HAVE__BOOL
typedef unsigned char _Bool;
#	endif
#	define bool _Bool
#	define false 0
#	define true 1
#	define __bool_true_false_are_defined 1
#endif

#include <string.h>

// Visual Studio 2013 update 2 supports only __inline, not inline.
// MSVC v19.0 / VS 2015 and newer support both.
//
// MSVC v19.27 (VS 2019 version 16.7) added support for restrict.
// Older ones support only __restrict.
#ifdef _MSC_VER
#	if _MSC_VER < 1900 && !defined(inline)
#		define inline __inline
#	endif
#	if _MSC_VER < 1927 && !defined(restrict)
#		define restrict __restrict
#	endif
#elif defined(__EDG__) && defined(__LCC__)
#	ifndef inline
#		define inline __inline
#	endif
#	ifndef restrict
#		define restrict __restrict
#	endif
#endif

////////////
// Macros //
////////////

#undef memzero
#define memzero(s, n) memset(s, 0, n)

// NOTE: Avoid using MIN() and MAX(), because even conditionally defining
// those macros can cause some portability trouble, since on some systems
// the system headers insist defining their own versions.
#define my_min(x, y) ((x) < (y) ? (x) : (y))
#define my_max(x, y) ((x) > (y) ? (x) : (y))

#ifndef ARRAY_SIZE
#	define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

#if defined(__GNUC__) \
		&& ((__GNUC__ == 4 && __GNUC_MINOR__ >= 3) || __GNUC__ > 4)
#	define lzma_attr_alloc_size(x) __attribute__((__alloc_size__(x)))
#else
#	define lzma_attr_alloc_size(x)
#endif

#endif
