// Copyright 2022 Open Source Robotics Foundation, Inc.
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

#include "rcutils/zero_memory.h"

// Tests the RCUTILS_ZERO_MEMORY() macro.
TEST(TestZeroMemory, test_zero_memory) {
  char test_str[] = "0123456789";

  RCUTILS_ZERO_MEMORY(test_str, sizeof(test_str));
  for (size_t i = 0; i < sizeof(test_str); ++i) {
    EXPECT_EQ(test_str[i], 0x0);
  }
}
