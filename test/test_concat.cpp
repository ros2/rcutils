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

#include "c_utilities/concat.h"

#define ENABLE_LOGGING 0

#if ENABLE_LOGGING
#define LOG(expected, actual) { \
    printf("Expected: %s Actual: %s\n", expected, actual); }
#else
#define LOG(X, arg) {}
#endif

TEST(test_concat, concat) {
  const char * lhs = NULL;
  const char * rhs = NULL;
  const char * delimiter = NULL;

  const char * res = utilities_concat(lhs, rhs, delimiter);
  EXPECT_STREQ(NULL, res);

  lhs = static_cast<const char *>("foo");
  res = utilities_concat(lhs, rhs, delimiter);
  EXPECT_STREQ(NULL, res);

  rhs = static_cast<const char *>("bar");
  res = utilities_concat(lhs, rhs, delimiter);
  LOG("foobar", res);
  EXPECT_STREQ("foobar", res);

  delimiter = static_cast<const char *>("/");
  res = utilities_concat(lhs, rhs, delimiter);
  LOG("foo/bar", res);
  EXPECT_STREQ("foo/bar", res);
}
