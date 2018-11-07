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

#include "rcutils/error_handling.h"
#include "rcutils/types/uint8_array.h"

rcutils_uint8_array_t
rcutils_get_zero_initialized_uint8_array(void)
{
  static rcutils_uint8_array_t uint8_array = {
    .buffer = NULL,
    .buffer_length = 0u,
    .buffer_capacity = 0u
  };
  uint8_array.allocator = rcutils_get_zero_initialized_allocator();
  return uint8_array;
}

rcutils_ret_t
rcutils_uint8_array_init(
  rcutils_uint8_array_t * uint8_array,
  size_t buffer_capacity,
  const rcutils_allocator_t * allocator)
{
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    uint8_array,
    "uint8 array pointer is null",
    return RCUTILS_RET_ERROR);

  if (!rcutils_allocator_is_valid(allocator)) {
    RCUTILS_SET_ERROR_MSG("uint8 array has no valid allocator");
    return RCUTILS_RET_ERROR;
  }

  uint8_array->buffer_length = 0;
  uint8_array->buffer_capacity = buffer_capacity;
  uint8_array->allocator = *allocator;

  if (buffer_capacity > 0u) {
    uint8_array->buffer = (uint8_t *)allocator->allocate(
      buffer_capacity * sizeof(uint8_t), allocator->state);
    RCUTILS_CHECK_FOR_NULL_WITH_MSG(
      uint8_array->buffer,
      "failed to allocate memory for uint8 array",
      return RCUTILS_RET_BAD_ALLOC);
  }

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_uint8_array_fini(rcutils_uint8_array_t * uint8_array)
{
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    uint8_array,
    "uint8 array pointer is null",
    return RCUTILS_RET_ERROR);

  rcutils_allocator_t * allocator = &uint8_array->allocator;
  if (!rcutils_allocator_is_valid(allocator)) {
    RCUTILS_SET_ERROR_MSG("uint8 array has no valid allocator");
    return RCUTILS_RET_ERROR;
  }

  allocator->deallocate(uint8_array->buffer, allocator->state);
  uint8_array->buffer = NULL;
  uint8_array->buffer_length = 0u;
  uint8_array->buffer_capacity = 0u;

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_uint8_array_resize(rcutils_uint8_array_t * uint8_array, size_t new_size)
{
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    uint8_array,
    "uint8 array pointer is null",
    return RCUTILS_RET_ERROR);

  rcutils_allocator_t * allocator = &uint8_array->allocator;
  if (!rcutils_allocator_is_valid(allocator)) {
    RCUTILS_SET_ERROR_MSG("uint8 array has no valid allocator");
    return RCUTILS_RET_ERROR;
  }

  if (new_size == uint8_array->buffer_capacity) {
    // nothing to do here
    return RCUTILS_RET_OK;
  }

  uint8_array->buffer = rcutils_reallocf(
    uint8_array->buffer, new_size * sizeof(uint8_t), allocator);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    uint8_array->buffer,
    "failed to reallocate memory for uint8 array",
    return RCUTILS_RET_BAD_ALLOC);

  uint8_array->buffer_capacity = new_size;
  if (new_size < uint8_array->buffer_length) {
    uint8_array->buffer_length = new_size;
  }

  return RCUTILS_RET_OK;
}
