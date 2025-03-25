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

#include "gtest/gtest.h"

#include "./allocator_testing_utils.h"
#include "./time_bomb_allocator_testing_utils.h"
#include "rcutils/error_handling.h"
#include "rcutils/join.h"
#include "rcutils/types/string_array.h"

#define LOG(expected, actual) printf("Expected: %s Actual: %s\n", expected, actual);

TEST(test_join, join) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_allocator_t bad_allocator = rcutils_get_zero_initialized_allocator();
  rcutils_allocator_t time_bomb_allocator = get_time_bomb_allocator();
  rcutils_string_array_t tokens0 = rcutils_get_zero_initialized_string_array();
  rcutils_string_array_t tokens1 = rcutils_get_zero_initialized_string_array();
  rcutils_string_array_t tokens2 = rcutils_get_zero_initialized_string_array();
  char * new_string;

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_string_array_init(&tokens0, 0, &allocator));
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_string_array_init(&tokens1, 1, &allocator));
  tokens1.data[0] = strdup("hallo");
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_string_array_init(&tokens2, 2, &allocator));
  tokens2.data[0] = strdup("hello");
  tokens2.data[1] = strdup("world");

  EXPECT_STREQ(
    nullptr,
    rcutils_join(NULL, "-", allocator));
  rcutils_reset_error();

  EXPECT_STREQ(
    nullptr,
    rcutils_join(&tokens0, NULL, allocator));
  rcutils_reset_error();

  EXPECT_STREQ(
    nullptr,
    rcutils_join(&tokens0, " ", bad_allocator));
  rcutils_reset_error();

  // Allocating new_string fails
  set_time_bomb_allocator_malloc_count(time_bomb_allocator, 0);
  EXPECT_STREQ(
    nullptr,
    rcutils_join(&tokens2, " ", time_bomb_allocator));
  rcutils_reset_error();

  new_string = rcutils_join(&tokens0, " ", allocator);
  EXPECT_STREQ("", new_string);
  allocator.deallocate(new_string, &allocator.state);

  new_string = rcutils_join(&tokens1, " ", allocator);
  EXPECT_STREQ("hallo", new_string);
  allocator.deallocate(new_string, &allocator.state);

  new_string = rcutils_join(&tokens2, "", allocator);
  EXPECT_STREQ("helloworld", new_string);
  allocator.deallocate(new_string, &allocator.state);

  new_string = rcutils_join(&tokens2, " ", allocator);
  EXPECT_STREQ("hello world", new_string);
  allocator.deallocate(new_string, &allocator.state);

  new_string = rcutils_join(&tokens2, " ... ", allocator);
  EXPECT_STREQ("hello ... world", new_string);
  allocator.deallocate(new_string, &allocator.state);

  EXPECT_EQ(
    RCUTILS_RET_OK,
    rcutils_string_array_fini(&tokens0));
}
