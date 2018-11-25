// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#include <gtest/gtest.h>

#include "rcutils/allocator.h"

#include "rcutils/types/char_array.h"

TEST(test_char_array, default_initialization) {
  auto char_array = rcutils_get_zero_initialized_char_array();

  auto allocator = rcutils_get_default_allocator();
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_char_array_init(&char_array, 0, &allocator));
  EXPECT_EQ(0lu, char_array.buffer_capacity);
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_char_array_fini(&char_array));
  EXPECT_EQ(0lu, char_array.buffer_capacity);
  EXPECT_FALSE(char_array.buffer);
}

TEST(test_char_array, resize) {
  auto char_array = rcutils_get_zero_initialized_char_array();
  auto allocator = rcutils_get_default_allocator();
  auto ret = rcutils_char_array_init(&char_array, 5, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  memcpy(char_array.buffer, "1234\0", 5);
  char_array.buffer_length = 5;
  EXPECT_STREQ("1234\0", char_array.buffer);

  ret = rcutils_char_array_resize(&char_array, 0);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);
  EXPECT_EQ(5lu, char_array.buffer_capacity);
  EXPECT_EQ(5lu, char_array.buffer_length);

  ret = rcutils_char_array_resize(&char_array, 11);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(11lu, char_array.buffer_capacity);
  EXPECT_EQ(5lu, char_array.buffer_length);

  memcpy(char_array.buffer, "0987654321\0", 11);
  char_array.buffer_length = 11;
  EXPECT_STREQ("0987654321\0", char_array.buffer);

  ret = rcutils_char_array_resize(&char_array, 3);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(3lu, char_array.buffer_capacity);
  EXPECT_EQ(3lu, char_array.buffer_length);
  EXPECT_EQ('0', char_array.buffer[0]);
  EXPECT_EQ('9', char_array.buffer[1]);
  EXPECT_EQ('8', char_array.buffer[2]);
  // the other fields are garbage.

  // cleanup only 3 fields
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_char_array_fini(&char_array));
}
