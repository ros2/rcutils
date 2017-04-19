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

#include <stdlib.h>

#include "rcutils/types/string_array.h"
#include "rcutils/types/rcutils_ret.h"

rcutils_string_array_t
rcutils_get_zero_initialized_string_array()
{
  static rcutils_string_array_t array = {0, NULL};
  return array;
}

rcutils_string_array_t
rcutils_get_pre_initialized_string_array(size_t size)
{
  static rcutils_string_array_t array = {0, NULL};
  array.size = size;
  array.data = (char **)calloc(array.size, sizeof(char *));
  return array;
}

rcutils_ret_t
rcutils_string_array_fini(rcutils_string_array_t * array)
{
  if (!array) {
    return RCUTILS_RET_ERROR;
  }

  if (!array->data) {
    return RCUTILS_RET_OK;
  }

  for (size_t i = 0; i < array->size; ++i) {
    free(array->data[i]);
    array->data[i] = NULL;
  }
  free(array->data);
  array->data = NULL;

  return RCUTILS_RET_OK;
}

#if __cplusplus
}
#endif
