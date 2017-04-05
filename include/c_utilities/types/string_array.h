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

#ifndef C_UTILITIES__TYPES__STRING_ARRAY_H_
#define C_UTILITIES__TYPES__STRING_ARRAY_H_

#if __cplusplus
extern "C"
{
#endif

#include <string.h>

#include "c_utilities/types/utilities_ret.h"
#include "c_utilities/visibility_control.h"

typedef struct C_UTILITIES_PUBLIC_TYPE string_array_t
{
  size_t size;
  char ** data;
} string_array_t;

/// Returns an empty string array struct
/*
 * This function returns an empty and zero initialized string array struct.
 * Calling utilities_string_array_fini on this instance leads to undefined
 * behavior.
 */
C_UTILITIES_PUBLIC
string_array_t
utilities_get_zero_initialized_string_array();

/// Return a pre-initialized string array struct
/**
 * This function returns a pre-initiliazed string array struct
 * which allocates the data array to the provided size and sets
 * it to NULL.
 * Setting these values later on manually requires a manual memory
 * allocation of the individual data[i] char pointer, such as memcpy or strdup.
 *
 * Example:
 * string_array_t sa2 = utilities_get_pre_initialized_string_array(2);
 * sa2.data[0] = strdup("Hello");
 * sa2.data[1] = strdup("World");
 */
C_UTILITIES_PUBLIC
string_array_t
utilities_get_pre_initialized_string_array(size_t size);

/// Frees the allocated string array structs
/**
 * This function destroys the string array instance
 * and frees all allocated memory within.
 */
C_UTILITIES_PUBLIC
utilities_ret_t
utilities_string_array_fini(string_array_t * array);

#if __cplusplus
}
#endif

#endif  // C_UTILITIES__TYPES__STRING_ARRAY_H_
