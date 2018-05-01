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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/concat.h"

char *
rcutils_concat(const char * lhs, const char * rhs, const char * delimiter)
{
  if (NULL == lhs) {
    return NULL;
  }
  if (NULL == rhs) {
    return NULL;
  }

  size_t lhs_len = strlen(lhs);
  size_t rhs_len = strlen(rhs);
  size_t del_len = (delimiter) ? strlen(delimiter) : 0;

  char * concat = (char *) malloc((lhs_len + rhs_len + del_len + 1) * sizeof(char));
  if (NULL == concat) {
    return NULL;
  }

  int n = snprintf(concat, lhs_len + 1, "%s", lhs);
  snprintf(concat + n, rhs_len + del_len + 1, "%s%s", (delimiter) ? delimiter : "", rhs);

  return concat;
}

#ifdef __cplusplus
}
#endif
