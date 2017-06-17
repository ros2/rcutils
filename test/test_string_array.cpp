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

#include "rcutils/types/string_array.h"

#ifdef _WIN32
  #define strdup _strdup
#endif

TEST(test_string_array, boot_string_array) {
  auto allocator = rcutils_get_default_allocator();
  rcutils_ret_t ret = RCUTILS_RET_OK;

  // UNDEFIEND BEHAVIOR
  // rcutils_string_array_t sa00;
  // rcutils_string_array_fini(&sa00);

  rcutils_string_array_t sa0 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_fini(&sa0);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  rcutils_string_array_t sa1 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_init(&sa1, 3, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ret = rcutils_string_array_fini(&sa1);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  rcutils_string_array_t sa2 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_init(&sa2, 2, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  sa2.data[0] = strdup("Hello");
  sa2.data[1] = strdup("World");
  ret = rcutils_string_array_fini(&sa2);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
}
