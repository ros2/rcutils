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
#include <rcutils/version.h>
#include <cstring>

TEST(test_version_header, test_version_macros) {
#ifdef RCUTILS_VERSION_TIMESTAMP
  EXPECT_NE(std::strlen(RCUTILS_VERSION_TIMESTAMP), 0);
#else  // RCUTILS_VERSION_TIMESTAMP
  EXPECT_TRUE(false);
#endif  // RCUTILS_VERSION_TIMESTAMP

#ifdef RCUTILS_VERSION_MAJOR
  EXPECT_GE(RCUTILS_VERSION_MAJOR, 0);
#else  // RCUTILS_VERSION_MAJOR
  EXPECT_TRUE(false);
#endif  // RCUTILS_VERSION_MAJOR

#ifdef RCUTILS_VERSION_MINOR
  EXPECT_GE(RCUTILS_VERSION_MINOR, 0);
#else  // RCUTILS_VERSION_MINOR
  EXPECT_TRUE(false);
#endif  // RCUTILS_VERSION_MINOR

#ifdef RCUTILS_VERSION_PATCH
  EXPECT_GE(RCUTILS_VERSION_PATCH, 0);
#else  // RCUTILS_VERSION_PATCH
  EXPECT_TRUE(false);
#endif  // RCUTILS_VERSION_PATCH

#ifdef RCUTILS_VERSION_STR
  EXPECT_NE(std::strlen(RCUTILS_VERSION_STR), 0);
#else  // RCUTILS_VERSION_STR
  EXPECT_TRUE(false);
#endif  // RCUTILS_VERSION_STR
}
