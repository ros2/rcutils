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

#include "c_utilities/types.h"

string_array_t
utilities_split(const char * str, char delimiter)
{
  if (!str || strlen(str) == 0) {
    string_array_t empty_array = {0, NULL};
    return empty_array;
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

  string_array_t tokens = utilities_get_zero_initialized_string_array();
  tokens.size = 1;
  for (size_t i = lhs_offset; i < string_size - rhs_offset; ++i) {
    if (str[i] == delimiter) {
      ++tokens.size;
    }
  }
  tokens.data = malloc(tokens.size * sizeof(char *));

  size_t token_counter = 0;
  size_t lhs = 0 + lhs_offset;
  size_t rhs = 0 + lhs_offset;
  for (; rhs < string_size - rhs_offset; ++rhs) {
    if (str[rhs] == delimiter) {
      // in case we have two consequetive delimiters
      // we ignore these and delimish the size of the array
      if (rhs - lhs < 1) {
        --tokens.size;
        tokens.data[tokens.size] = NULL;
      } else {
        tokens.data[token_counter] = malloc((rhs - lhs + 2) * sizeof(char));
        snprintf(tokens.data[token_counter], (rhs - lhs + 1), "%s", str + lhs);
        ++token_counter;
      }
      lhs = rhs;
      ++lhs;
    }
  }

  // take care of trailing token
  if (rhs - lhs < 1) {
    --tokens.size;
    free(tokens.data[tokens.size]);
    tokens.data[tokens.size] = NULL;
  } else {
    tokens.data[token_counter] = malloc((rhs - lhs + 2) * sizeof(char));
    snprintf(tokens.data[token_counter], (rhs - lhs + 1), "%s", str + lhs);
  }
  return tokens;
}

string_array_t
utilities_split_last(const char * str, char delimiter)
{
  if (!str || strlen(str) == 0) {
    string_array_t empty_array = {0, NULL};
    return empty_array;
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

  string_array_t tokens = utilities_get_zero_initialized_string_array();

  size_t found_last = string_size;
  for (size_t i = lhs_offset; i < string_size - rhs_offset; ++i) {
    if (str[i] == delimiter) {
      found_last = i;
    }
  }

  if (found_last == string_size) {
    tokens.size = 1;
    tokens.data = malloc(1 * sizeof(char *));
    tokens.data[0] = malloc((found_last - lhs_offset + 2) * sizeof(char));
    snprintf(tokens.data[0], found_last - lhs_offset + 1, "%s", str + lhs_offset);
  } else {
    tokens.size = 2;
    tokens.data = malloc(2 * sizeof(char *));

    size_t inner_rhs_offset = (str[found_last - 1] == delimiter) ? 1 : 0;
    tokens.data[0] = malloc((found_last - lhs_offset - inner_rhs_offset + 2) * sizeof(char));
    snprintf(tokens.data[0], found_last - lhs_offset - inner_rhs_offset + 1,
      "%s", str + lhs_offset);

    tokens.data[1] = malloc((string_size - found_last + 2) * sizeof(char));
    snprintf(tokens.data[1], found_last + 1, "%s", str + found_last + 1);
  }
  return tokens;
}

#if __cplusplus
}
#endif
