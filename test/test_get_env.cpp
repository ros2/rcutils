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

#include <gtest/gtest.h>

#include <string>

#include "rcutils/get_env.h"

/* Tests the default allocator.
 *
 * Expected environment variables must be set by the calling code:
 *
 *   - EMPTY_TEST=
 *   - NORMAL_TEST=foo
 *
 * These are set in the call to `ament_add_gtest()` in the `CMakeLists.txt`.
 */
TEST(TestGetEnv, test_get_env) {
  const char * env;
  const char * ret;
  ret = rcutils_get_env("NORMAL_TEST", NULL);
  EXPECT_STREQ("argument env_value is null", ret);
  ret = rcutils_get_env(NULL, &env);
  EXPECT_STREQ("argument env_name is null", ret);
  ret = rcutils_get_env("SHOULD_NOT_EXIST_TEST", &env);
  EXPECT_FALSE(ret);
  EXPECT_STREQ("", env);
  ret = rcutils_get_env("NORMAL_TEST", &env);
  EXPECT_FALSE(ret);
  EXPECT_FALSE(NULL == env);
  EXPECT_STREQ("foo", env);
  ret = rcutils_get_env("EMPTY_TEST", &env);
  EXPECT_FALSE(ret);
  EXPECT_FALSE(NULL == env);
  EXPECT_STREQ("", env);
}
