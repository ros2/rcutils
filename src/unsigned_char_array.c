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
#include "rcutils/types/unsigned_char_array.h"

rcutils_unsigned_char_array_t
rcutils_get_zero_initialized_unsigned_char_array(void)
{
  static rcutils_unsigned_char_array_t char_array = {
    .buffer = NULL,
    .buffer_length = 0u,
    .buffer_capacity = 0u
  };
  char_array.allocator = rcutils_get_zero_initialized_allocator();
  return char_array;
}

rcutils_ret_t
rcutils_unsigned_char_array_init(
  rcutils_unsigned_char_array_t * char_array,
  size_t buffer_capacity,
  const rcutils_allocator_t * allocator)
{
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    char_array,
    "char array pointer is null",
    return RCUTILS_RET_ERROR);

  if (!rcutils_allocator_is_valid(allocator)) {
    RCUTILS_SET_ERROR_MSG("char array has no valid allocator");
    return RCUTILS_RET_ERROR;
  }

  char_array->buffer_length = 0;
  char_array->buffer_capacity = buffer_capacity;
  char_array->allocator = *allocator;

  if (buffer_capacity > 0u) {
    char_array->buffer = (unsigned char *)allocator->allocate(
      buffer_capacity * sizeof(unsigned char), allocator->state);
    RCUTILS_CHECK_FOR_NULL_WITH_MSG(
      char_array->buffer,
      "failed to allocate memory for char array",
      return RCUTILS_RET_BAD_ALLOC);
  }

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_unsigned_char_array_fini(rcutils_unsigned_char_array_t * char_array)
{
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    char_array,
    "char array pointer is null",
    return RCUTILS_RET_ERROR);

  rcutils_allocator_t * allocator = &char_array->allocator;
  if (!rcutils_allocator_is_valid(allocator)) {
    RCUTILS_SET_ERROR_MSG("char array has no valid allocator");
    return RCUTILS_RET_ERROR;
  }

  allocator->deallocate(char_array->buffer, allocator->state);
  char_array->buffer = NULL;
  char_array->buffer_length = 0u;
  char_array->buffer_capacity = 0u;

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_unsigned_char_array_resize(rcutils_unsigned_char_array_t * char_array, size_t new_size)
{
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    char_array,
    "char array pointer is null",
    return RCUTILS_RET_ERROR);

  rcutils_allocator_t * allocator = &char_array->allocator;
  if (!rcutils_allocator_is_valid(allocator)) {
    RCUTILS_SET_ERROR_MSG("char array has no valid allocator");
    return RCUTILS_RET_ERROR;
  }

  if (new_size == char_array->buffer_capacity) {
    // nothing to do here
    return RCUTILS_RET_OK;
  }

  char_array->buffer = rcutils_reallocf(
    char_array->buffer, new_size * sizeof(unsigned char), allocator);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    char_array->buffer,
    "failed to reallocate memory for char array",
    return RCUTILS_RET_BAD_ALLOC);

  char_array->buffer_capacity = new_size;
  if (new_size < char_array->buffer_length) {
    char_array->buffer_length = new_size;
  }

  return RCUTILS_RET_OK;
}
