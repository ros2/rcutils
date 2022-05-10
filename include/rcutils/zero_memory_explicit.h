// Copyright 2022 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \file

#ifndef RCUTILS__ZERO_MEMORY_EXPLICIT_H_
#define RCUTILS__ZERO_MEMORY_EXPLICIT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1  // indicate we would like memset_s if available
#endif
#include <string.h>

#include "rcutils/memset.h"

#if defined(_WIN32)

#define RCUTILS_DETAIL_ZERO_MEMORY_EXPLICIT(dest, destsz) \
  SecureZeroMemory(dest, destsz)

#elif __STDC_LIB_EXT1__

// The macro is surrounded by `do { .. } while (0)` to implement
// the standard C macro idiom to make the macro safe in all contexts; see
// http://c-faq.com/cpp/multistmt.html for more information.
//
// RCUTILS_MEMSET() cannot fail when the destsz and count, second and third
// arguments respectively, are equal, so we ignore the return code.
//
// When __STDC_LIB_EXT1__ is defined, RCUTILS_MEMSET() will use memset_s
// which provides the guarantee against optimization.
#define RCUTILS_DETAIL_ZERO_MEMORY_EXPLICIT(dest, destsz) \
  do { \
    (void)RCUTILS_MEMSET(dest, destsz, 0, destsz); \
  } while (0)

#else

// The macro is surrounded by `do { .. } while (0)` to implement
// the standard C macro idiom to make the macro safe in all contexts; see
// http://c-faq.com/cpp/multistmt.html for more information.
//
// Zeroing is done with a custom while-loop that uses volatile to avoid
// being optimized.
// This is a last resort, but should work on most compilers.
#define RCUTILS_DETAIL_ZERO_MEMORY_EXPLICIT(dest, destsz) \
  do { \
    volatile char * dest_as_char_ptr = (char *)dest; \
    size_t local_destsz = destsz; \
    while (local_destsz--) { \
      *dest_as_char_ptr++ = 0x0; \
    } \
  } while (0)

#endif

/// Fill memory with zeros, even when the code is optimized.
/**
 * This wraps RCUTILS_MEMSET() macro, which itself wraps memset_s, when
 * memset_s is available, otherwise it uses SecureZeroMemory() on Windows and
 * if not Windows and memset_s is not available, then a custom while-loop is
 * used with a volatile pointer to prevent optimizations.
 *
 * \sa https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85)
 * \sa https://en.cppreference.com/w/c/string/byte/memset
 * \sa https://en.cppreference.com/w/c/language/volatile
 *
 * \param destination void * pointer to the start of the memory to be zeroed
 * \param destination_size size_t length of the memory to be zeroed, starting from destination
 * \returns void
 */
#define RCUTILS_ZERO_MEMORY_EXPLICIT(destination, destination_size) \
  RCUTILS_DETAIL_ZERO_MEMORY_EXPLICIT(destination, destination_size)

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__ZERO_MEMORY_EXPLICIT_H_
