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

#include "rcutils/types/unsigned_char_array.h"

TEST(test_unsigned_char_array, default_initialization) {
  auto char_array = rcutils_get_zero_initialized_unsigned_char_array();

  auto allocator = rcutils_get_default_allocator();
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_unsigned_char_array_init(&char_array, 0, &allocator));
  EXPECT_EQ(0u, char_array.buffer_capacity);
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_unsigned_char_array_fini(&char_array));
  EXPECT_EQ(0u, char_array.buffer_capacity);
  EXPECT_FALSE(char_array.buffer);
}

TEST(test_unsigned_char_array, resize) {
  auto char_array = rcutils_get_zero_initialized_unsigned_char_array();
  auto allocator = rcutils_get_default_allocator();
  auto ret = rcutils_unsigned_char_array_init(&char_array, 5, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  for (size_t i = 0; i < 5; ++i) {
    unsigned char c = 0xFF;
    memcpy(char_array.buffer + i, &c, 1);
  }
  char_array.buffer_length = 5;
  for (size_t i = 0; i < char_array.buffer_length; ++i) {
    EXPECT_EQ(0xFF, char_array.buffer[i]);
  }

  ret = rcutils_unsigned_char_array_resize(&char_array, 10);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(10u, char_array.buffer_capacity);
  EXPECT_EQ(5u, char_array.buffer_length);

  for (size_t i = 0; i < 10; ++i) {
    unsigned char c = 0xFF - i;
    memcpy(char_array.buffer + i, &c, 1);
  }
  char_array.buffer_length = 10;
  for (size_t i = 0; i < char_array.buffer_length; ++i) {
    unsigned char c = 0xFF - i;
    EXPECT_EQ(c, char_array.buffer[i]);
  }

  ret = rcutils_unsigned_char_array_resize(&char_array, 3);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(3u, char_array.buffer_capacity);
  EXPECT_EQ(3u, char_array.buffer_length);
  EXPECT_EQ(0xFF, char_array.buffer[0]);
  EXPECT_EQ(0xFF - 1, char_array.buffer[1]);
  EXPECT_EQ(0xFF - 2, char_array.buffer[2]);
  // the other fields are garbage.

  // cleanup only 3 fields
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_unsigned_char_array_fini(&char_array));
}
