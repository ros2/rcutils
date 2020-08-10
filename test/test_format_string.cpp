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

#include <gtest/gtest.h>

#include <string>

#include "./allocator_testing_utils.h"
#include "./mocking_utils/patch.hpp"
#include "./mocking_utils/stdio.hpp"

#include "rcutils/allocator.h"
#include "rcutils/format_string.h"

TEST(test_format_string_limit, nominal) {
  {
    auto allocator = rcutils_get_default_allocator();
    char * formatted = rcutils_format_string_limit(allocator, 10, "%s", "test");
    EXPECT_STREQ("test", formatted);
    if (formatted) {
      allocator.deallocate(formatted, allocator.state);
    }
  }

  {
    auto allocator = rcutils_get_default_allocator();
    char * formatted = rcutils_format_string_limit(allocator, 3, "%s", "test");
    EXPECT_STREQ("te", formatted);
    if (formatted) {
      allocator.deallocate(formatted, allocator.state);
    }
  }

  {
    auto allocator = rcutils_get_default_allocator();
    char * formatted = rcutils_format_string_limit(allocator, 3, "string is too long %s", "test");
    EXPECT_STREQ("st", formatted);
    if (formatted) {
      allocator.deallocate(formatted, allocator.state);
    }
  }
}

TEST(test_format_string_limit, invalid_arguments) {
  auto allocator = rcutils_get_default_allocator();
  auto failing_allocator = get_failing_allocator();

  char * formatted = rcutils_format_string_limit(allocator, 10, NULL);
  EXPECT_STREQ(NULL, formatted);

  formatted = rcutils_format_string_limit(failing_allocator, 10, "%s", "test");
  EXPECT_STREQ(NULL, formatted);
}

#ifdef MOCKING_UTILS_CAN_PATCH_VSNPRINTF
TEST(test_format_string_limit, on_internal_error) {
#ifdef _WIN32
  auto _vscprintf_mock = mocking_utils::patch__vscprintf(
    "lib:rcutils", [](auto && ...) {return 1;});

  auto _vsnprintf_s_mock = mocking_utils::patch__vsnprintf_s(
    "lib:rcutils", [](auto && ...) {
      errno = EINVAL;
      return -1;
    });
#else
  auto mock = mocking_utils::patch_vsnprintf(
    "lib:rcutils", [](char * buffer, auto && ...) {
      if (nullptr == buffer) {
        return 1;  // provide a dummy value if buffer required size is queried
      }
      errno = EINVAL;
      return -1;
    });
#endif

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  char * formatted = rcutils_format_string_limit(allocator, 10, "%s", "test");
  EXPECT_STREQ(NULL, formatted);
}
#endif  // MOCKING_UTILS_CAN_PATCH_VSNPRINTF
