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

#include "gtest/gtest.h"

#include "c_utilities/find.h"

#define ENABLE_LOGGING 1

#if ENABLE_LOGGING
#define LOG(expected, actual) { \
    printf("Expected: %zu Actual: %zu\n", expected, actual); }
#else
#define LOG(X, arg) {}
#endif

size_t test_find(const char * str, char delimiter, size_t expected_pos)
{
  size_t actual_pos = utilities_find(str, delimiter);
  EXPECT_EQ(expected_pos, actual_pos);
  return actual_pos;
}

size_t test_find_last(const char * str, char delimiter, size_t expected_pos)
{
  size_t actual_pos = utilities_find_last(str, delimiter);
  EXPECT_EQ(expected_pos, actual_pos);
  return actual_pos;
}

TEST(test_find, find) {
  size_t ret0 = test_find("", '/', 0);
  LOG((size_t)0, ret0);

  size_t ret00 = test_find(NULL, '/', 0);
  LOG((size_t)0, ret00);

  size_t ret1 = test_find("hello_world", '/', strlen("hello_world"));
  LOG(strlen("hello_world"), ret1);

  size_t ret2 = test_find("hello/world", '/', 5);
  LOG((size_t)5, ret2);

  size_t ret3 = test_find("/hello/world", '/', 0);
  LOG((size_t)0, ret3);

  size_t ret4 = test_find("hello/world/", '/', 5);
  LOG((size_t)5, ret4);

  size_t ret5 = test_find("hello//world", '/', 5);
  LOG((size_t)5, ret5);

  size_t ret6 = test_find("/hello//world", '/', 0);
  LOG((size_t)0, ret6);
}

TEST(test_find, find_last) {
  size_t ret0 = test_find_last("", '/', 0);
  LOG((size_t)0, ret0);

  size_t ret00 = test_find_last(NULL, '/', 0);
  LOG((size_t)0, ret00);

  size_t ret1 = test_find_last("hello_world", '/', strlen("hello_world"));
  LOG(strlen("hello_world"), ret1);

  size_t ret2 = test_find_last("hello/world", '/', 5);
  LOG((size_t)5, ret2);

  size_t ret3 = test_find_last("/hello/world", '/', 6);
  LOG((size_t)6, ret3);

  size_t ret4 = test_find_last("hello/world/", '/', strlen("hello_world/") - 1);
  LOG((size_t)strlen("hello_world/") - 1, ret4);

  size_t ret5 = test_find_last("hello//world", '/', 6);
  LOG((size_t)6, ret5);

  size_t ret6 = test_find_last("/hello//world", '/', 7);
  LOG((size_t)7, ret6);
}
