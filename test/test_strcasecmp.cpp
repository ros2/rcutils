// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rcutils/strcasecmp.h"

// Tests the rcutils_strcasecmp() function.
TEST(TestStrcasecmp, test_strcasecmp) {
  EXPECT_EQ(0, rcutils_strcasecmp("", ""));
  EXPECT_EQ(0, rcutils_strcasecmp("abc", "abc"));
  EXPECT_EQ(0, rcutils_strcasecmp("ABC", "ABC"));
  EXPECT_EQ(0, rcutils_strcasecmp("1abc", "1abc"));
  EXPECT_EQ(0, rcutils_strcasecmp("abc1", "abc1"));
  EXPECT_EQ(0, rcutils_strcasecmp("1ABC", "1ABC"));
  EXPECT_EQ(0, rcutils_strcasecmp("ABC1", "ABC1"));
  EXPECT_EQ(0, rcutils_strcasecmp("ABC", "abc"));
  EXPECT_EQ(0, rcutils_strcasecmp("abc", "ABC"));
  EXPECT_EQ(0, rcutils_strcasecmp("Abc", "abc"));
  EXPECT_EQ(0, rcutils_strcasecmp("abc", "Abc"));
  EXPECT_EQ(0, rcutils_strcasecmp("Abc", "Abc"));
  EXPECT_EQ(0, rcutils_strcasecmp("aBc", "abc"));
  EXPECT_EQ(0, rcutils_strcasecmp("abc", "aBc"));
  EXPECT_EQ(0, rcutils_strcasecmp("aBc", "aBc"));
  EXPECT_EQ(0, rcutils_strcasecmp("abC", "abc"));
  EXPECT_EQ(0, rcutils_strcasecmp("abc", "abC"));
  EXPECT_EQ(0, rcutils_strcasecmp("abC", "abC"));

  EXPECT_NE(0, rcutils_strcasecmp("", "abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc", ""));
  EXPECT_NE(0, rcutils_strcasecmp("abcd", "abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc", "abcd"));
  EXPECT_NE(0, rcutils_strcasecmp("abcD", "abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc", "abcD"));
  EXPECT_NE(0, rcutils_strcasecmp("1abc", "abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc", "1abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc1", "abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc", "abc1"));
  EXPECT_NE(0, rcutils_strcasecmp("ABCd", "abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc", "ABCd"));
  EXPECT_NE(0, rcutils_strcasecmp("1Abc", "abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc", "1Abc"));
  EXPECT_NE(0, rcutils_strcasecmp("a1Bc", "abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc", "a1Bc"));
  EXPECT_NE(0, rcutils_strcasecmp("ab1C", "abc"));
  EXPECT_NE(0, rcutils_strcasecmp("abc", "ab1C"));
}
