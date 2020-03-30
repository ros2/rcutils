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

#include "rcutils/directory_and_file_reader.h"
#include "rcutils/filesystem.h"

class TestDirectoryAndFileReader : public ::testing::Test
{
public:
  void SetUp()
  {
    allocator = rcutils_get_default_allocator();
    g_allocator = rcutils_get_default_allocator();
    EXPECT_TRUE(rcutils_get_cwd(this->cwd, sizeof(this->cwd)));

    test_path = rcutils_join_path(this->cwd, "test/dummy_folder", g_allocator);
    ASSERT_FALSE(nullptr == test_path);

    dir = rcutils_get_zero_initialized_dir();
  }

  void TearDown()
  {
    g_allocator.deallocate(test_path, g_allocator.state);
  }

  rcutils_allocator_t g_allocator;

  rcutils_allocator_t allocator;
  char cwd[1024];
  char * test_path = nullptr;
  rcutils_dir_t dir;
};

TEST_F(TestDirectoryAndFileReader, basic_open) {
  rcutils_ret_t ret;
  ret = rcutils_open_dir(&dir, test_path, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ASSERT_TRUE(dir.has_next);

  rcutils_file_t dummy_file = rcutils_get_zero_initialized_file();
  ret = rcutils_readfile(&dir, &dummy_file);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ASSERT_STRNE(dummy_file.name, "");
  ASSERT_TRUE(dir.has_next);

  ret = rcutils_next_dir(&dir);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ret = rcutils_readfile(&dir, &dummy_file);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ASSERT_STRNE(dummy_file.name, "");
  ASSERT_TRUE(dir.has_next);

  ret = rcutils_next_dir(&dir);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ret = rcutils_readfile(&dir, &dummy_file);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ASSERT_STRNE(dummy_file.name, "");
  ASSERT_TRUE(dir.has_next);

  ret = rcutils_next_dir(&dir);
  ASSERT_EQ(RCUTILS_RET_ERROR, ret);
  ASSERT_FALSE(dir.has_next);

  ret = rcutils_file_fini(&dummy_file, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_close_dir(&dir);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
}

TEST_F(TestDirectoryAndFileReader, open_two_times) {
  rcutils_ret_t ret;
  ret = rcutils_open_dir(&dir, test_path, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ASSERT_TRUE(dir.has_next);

  ret = rcutils_open_dir(&dir, test_path, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  allocator.deallocate(dir.path, allocator.state);
  dir.path = NULL;
  ret = rcutils_open_dir(&dir, test_path, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_close_dir(&dir);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
}

TEST_F(TestDirectoryAndFileReader, unload_two_times) {
  rcutils_ret_t ret;
  ret = rcutils_open_dir(&dir, test_path, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_close_dir(&dir);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_close_dir(&dir);
  ASSERT_EQ(RCUTILS_RET_ERROR, ret);

  rcutils_dir_t * dir_null = NULL;
  ret = rcutils_close_dir(dir_null);
  EXPECT_TRUE(dir_null == NULL);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);
}

TEST_F(TestDirectoryAndFileReader, readdir_failures) {
  rcutils_ret_t ret;
  rcutils_dir_t dir_null;
  rcutils_file_t dummy_file = rcutils_get_zero_initialized_file();
  ret = rcutils_readfile(&dir_null, &dummy_file);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  rcutils_dir_t * dir_null2 = NULL;
  ret = rcutils_readfile(dir_null2, &dummy_file);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  rcutils_file_t * dummy_file2 = NULL;
  ret = rcutils_readfile(dir_null2, dummy_file2);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  ret = rcutils_open_dir(&dir, test_path, allocator);
  allocator.deallocate(dir.path, allocator.state);
  dir.path = NULL;
  ret = rcutils_readfile(&dir, &dummy_file);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  ret = rcutils_file_fini(&dummy_file, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_close_dir(&dir);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
}

TEST_F(TestDirectoryAndFileReader, readfile_failures) {
  rcutils_ret_t ret;
  rcutils_dir_t dir_null;
  rcutils_file_t dummy_file = rcutils_get_zero_initialized_file();
  ret = rcutils_readfile(&dir_null, &dummy_file);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  rcutils_dir_t * dir_null2 = NULL;
  ret = rcutils_readfile(dir_null2, &dummy_file);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  rcutils_file_t * dummy_file2 = NULL;
  ret = rcutils_readfile(dir_null2, dummy_file2);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  ret = rcutils_open_dir(&dir, test_path, allocator);
  allocator.deallocate(dir.path, allocator.state);
  dir.path = NULL;
  ret = rcutils_readfile(&dir, &dummy_file);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  ret = rcutils_file_fini(&dummy_file, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_close_dir(&dir);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
}

TEST_F(TestDirectoryAndFileReader, next_dir_failures) {
  rcutils_ret_t ret;
  rcutils_dir_t * dir_null = NULL;
  ret = rcutils_next_dir(dir_null);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  dir_null = reinterpret_cast<rcutils_dir_t*>(
    g_allocator.allocate(sizeof(rcutils_dir_t), allocator.state));
  if (dir_null == NULL) {
    FAIL() << "bad alloc";
  }
  *dir_null = rcutils_get_zero_initialized_dir();
  ret = rcutils_open_dir(dir_null, test_path, allocator);
  allocator.deallocate(dir_null->path, allocator.state);
  dir_null->path = NULL;
  ret = rcutils_next_dir(dir_null);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_close_dir(dir_null);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  g_allocator.deallocate(dir_null, g_allocator.state);
}

TEST_F(TestDirectoryAndFileReader, file_struct_tests) {
  rcutils_ret_t ret;
  rcutils_file_t * dummy_file = NULL;

  ret = rcutils_file_fini(dummy_file, allocator);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  dummy_file = reinterpret_cast<rcutils_file_t*>(
    g_allocator.allocate(sizeof(rcutils_file_t), allocator.state));
  if (dummy_file == NULL) {
    FAIL() << "bad alloc";
  }

  *dummy_file = rcutils_get_zero_initialized_file();
  ret = rcutils_file_fini(dummy_file, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  g_allocator.deallocate(dummy_file, g_allocator.state);
}

TEST_F(TestDirectoryAndFileReader, check_init_struct_tests) {
  ASSERT_STRNE(dir.path, "");
  EXPECT_TRUE(dir.has_next == 0);
  EXPECT_TRUE(dir.dir == NULL);
  EXPECT_TRUE(dir.f == NULL);

  rcutils_file_t dummy_file = rcutils_get_zero_initialized_file();
  ASSERT_STRNE(dummy_file.path, "");
  ASSERT_STRNE(dummy_file.name, "");
  EXPECT_TRUE(dummy_file.is_dir == 0);
}
