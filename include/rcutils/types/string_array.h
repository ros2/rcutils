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

#ifndef RCUTILS__TYPES__STRING_ARRAY_H_
#define RCUTILS__TYPES__STRING_ARRAY_H_

#if __cplusplus
extern "C"
{
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

typedef struct RCUTILS_PUBLIC_TYPE rcutils_string_array_t
{
  size_t size;
  char ** data;
} rcutils_string_array_t;

/// Return an empty string array struct.
/**
 * This function returns an empty and zero initialized string array struct.
 * Calling rcutils_string_array_fini() on any non-initialized instance leads
 * to undefined behavior.
 * Every instance of string_array_t has to either be zero_initialized with this
 * function or manually allocated.
 *
 * Example:
 *
 * ```c
 * rcutils_string_array_t foo;
 * rcutils_string_array_fini(&foo); // undefined behavior!
 *
 * rcutils_string_array_t bar = rcutils_get_zero_initialized_string_array();
 * rcutils_string_array_fini(&bar); // ok
 * ```
 */
RCUTILS_PUBLIC
rcutils_string_array_t
rcutils_get_zero_initialized_string_array();

/// Return a pre-initialized string array struct.
/**
 * This function returns a pre-initialized string array struct
 * which allocates the data array to the provided size and sets
 * it to NULL.
 * Setting these values later on manually requires a manual memory
 * allocation of the individual data[i] char pointer, such as memcpy or strdup.
 *
 * Example:
 *
 * ```c
 * rcutils_string_array_t sa2 = rcutils_get_pre_initialized_string_array(2);
 * sa2.data[0] = strdup("Hello");
 * sa2.data[1] = strdup("World");
 * ```
 */
RCUTILS_PUBLIC
rcutils_string_array_t
rcutils_get_pre_initialized_string_array(size_t size, const rcutils_allocator_t * allocator);

/// Free the allocated string array struct.
/**
 * This function destroys the string array instance
 * and frees all allocated memory within.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_string_array_fini(rcutils_string_array_t * array, const rcutils_allocator_t * allocator);

#if __cplusplus
}
#endif

#endif  // RCUTILS__TYPES__STRING_ARRAY_H_
