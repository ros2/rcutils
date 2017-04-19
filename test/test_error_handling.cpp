// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#include <string>

#include "gmock/gmock.h"

#include "rcutils/error_handling.h"

int
count_substrings(const std::string & str, const std::string & substr)
{
  if (substr.length() == 0) {
    return 0;
  }
  int count = 0;
  for (
    size_t offset = str.find(substr);
    offset != std::string::npos;
    offset = str.find(substr, offset + substr.length()))
  {
    ++count;
  }
  return count;
}

TEST(test_error_handling, nominal) {
  rcutils_reset_error();
  const char * test_message = "test message";
  RCUTILS_SET_ERROR_MSG(test_message, rcutils_get_default_allocator());
  using ::testing::StartsWith;
  ASSERT_THAT(rcutils_get_error_string_safe(), StartsWith(test_message));
  rcutils_reset_error();
}

TEST(test_error_handling, reset) {
  rcutils_reset_error();
  {
    const char * test_message = "test message";
    RCUTILS_SET_ERROR_MSG(test_message, rcutils_get_default_allocator());
    using ::testing::StartsWith;
    ASSERT_THAT(rcutils_get_error_string_safe(), StartsWith(test_message));
  }
  rcutils_reset_error();
  {
    const char * test_message = "different message";
    RCUTILS_SET_ERROR_MSG(test_message, rcutils_get_default_allocator());
    using ::testing::StartsWith;
    ASSERT_THAT(rcutils_get_error_string_safe(), StartsWith(test_message));
  }
  rcutils_reset_error();
  ASSERT_EQ(nullptr, rcutils_get_error_string());
  ASSERT_NE(nullptr, rcutils_get_error_string_safe());
  ASSERT_STREQ("error not set", rcutils_get_error_string_safe());
  rcutils_reset_error();
}

TEST(test_error_handling, empty) {
  rcutils_reset_error();
  ASSERT_EQ(nullptr, rcutils_get_error_string());
  ASSERT_NE(nullptr, rcutils_get_error_string_safe());
  ASSERT_STREQ("error not set", rcutils_get_error_string_safe());
  rcutils_reset_error();
}

TEST(test_error_handling, recursive) {
  rcutils_reset_error();
  const char * test_message = "test message";
  RCUTILS_SET_ERROR_MSG(test_message, rcutils_get_default_allocator());
  using ::testing::HasSubstr;
  ASSERT_THAT(rcutils_get_error_string_safe(), HasSubstr(", at"));
  RCUTILS_SET_ERROR_MSG(rcutils_get_error_string_safe(), rcutils_get_default_allocator());
  std::string err_msg(rcutils_get_error_string_safe());
  int count = count_substrings(err_msg, ", at");
  EXPECT_EQ(2, count) <<
    "Expected ', at' in the error string twice but got it '" << count << "': " << err_msg;
  rcutils_reset_error();
}
