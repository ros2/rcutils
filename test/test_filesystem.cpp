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
#include "rcutils/get_env.h"

#include "osrf_testing_tools_cpp/scope_exit.hpp"

#include "./mocking_utils/filesystem.hpp"

static rcutils_allocator_t g_allocator = rcutils_get_default_allocator();

class TestFilesystemFixture : public ::testing::Test
{
public:
  void SetUp()
  {
    EXPECT_TRUE(rcutils_get_cwd(this->cwd, sizeof(this->cwd)));

    test_path = rcutils_join_path(this->cwd, "test", g_allocator);
    ASSERT_FALSE(nullptr == test_path);
  }

  void TearDown()
  {
    g_allocator.deallocate(test_path, g_allocator.state);
  }

  char cwd[1024];
  char * test_path = nullptr;
};

TEST_F(TestFilesystemFixture, get_cwd_nullptr) {
  EXPECT_FALSE(rcutils_get_cwd(NULL, sizeof(this->cwd)));
  EXPECT_FALSE(rcutils_get_cwd(this->cwd, 0));

  // ERANGE, including a null terminating character, cwd should always be longer than 1 char
  EXPECT_FALSE(rcutils_get_cwd(this->cwd, 1));
}

TEST_F(TestFilesystemFixture, join_path) {
  char * path = rcutils_join_path("foo", "bar", g_allocator);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    g_allocator.deallocate(path, g_allocator.state);
  });
#ifdef _WIN32
  const char * ref_str = "foo\\bar";
#else
  const char * ref_str = "foo/bar";
#endif  // _WIN32
  ASSERT_FALSE(nullptr == path);
  EXPECT_STREQ(ref_str, path);

  EXPECT_STREQ(NULL, rcutils_join_path(NULL, "bar", g_allocator));
  EXPECT_STREQ(NULL, rcutils_join_path("foo", NULL, g_allocator));
}

TEST_F(TestFilesystemFixture, to_native_path) {
  {
    char * path = rcutils_to_native_path("/foo/bar/baz", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
#ifdef _WIN32
    const char * ref_str = "\\foo\\bar\\baz";
#else
    const char * ref_str = "/foo/bar/baz";
#endif  // _WIN32
    ASSERT_FALSE(nullptr == path);
    EXPECT_STREQ(ref_str, path);
    EXPECT_STREQ(NULL, rcutils_to_native_path(NULL, g_allocator));
  }
  {
    char * path = rcutils_to_native_path("/foo//bar/baz", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
#ifdef _WIN32
    const char * ref_str = "\\foo\\\\bar\\baz";
#else
    const char * ref_str = "/foo//bar/baz";
#endif  // _WIN32
    ASSERT_FALSE(nullptr == path);
    EXPECT_STREQ(ref_str, path);
  }
}

TEST_F(TestFilesystemFixture, exists) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_readable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_exists(path));
    EXPECT_FALSE(rcutils_exists("non_existent_file"));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_exists(path));
  }
}

TEST_F(TestFilesystemFixture, is_directory) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_readable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_directory(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_directory(path));
  }
}

TEST_F(TestFilesystemFixture, is_file) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_readable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_file(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_file(path));
  }
}

TEST_F(TestFilesystemFixture, is_readable) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_readable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable(path));
  }
  {
    char * path =
      rcutils_join_path(this->test_path, "dummy_readable_writable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_nonexistent_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_readable(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_nonexistent_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_readable(path));
  }
  {
    auto fs = mocking_utils::patch_filesystem("lib:rcutils");
    const char * path = "fake_unreadable_file.txt";
    fs.file_info(path).st_mode &= ~mocking_utils::filesystem::permissions::USER_READABLE;
    EXPECT_FALSE(rcutils_is_readable(path));
  }
}

TEST_F(TestFilesystemFixture, is_writable) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_writable(path));
  }
  {
    char * path =
      rcutils_join_path(this->test_path, "dummy_readable_writable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_writable(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_nonexistent_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_writable(path));
  }
  {
    auto fs = mocking_utils::patch_filesystem("lib:rcutils");
    const char * path = "fake_unwritable_file.txt";
    fs.file_info(path).st_mode &= ~mocking_utils::filesystem::permissions::USER_WRITABLE;
    EXPECT_FALSE(rcutils_is_writable(path));
  }
}

