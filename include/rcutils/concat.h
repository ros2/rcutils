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

#ifndef RCUTILS__CONCAT_H_
#define RCUTILS__CONCAT_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcutils/visibility_control.h"

/// Return a newly allocated string that contains lhs, followed by delimiter, followed by rhs.
/**
 * This function allocates memory and returns it to the caller.  It is up to the
 * caller to release the memory once it is done with it by calling `free`.
 *
 * \return char * concatenated string on success
 *         NULL on invalid arguments
 *         NULL on failure
 */
RCUTILS_PUBLIC
char *
rcutils_concat(const char * lhs, const char * rhs, const char * delimiter);

#if __cplusplus
}
#endif

#endif  // RCUTILS__CONCAT_H_
