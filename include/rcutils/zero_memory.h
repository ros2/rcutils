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

#ifndef RCUTILS__ZERO_MEMORY_H_
#define RCUTILS__ZERO_MEMORY_H_

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

#define RCUTILS_DETAIL_ZERO_MEMORY(dest, destsz) \
  ZeroMemory(dest, destsz)

#else

// The macro is surrounded by `do { .. } while (0)` to implement
// the standard C macro idiom to make the macro safe in all contexts; see
// http://c-faq.com/cpp/multistmt.html for more information.
//
// RCUTILS_MEMSET() cannot fail when the destsz and count, second and third
// arguments respectively, are equal, so we ignore the return code.
#define RCUTILS_DETAIL_ZERO_MEMORY(dest, destsz) \
  do { \
    (void)RCUTILS_MEMSET(dest, destsz, 0, destsz); \
  } while (0)

#endif

/// Fill memory with zeros.
/**
 * This wraps RCUTILS_MEMSET() macro, which itself wraps memset_s/memset, and
 * on Windows it uses ZeroMemory().
 * This macro can sometimes be optimized out by the compiler, unless memset_s
 * is used under the hood.
 * Use RCUTILS_ZERO_MEMORY_EXPLICIT() to avoid this optimization in all cases,
 * if that is required.
 *
 * \sa https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366920(v=vs.85)
 * \sa https://en.cppreference.com/w/c/string/byte/memset
 *
 * \param destination void * pointer to the start of the memory to be zeroed
 * \param destination_size size_t length of the memory to be zeroed, starting from destination
 * \returns void
 */
#define RCUTILS_ZERO_MEMORY(destination, destination_size) \
  RCUTILS_DETAIL_ZERO_MEMORY(destination, destination_size)

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__ZERO_MEMORY_H_
