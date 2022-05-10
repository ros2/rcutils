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

#ifndef RCUTILS__MEMSET_H_
#define RCUTILS__MEMSET_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1  // indicate we would like memset_s if available
#endif
#include <string.h>

#if __STDC_LIB_EXT1__

// In this case we have memset_s so just use that.

typedef errno_t rcutils_errno_t;

#define RCUTILS_DETAIL_MEMSET(dest, destsz, ch, count) \
  memset_s(dest, destsz, ch, count)

// #elif __GNUC__

// // In this case we don't have memset_s, but we do have gnu extensions, so
// // use a Statement Expression to avoid a function call.

#else

// In this case we don't have memset_s, and we do not have gnu extensions, so
// we have to call a function, so we'll use a static inlined function.

typedef int rcutils_errno_t;

static
inline
rcutils_errno_t
_rcutils_detail_memset(void * dest, size_t destsz, int ch, size_t count)
{
  (void)destsz;
  memset(dest, ch, count);
  return 0;
}

#define RCUTILS_DETAIL_MEMSET(dest, destsz, ch, count) \
  _rcutils_detail_memset(dest, destsz, ch, count)

#endif

/// Copy a fill character into memory at the given address.
/**
 * This just wraps the memset_s call or emulates it, if it is not available.
 *
 * When memset_s is not available then a custom logic is used instead.
 *
 * When emulating memset_s, errno_t is typedef'ed to int.
 * In either case rcutils_errno_t is set as the right return type.
 *
 * See https://en.cppreference.com/w/c/string/byte/memset
 */
#define RCUTILS_MEMSET(dest, destsz, ch, count) \
  RCUTILS_DETAIL_MEMSET(dest, destsz, ch, count)

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__MEMSET_H_
