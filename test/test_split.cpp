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

#include "c_utilities/split.h"
#include "c_utilities/types/string_array.h"

#define ENABLE_LOGGING 1

#if ENABLE_LOGGING
#define LOG(expected, actual) { \
    printf("Expected: %s Actual: %s\n", expected, actual); }
#else
#define LOG(X, arg) {}
#endif

string_array_t test_split(const char * str, char delimiter, size_t expected_token_size)
{
  string_array_t tokens = utilities_split(str, delimiter);
  EXPECT_EQ(expected_token_size, tokens.size);
  for (size_t i = 0; i < tokens.size; ++i) {
    EXPECT_NE((size_t)0, strlen(tokens.data[i]));
  }
  return tokens;
}

string_array_t test_split_last(const char * str, char delimiter, size_t expected_token_size)
{
  string_array_t tokens = utilities_split_last(str, delimiter);
  EXPECT_EQ(expected_token_size, tokens.size);
  for (size_t i = 0; i < tokens.size; ++i) {
    EXPECT_NE((size_t)0, strlen(tokens.data[i]));
  }
  return tokens;
}

TEST(test_split, split) {
  string_array_t ret0 = test_split("", '/', 0);
  utilities_string_array_fini(&ret0);

  string_array_t ret00 = test_split(NULL, '/', 0);
  utilities_string_array_fini(&ret00);

  string_array_t ret1 = test_split("hello_world", '/', 1);
  EXPECT_STREQ("hello_world", ret1.data[0]);
  utilities_string_array_fini(&ret1);

  string_array_t ret2 = test_split("hello/world", '/', 2);
  EXPECT_STREQ("hello", ret2.data[0]);
  EXPECT_STREQ("world", ret2.data[1]);
  utilities_string_array_fini(&ret2);

  string_array_t ret3 = test_split("/hello/world", '/', 2);
  EXPECT_STREQ("hello", ret3.data[0]);
  EXPECT_STREQ("world", ret3.data[1]);
  utilities_string_array_fini(&ret3);

  string_array_t ret4 = test_split("hello/world/", '/', 2);
  EXPECT_STREQ("hello", ret4.data[0]);
  EXPECT_STREQ("world", ret4.data[1]);
  utilities_string_array_fini(&ret4);

  string_array_t ret5 = test_split("hello//world", '/', 2);
  EXPECT_STREQ("hello", ret5.data[0]);
  EXPECT_STREQ("world", ret5.data[1]);
  utilities_string_array_fini(&ret5);

  string_array_t ret6 = test_split("/hello//world", '/', 2);
  EXPECT_STREQ("hello", ret6.data[0]);
  EXPECT_STREQ("world", ret6.data[1]);
  utilities_string_array_fini(&ret6);

  string_array_t ret7 = test_split("my/hello/world", '/', 3);
  EXPECT_STREQ("my", ret7.data[0]);
  EXPECT_STREQ("hello", ret7.data[1]);
  EXPECT_STREQ("world", ret7.data[2]);

  string_array_t ret8 = test_split("/my/hello/world", '/', 3);
  EXPECT_STREQ("my", ret8.data[0]);
  EXPECT_STREQ("hello", ret8.data[1]);
  EXPECT_STREQ("world", ret8.data[2]);

  string_array_t ret9 = test_split("/my/hello/world/", '/', 3);
  EXPECT_STREQ("my", ret9.data[0]);
  EXPECT_STREQ("hello", ret9.data[1]);
  EXPECT_STREQ("world", ret9.data[2]);

  string_array_t ret10 = test_split("/my//hello//world/", '/', 3);
  EXPECT_STREQ("my", ret10.data[0]);
  EXPECT_STREQ("hello", ret10.data[1]);
  EXPECT_STREQ("world", ret10.data[2]);
}

TEST(test_split, split_last) {
  string_array_t ret0 = test_split_last("", '/', 0);
  utilities_string_array_fini(&ret0);

  string_array_t ret00 = test_split_last(NULL, '/', 0);
  utilities_string_array_fini(&ret00);

  string_array_t ret1 = test_split_last("hello_world", '/', 1);
  LOG("hello_world", ret1.data[0]);
  EXPECT_STREQ("hello_world", ret1.data[0]);
  utilities_string_array_fini(&ret1);

  string_array_t ret2 = test_split_last("hello/world", '/', 2);
  EXPECT_STREQ("hello", ret2.data[0]);
  LOG("hello", ret2.data[0]);
  EXPECT_STREQ("world", ret2.data[1]);
  LOG("world", ret2.data[1]);
  utilities_string_array_fini(&ret2);

  string_array_t ret3 = test_split_last("/hello/world", '/', 2);
  EXPECT_STREQ("hello", ret3.data[0]);
  LOG("hello", ret3.data[0]);
  EXPECT_STREQ("world", ret3.data[1]);
  utilities_string_array_fini(&ret3);

  string_array_t ret4 = test_split_last("hello/world/", '/', 2);
  EXPECT_STREQ("hello", ret4.data[0]);
  EXPECT_STREQ("world", ret4.data[1]);
  utilities_string_array_fini(&ret4);

  string_array_t ret5 = test_split_last("hello//world/", '/', 2);
  EXPECT_STREQ("hello", ret5.data[0]);
  LOG("hello", ret5.data[0]);
  EXPECT_STREQ("world", ret5.data[1]);
  LOG("world", ret5.data[1]);
  utilities_string_array_fini(&ret5);

  string_array_t ret6 = test_split_last("/hello//world", '/', 2);
  EXPECT_STREQ("hello", ret6.data[0]);
  EXPECT_STREQ("world", ret6.data[1]);
  utilities_string_array_fini(&ret6);

  string_array_t ret7 = test_split_last("my/hello//world", '/', 2);
  EXPECT_STREQ("my/hello", ret7.data[0]);
  EXPECT_STREQ("world", ret7.data[1]);
  utilities_string_array_fini(&ret7);

  string_array_t ret8 = test_split_last("/my/hello//world/", '/', 2);
  EXPECT_STREQ("my/hello", ret8.data[0]);
  EXPECT_STREQ("world", ret8.data[1]);
  utilities_string_array_fini(&ret8);
}
