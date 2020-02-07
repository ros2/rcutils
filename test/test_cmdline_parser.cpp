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

#include "rcutils/cmdline_parser.h"


TEST(CmdLineParser, cli_option_exist) {
  char * arr[3];
  char a[] = "option1";
  char b[] = "option2";
  char c[] = "option3";
  arr[0] = reinterpret_cast<char *>(a);
  arr[1] = reinterpret_cast<char *>(b);
  arr[2] = reinterpret_cast<char *>(c);

  EXPECT_EQ(rcutils_cli_option_exist(arr, arr + 3, "option1"), true);
  EXPECT_EQ(rcutils_cli_option_exist(arr, arr + 3, "option2"), true);
  EXPECT_EQ(rcutils_cli_option_exist(arr, arr + 3, "option3"), true);
  EXPECT_EQ(rcutils_cli_option_exist(arr, arr + 3, "opt"), false);
  EXPECT_EQ(rcutils_cli_option_exist(arr, arr + 3, "NotRelated"), false);
}

TEST(CmdLineParser, cli_get_option) {
  char * arr[4];
  char a[] = "option1";
  char b[] = "sub1";
  char c[] = "option2";
  arr[0] = reinterpret_cast<char *>(a);
  arr[1] = reinterpret_cast<char *>(b);
  arr[2] = reinterpret_cast<char *>(c);

  EXPECT_STREQ(rcutils_cli_get_option(arr, arr + 3, "option1"), "sub1");
  EXPECT_STREQ(rcutils_cli_get_option(arr, arr + 3, "NotRelated"), NULL);
  EXPECT_STREQ(rcutils_cli_get_option(arr, arr + 3, "option2"), NULL);
}
