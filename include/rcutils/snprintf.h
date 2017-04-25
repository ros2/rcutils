// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS__SNPRINTF_H_
#define RCUTILS__SNPRINTF_H_

#if __cplusplus
extern "C"
{
#endif

#include <stdio.h>

/// Format a string.
/**
 * This function just wraps snprintf() as defined in C11 in a portable way.
 *
 * On Windows this defaults to the _TRUNCATE behavior of _snprintf_s().
 *
 * \see snprintf()
 */
#ifndef _WIN32
#define rcutils_snprintf snprintf
#else
#define rcutils_snprintf(buffer, buffer_size, format, ...) \
  _snprintf_s(buffer, buffer_size, _TRUNCATE, format, __VA_ARGS__)
#endif

/// Format a string with va_list for arguments, see rcutils_snprintf().
#ifndef _WIN32
  #define rcutils_vsnprintf vsnprintf
#else
  #define rcutils_vsnprintf(buffer, buffer_size, format, ...) \
  _vsnprintf_s(buffer, buffer_size, _TRUNCATE, format, __VA_ARGS__)
#endif

#if __cplusplus
}
#endif

#endif  // RCUTILS__SNPRINTF_H_
