// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#if __cplusplus
extern "C"
{
#endif

#include "rcutils/time.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/snprintf.h"

RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_time_point_value_as_nanoseconds_string(
  const rcutils_time_point_value_t * timepoint,
  char * str,
  size_t str_size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(timepoint, RCUTILS_RET_INVALID_ARGUMENT, allocator)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(str, RCUTILS_RET_INVALID_ARGUMENT, allocator)
  if (rcutils_snprintf(str, str_size, "%.19" PRId64, *timepoint) <= 0) {
    RCUTILS_SET_ERROR_MSG("failed to format timepoint into string as nanoseconds", allocator)
    return RCUTILS_RET_ERROR;
  }
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_time_point_value_as_seconds_string(
  const rcutils_time_point_value_t * timepoint,
  char * str,
  size_t str_size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(timepoint, RCUTILS_RET_INVALID_ARGUMENT, allocator)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(str, RCUTILS_RET_INVALID_ARGUMENT, allocator)
  // break into two parts to avoid floating point error
  uint64_t seconds = *timepoint / (1000 * 1000 * 1000);
  uint64_t nanoseconds = *timepoint % (1000 * 1000 * 1000);
  if (rcutils_snprintf(str, str_size, "%.10" PRId64 ".%.9" PRId64, seconds, nanoseconds) <= 0) {
    RCUTILS_SET_ERROR_MSG("failed to format timepoint into string as float seconds", allocator)
    return RCUTILS_RET_ERROR;
  }
  return RCUTILS_RET_OK;
}

#if __cplusplus
}
#endif
