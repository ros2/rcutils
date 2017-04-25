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

#ifndef COMMON_H_
#define COMMON_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcutils/error_handling.h"

#define RCUTILS_CHECK_ARGUMENT_FOR_NULL(argument, error_return_type, allocator) \
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(argument, #argument " argument is null", \
    return error_return_type, allocator)

#define RCUTILS_CHECK_FOR_NULL_WITH_MSG(value, msg, error_statement, allocator) \
  if (!(value)) { \
    RCUTILS_SET_ERROR_MSG(msg, allocator); \
    error_statement; \
  }

#if __cplusplus
}
#endif

#endif  // COMMON_H_
