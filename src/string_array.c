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

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/types/string_array.h"
#include "rcutils/types/rcutils_ret.h"

rcutils_string_array_t
rcutils_get_zero_initialized_string_array(void)
{
  static rcutils_string_array_t array = {
    .size = 0,
    .data = NULL,
  };
  array.allocator = rcutils_get_zero_initialized_allocator();
  return array;
}

rcutils_ret_t
rcutils_string_array_init(
  rcutils_string_array_t * string_array,
  size_t size,
  const rcutils_allocator_t * allocator)
{
  if (NULL == allocator) {
    RCUTILS_SET_ERROR_MSG("allocator is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  if (NULL == string_array) {
    RCUTILS_SET_ERROR_MSG("string_array is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  string_array->size = size;
  string_array->data = allocator->zero_allocate(size, sizeof(char *), allocator->state);
  if (NULL == string_array->data) {
    RCUTILS_SET_ERROR_MSG("failed to allocator string array");
    return RCUTILS_RET_BAD_ALLOC;
  }
  string_array->allocator = *allocator;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_string_array_fini(rcutils_string_array_t * string_array)
{
  if (NULL == string_array) {
    RCUTILS_SET_ERROR_MSG("string_array is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  if (NULL == string_array->data) {
    return RCUTILS_RET_OK;
  }

  rcutils_allocator_t * allocator = &string_array->allocator;
  if (!rcutils_allocator_is_valid(allocator)) {
    RCUTILS_SET_ERROR_MSG("allocator is invalid");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  size_t i;
  for (i = 0; i < string_array->size; ++i) {
    allocator->deallocate(string_array->data[i], allocator->state);
    string_array->data[i] = NULL;
  }
  allocator->deallocate(string_array->data, allocator->state);
  string_array->data = NULL;

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_string_array_cmp(
  const rcutils_string_array_t * lhs,
  const rcutils_string_array_t * rhs,
  int * res)
{
  if (NULL == lhs || NULL == rhs) {
    RCUTILS_SET_ERROR_MSG("string_array is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  if (NULL == lhs->data || NULL == rhs->data) {
    RCUTILS_SET_ERROR_MSG("string_array data is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  if (NULL == res) {
    RCUTILS_SET_ERROR_MSG("res argument is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  size_t smallest_size = lhs->size;
  if (rhs->size < smallest_size) {
    smallest_size = rhs->size;
  }

  for (size_t i = 0; i < smallest_size; ++i) {
    if (NULL == lhs->data[i] || NULL == rhs->data[i]) {
      RCUTILS_SET_ERROR_MSG("string_array element is null");
      return RCUTILS_RET_ERROR;
    }
    // Loop until we find a pair of strings that are not equal
    int strcmp_res = strcmp(lhs->data[i], rhs->data[i]);
    if (0 != strcmp_res) {
      *res = strcmp_res;
      return RCUTILS_RET_OK;
    }
  }

  // If all strings equal, compare array sizes
  *res = 0;
  if (lhs->size < rhs->size) {
    *res = -1;
  } else if (lhs->size > rhs->size) {
    *res = 1;
  }
  return RCUTILS_RET_OK;
}

#ifdef __cplusplus
}
#endif
