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

#if __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/error_handling.h"
#include "rcutils/format_string.h"
#include "rcutils/split.h"
#include "rcutils/types.h"

rcutils_ret_t
rcutils_split(
  const char * str,
  char delimiter,
  rcutils_allocator_t allocator,
  rcutils_string_array_t * string_array)
{
  if (!str || strlen(str) == 0) {
    *string_array = rcutils_get_zero_initialized_string_array();
    return RCUTILS_RET_OK;
  }

  size_t string_size = strlen(str);

  // does it start with a delmiter?
  size_t lhs_offset = 0;
  if (str[0] == delimiter) {
    lhs_offset = 1;
  }

  // does it end with a delimiter?
  size_t rhs_offset = 0;
  if (str[string_size - 1] == delimiter) {
    rhs_offset = 1;
  }

  const char * error_msg;
  string_array->size = 1;
  for (size_t i = lhs_offset; i < string_size - rhs_offset; ++i) {
    if (str[i] == delimiter) {
      ++string_array->size;
    }
  }
  string_array->data = allocator.allocate(string_array->size * sizeof(char *), allocator.state);
  if (!string_array->data) {
    goto fail;
  }

  size_t token_counter = 0;
  size_t lhs = 0 + lhs_offset;
  size_t rhs = 0 + lhs_offset;
  for (; rhs < string_size - rhs_offset; ++rhs) {
    if (str[rhs] == delimiter) {
      // in case we have two consequetive delimiters
      // we ignore these and delimish the size of the array
      if (rhs - lhs < 1) {
        --string_array->size;
        string_array->data[string_array->size] = NULL;
      } else {
        // +2 (1+1) because lhs is index, not actual position
        // and nullterminating
        string_array->data[token_counter] =
          allocator.allocate((rhs - lhs + 2) * sizeof(char), allocator.state);
        if (!string_array->data[token_counter]) {
          goto fail;
        }
        snprintf(string_array->data[token_counter], (rhs - lhs + 1), "%s", str + lhs);
        ++token_counter;
      }
      lhs = rhs;
      ++lhs;
    }
  }

  // take care of trailing token
  if (rhs - lhs < 1) {
    --string_array->size;
    allocator.deallocate(string_array->data[string_array->size], allocator.state);
    string_array->data[string_array->size] = NULL;
  } else {
    string_array->data[token_counter] =
      allocator.allocate((rhs - lhs + 2) * sizeof(char), allocator.state);
    snprintf(string_array->data[token_counter], (rhs - lhs + 1), "%s", str + lhs);
  }

  return RCUTILS_RET_OK;

fail:
  error_msg = "unable to allocate memory for string array data";
  if (rcutils_string_array_fini(string_array, &allocator) != RCUTILS_RET_OK) {
    error_msg = rcutils_format_string(allocator, "FATAL: %s. Leaking memory", error_msg);
  }
  string_array = NULL;
  RCUTILS_SET_ERROR_MSG(error_msg, allocator);
  return RCUTILS_RET_ERROR;
}

rcutils_ret_t
rcutils_split_last(
  const char * str,
  char delimiter,
  rcutils_allocator_t allocator,
  rcutils_string_array_t * string_array)
{
  if (!str || strlen(str) == 0) {
    *string_array = rcutils_get_zero_initialized_string_array();
    return RCUTILS_RET_OK;
  }

  size_t string_size = strlen(str);

  // does it start with a delmiter?
  size_t lhs_offset = 0;
  if (str[0] == delimiter) {
    lhs_offset = 1;
  }

  // does it end with a delimiter?
  size_t rhs_offset = 0;
  if (str[string_size - 1] == delimiter) {
    rhs_offset = 1;
  }

  const char * error_msg;

  size_t found_last = string_size;
  for (size_t i = lhs_offset; i < string_size - rhs_offset; ++i) {
    if (str[i] == delimiter) {
      found_last = i;
    }
  }

  if (found_last == string_size) {
    string_array->size = 1;
    string_array->data = allocator.allocate(1 * sizeof(char *), allocator.state);
    if (!string_array->data) {
      goto fail;
    }
    string_array->data[0] =
      allocator.allocate((found_last - lhs_offset + 2) * sizeof(char), allocator.state);
    if (!string_array->data) {
      goto fail;
    }
    snprintf(string_array->data[0], found_last - lhs_offset + 1, "%s", str + lhs_offset);
  } else {
    string_array->size = 2;
    string_array->data = allocator.allocate(2 * sizeof(char *), allocator.state);
    if (!string_array->data) {
      goto fail;
    }

    /*
     * The extra +1 after 'found_last' is to compensate its position
     * found_last is the index of the the last position the delimiter was found
     * and not the actual position in terms of counting from 1
     */
    size_t inner_rhs_offset = (str[found_last - 1] == delimiter) ? 1 : 0;
    string_array->data[0] = allocator.allocate(
      (found_last + 1 - lhs_offset - inner_rhs_offset + 1) * sizeof(char),
      allocator.state);
    if (!string_array->data[0]) {
      goto fail;
    }
    snprintf(string_array->data[0], found_last + 1 - lhs_offset - inner_rhs_offset,
      "%s", str + lhs_offset);

    string_array->data[1] = allocator.allocate(
      (string_size - found_last - rhs_offset + 1) * sizeof(char),
      allocator.state);
    if (!string_array->data[1]) {
      goto fail;
    }
    snprintf(string_array->data[1], string_size - found_last - rhs_offset, "%s",
      str + found_last + 1);
  }

  return RCUTILS_RET_OK;

fail:
  error_msg = "unable to allocate memory for string array data";
  if (rcutils_string_array_fini(string_array, &allocator) != RCUTILS_RET_OK) {
    error_msg = rcutils_format_string(allocator, "FATAL: %s. Leaking memory", error_msg);
  }
  string_array = NULL;
  RCUTILS_SET_ERROR_MSG(error_msg, allocator);
  return RCUTILS_RET_ERROR;
}

#if __cplusplus
}
#endif
