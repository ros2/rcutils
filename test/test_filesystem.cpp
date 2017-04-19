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

#include "rcutils/filesystem.h"

static char cwd[1024];

TEST(test_filesystem, join_path) {
  const char * path = rcutils_join_path("foo", "bar");
#ifdef WIN32
  const char * ref_str = "foo\\bar";
#else
  const char * ref_str = "foo/bar";
#endif
  EXPECT_FALSE(NULL == path);
  EXPECT_STREQ(ref_str, path);
}

TEST(test_filesystem, exists) {
  EXPECT_FALSE(rcutils_get_cwd(NULL, 1024));
  EXPECT_TRUE(rcutils_get_cwd(cwd, 1024));
  const char * path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_readable_file.txt");
  EXPECT_TRUE(rcutils_exists(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_folder");
  EXPECT_TRUE(rcutils_exists(path));
}

TEST(test_filesystem, is_directory) {
  EXPECT_TRUE(rcutils_get_cwd(cwd, 1024));
  const char * path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_readable_file.txt");
  EXPECT_FALSE(rcutils_is_directory(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_folder");
  EXPECT_TRUE(rcutils_is_directory(path));
}

TEST(test_filesystem, is_file) {
  EXPECT_TRUE(rcutils_get_cwd(cwd, 1024));
  const char * path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_readable_file.txt");
  EXPECT_TRUE(rcutils_is_file(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_folder");
  EXPECT_FALSE(rcutils_is_file(path));
}

TEST(test_filesystem, is_readable) {
  EXPECT_TRUE(rcutils_get_cwd(cwd, 1024));
  const char * path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_readable_file.txt");
  EXPECT_TRUE(rcutils_is_readable(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_folder");
  EXPECT_TRUE(rcutils_is_readable(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_readable_writable_file.txt");
  EXPECT_TRUE(rcutils_is_readable(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_nonexisting_file.txt");
  EXPECT_FALSE(rcutils_is_readable(path));
}

TEST(test_filesystem, is_writable) {
  EXPECT_TRUE(rcutils_get_cwd(cwd, 1024));
  // path = std::string(cwd) + delimiter + std::string("test") + delimiter + std::string(
  //   "dummy_readable_file.txt");
  // EXPECT_FALSE(rcutils_is_writable(path.c_str()));
  const char * path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_folder");
  EXPECT_TRUE(rcutils_is_writable(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_readable_writable_file.txt");
  EXPECT_TRUE(rcutils_is_writable(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_nonexisting_file.txt");
  EXPECT_FALSE(rcutils_is_writable(path));
}

TEST(test_filesystem, is_readable_and_writable) {
  EXPECT_TRUE(rcutils_get_cwd(cwd, 1024));
  // path = std::string(cwd) + std::string("/test/dummy_readable_file.txt");
  // EXPECT_FALSE(rcutils_is_readable_and_writable(path.c_str()));
  const char * path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_folder");
  EXPECT_TRUE(rcutils_is_readable_and_writable(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_readable_writable_file.txt");
  EXPECT_TRUE(rcutils_is_readable_and_writable(path));
  path = rcutils_join_path(cwd, "test");
  path = rcutils_join_path(path, "dummy_nonexisting_file.txt");
  EXPECT_FALSE(rcutils_is_readable_and_writable(path));
}
