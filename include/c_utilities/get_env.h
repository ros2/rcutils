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

#ifndef C_UTILITIES__GET_ENV_H_
#define C_UTILITIES__GET_ENV_H_

#if __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "c_utilities/macros.h"
#include "c_utilities/visibility_control.h"

/// Retrieve the value of the given environment variable if it exists, or "".
/* The returned c-string is only valid until the next time this function is
 * called, because it is a direct pointer to the static storage.
 * The variable env_value populated by this function should never have free() called on it.
 * If the environment variable is not set, an empty string will be returned.
 *
 * Environment variables will be truncated at 2048 characters on Windows.
 *
 * This function may allocate heap memory.
 * This function is not thread-safe.
 * This function is not lock-free.
 *
 * \param[in] env_name the name of the environment variable
 * \param[out] env_value pointer to the value cstring, or "" if unset
 * \return NULL on success (success can be returning an empty string)
 *         error string on failure
 */
C_UTILITIES_WARN_UNUSED
C_UTILITIES_PUBLIC
const char *
utilities_get_env(const char * env_name, const char ** env_value);

#if __cplusplus
}
#endif

#endif  // C_UTILITIES__GET_ENV_H_
