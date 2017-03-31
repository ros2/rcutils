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

#include "c_utilities/filesystem.h"

static char cwd[1024];

TEST(test_filesystem, utilities_exists) {
  utilities_get_cwd(cwd, 1024);
  std::string path = std::string(cwd) + std::string("/test/dummy_readable_file.txt");
  EXPECT_TRUE(utilities_exists(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_folder");
  EXPECT_TRUE(utilities_exists(path.c_str()));
}

TEST(test_filesystem, is_directory) {
  utilities_get_cwd(cwd, 1024);
  std::string path = std::string(cwd) + std::string("/test/dummy_readable_file.txt");
  EXPECT_FALSE(utilities_is_directory(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_folder");
  EXPECT_TRUE(utilities_is_directory(path.c_str()));
}

TEST(test_filesystem, is_file) {
  utilities_get_cwd(cwd, 1024);
  std::string path = std::string(cwd) + std::string("/test/dummy_readable_file.txt");
  EXPECT_TRUE(utilities_is_file(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_folder");
  EXPECT_FALSE(utilities_is_file(path.c_str()));
}

TEST(test_filesystem, is_readable) {
  utilities_get_cwd(cwd, 1024);
  std::string path = std::string(cwd) + std::string("/test/dummy_readable_file.txt");
  EXPECT_TRUE(utilities_is_readable(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_folder");
  EXPECT_TRUE(utilities_is_readable(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_readable_writable_file.txt");
  EXPECT_TRUE(utilities_is_readable(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_nonexisting_file.txt");
  EXPECT_FALSE(utilities_is_readable(path.c_str()));
}

TEST(test_filesystem, is_writable) {
  utilities_get_cwd(cwd, 1024);
  std::string path = std::string(cwd) + std::string("/test/dummy_readable_file.txt");
  EXPECT_FALSE(utilities_is_writable(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_folder");
  EXPECT_TRUE(utilities_is_writable(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_readable_writable_file.txt");
  EXPECT_TRUE(utilities_is_writable(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_nonexisting_file.txt");
  EXPECT_FALSE(utilities_is_writable(path.c_str()));
}

TEST(test_filesystem, is_readable_and_writable) {
  utilities_get_cwd(cwd, 1024);
  std::string path = std::string(cwd) + std::string("/test/dummy_readable_file.txt");
  EXPECT_FALSE(utilities_is_readable_and_writable(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_folder");
  EXPECT_TRUE(utilities_is_readable_and_writable(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_readable_writable_file.txt");
  EXPECT_TRUE(utilities_is_readable_and_writable(path.c_str()));
  path = std::string(cwd) + std::string("/test/dummy_nonexisting_file.txt");
  EXPECT_FALSE(utilities_is_readable_and_writable(path.c_str()));
}
