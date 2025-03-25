// Copyright 2025 Open Source Robotics Foundation, Inc.
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

#include <string.h>

#include "rcutils/error_handling.h"
#include "rcutils/join.h"
#include "rcutils/macros.h"
#include "rcutils/strdup.h"
#include "rcutils/types.h"

char *
rcutils_join(
  const rcutils_string_array_t * string_array,
  const char * separator,
  rcutils_allocator_t allocator)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(NULL);

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_array, NULL);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(separator, NULL);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "allocator is invalid", return NULL);

  if (string_array->size < 1) {
    return rcutils_strdup("", allocator);
  }

  size_t sep_length = strlen(separator);
  size_t string_length = sep_length * (string_array->size - 1);

  for (size_t i = 0; i < string_array->size; i++) {
    if (string_array->data[i]) {
      string_length += strlen(string_array->data[i]);
    }
  }

  char * new_string = allocator.allocate(string_length + 1, allocator.state);
  if (NULL == new_string) {
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for new string");
    return NULL;
  }

  char * pos = new_string;
  for (size_t i = 0; i < string_array->size; i++) {
    if (i != 0) {
      memcpy(pos, separator, sep_length);
      pos += sep_length;
    }
    if (string_array->data[i]) {
      string_length = strlen(string_array->data[i]);
      memcpy(pos, string_array->data[i], string_length);
      pos += string_length;
    }
  }

  *pos = '\0';

  return new_string;
}

#ifdef __cplusplus
}
#endif
