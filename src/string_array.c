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

#include "c_utilities/types/string_array.h"
#include "c_utilities/types/utilities_ret.h"

string_array_t
utilities_get_zero_initialized_string_array()
{
  static string_array_t array = {0, NULL};
  return array;
}

string_array_t
utilities_get_pre_initialized_string_array(size_t size)
{
  static string_array_t array = {0, NULL};
  array.size = size;
  array.data = (char **)malloc(array.size * sizeof(char *));
  for (size_t i = 0; i < size; ++i) {
    array.data[i] = NULL;
  }
  return array;
}

utilities_ret_t
utilities_string_array_fini(string_array_t * array)
{
  if (!array) {
    return UTILITIES_RET_ERROR;
  }

  utilities_ret_t ret = UTILITIES_RET_OK;
  for (size_t i = 0; i < array->size; ++i) {
    free(array->data[i]);
    array->data[i] = NULL;
  }

  if (array->data) {
    free(array->data);
    array->data = NULL;
  }

  return ret;
}

#if __cplusplus
}
#endif