TEST_F(TestFilesystemFixture, is_readable_and_writable) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable_and_writable(path));
  }
  {
    char * path =
      rcutils_join_path(this->test_path, "dummy_readable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable_and_writable(path));
  }
  {
    auto fs = mocking_utils::patch_filesystem("lib:rcutils");
    const char * path = "fake_writable_but_unreadable_file.txt";
    fs.file_info(path).st_mode |= mocking_utils::filesystem::permissions::USER_READABLE;
    fs.file_info(path).st_mode &= ~mocking_utils::filesystem::permissions::USER_WRITABLE;
    EXPECT_FALSE(rcutils_is_readable_and_writable(path));
    EXPECT_FALSE(rcutils_is_writable(path));
    EXPECT_TRUE(rcutils_is_readable(path));
  }
  {
    char * path =
      rcutils_join_path(this->test_path, "dummy_readable_writable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable_and_writable(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_nonexisting_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_readable_and_writable(path));
  }
}

TEST_F(TestFilesystemFixture, expand_user) {
  const char * homedir = rcutils_get_home_dir();
  ASSERT_STRNE(NULL, homedir);
  const std::string homedir_str(homedir);

  {
    // Invalid path arg
    EXPECT_EQ(NULL, rcutils_expand_user(NULL, g_allocator));
  }
  {
    // No ~
    const char * path = "/this/path/has/no/tilde";
    char * ret_path = rcutils_expand_user(path, g_allocator);
    EXPECT_STREQ(ret_path, path);
    ASSERT_NE(ret_path, path);
    g_allocator.deallocate(ret_path, g_allocator.state);
  }
  {
    // No ~ and empty path
    const char * path = "";
    char * ret_path = rcutils_expand_user(path, g_allocator);
    EXPECT_STREQ(ret_path, path);
    ASSERT_NE(ret_path, path);
    g_allocator.deallocate(ret_path, g_allocator.state);
  }
  {
    // With ~ but not leading
    const char * path = "/prefix/~/my/dir";
    char * ret_path = rcutils_expand_user(path, g_allocator);
    EXPECT_STREQ(ret_path, path);
    ASSERT_NE(ret_path, path);
    g_allocator.deallocate(ret_path, g_allocator.state);
  }
  {
    // With ~ only
    const char * path = "~";
    char * ret_path = rcutils_expand_user(path, g_allocator);
    EXPECT_STREQ(ret_path, homedir);
    g_allocator.deallocate(ret_path, g_allocator.state);
  }
  {
    // With ~/ only
    const char * path = "~/";
    char * ret_path = rcutils_expand_user(path, g_allocator);
    EXPECT_STREQ(ret_path, (homedir_str + "/").c_str());
    g_allocator.deallocate(ret_path, g_allocator.state);
  }
  {
    // Normal use-case
    const char * path = "~/my/directory";
    char * ret_path = rcutils_expand_user(path, g_allocator);
    EXPECT_STREQ(ret_path, (homedir_str + "/my/directory").c_str());
    g_allocator.deallocate(ret_path, g_allocator.state);
  }
}

TEST_F(TestFilesystemFixture, mkdir) {
  {
    // Make a new directory
    char * path =
      rcutils_join_path(BUILD_DIR, "mkdir_test_dir", g_allocator);
    ASSERT_FALSE(nullptr == path);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_TRUE(rcutils_mkdir(path));
  }
  {
    // Purposely do it again, to make sure mkdir handles the case where the
    // directory already exists
    char * path =
      rcutils_join_path(BUILD_DIR, "mkdir_test_dir", g_allocator);
    ASSERT_FALSE(nullptr == path);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_TRUE(rcutils_mkdir(path));
  }
  {
    ASSERT_FALSE(rcutils_mkdir(nullptr));
    ASSERT_FALSE(rcutils_mkdir(""));
  }
  {
    ASSERT_FALSE(rcutils_mkdir("foo/bar"));
  }
  {
    // Make sure it throws an error when the intermediate path doesn't exist
    char * path =
      rcutils_join_path(BUILD_DIR, "mkdir_test_dir2", g_allocator);
    ASSERT_FALSE(nullptr == path);
    char * path2 =
      rcutils_join_path(path, "mkdir_test_dir3", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      g_allocator.deallocate(path2, g_allocator.state);
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path2);
    ASSERT_FALSE(rcutils_mkdir(path2));
  }
}

TEST_F(TestFilesystemFixture, calculate_directory_size) {
  char * path =
    rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
  size_t size = rcutils_calculate_directory_size(path, g_allocator);
#ifdef WIN32
  // Due to different line breaks on windows, we have one more byte in the file.
  // See https://github.com/ros2/rcutils/issues/198
  EXPECT_EQ(6u, size);
#else
  EXPECT_EQ(5u, size);
#endif
  g_allocator.deallocate(path, g_allocator.state);

  char * non_existing_path = rcutils_join_path(this->test_path, "non_existing_folder", g_allocator);
  size = rcutils_calculate_directory_size(non_existing_path, g_allocator);
  EXPECT_EQ(0u, size);
  g_allocator.deallocate(non_existing_path, g_allocator.state);

  {
    auto fs = mocking_utils::patch_filesystem("lib:rcutils");
    const char * path = "some_fake_directory/some_fake_folder";
    fs.file_info(path).st_mode |= mocking_utils::filesystem::file_types::DIRECTORY;
    fs.exhaust_file_descriptors();
    size = rcutils_calculate_directory_size(path, g_allocator);
    EXPECT_EQ(0u, size);
  }
}

TEST_F(TestFilesystemFixture, calculate_file_size) {
  char * path =
    rcutils_join_path(this->test_path, "dummy_readable_file.txt", g_allocator);
  size_t size = rcutils_get_file_size(path);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    g_allocator.deallocate(path, g_allocator.state);
  });
#ifdef WIN32
  // Due to different line breaks on windows, we have one more byte in the file.
  // See https://github.com/ros2/rcutils/issues/198
  ASSERT_EQ(6u, size);
#else
  ASSERT_EQ(5u, size);
#endif

  char * non_existing_path =
    rcutils_join_path(this->test_path, "non_existing_file.txt", g_allocator);
  size = rcutils_get_file_size(non_existing_path);
  ASSERT_EQ(0u, size);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    g_allocator.deallocate(non_existing_path, g_allocator.state);
  });
}
