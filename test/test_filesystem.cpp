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

#include "osrf_testing_tools_cpp/scope_exit.hpp"

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
}

TEST_F(TestFilesystemFixture, join_path) {
  char * path = rcutils_join_path("foo", "bar", g_allocator);
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
    g_allocator.deallocate(path, g_allocator.state);
  });
#ifdef _WIN32
  const char * ref_str = "foo\\bar";
#else
  const char * ref_str = "foo/bar";
#endif  // _WIN32
  ASSERT_FALSE(nullptr == path);
  EXPECT_STREQ(ref_str, path);
}

TEST_F(TestFilesystemFixture, to_native_path) {
  {
    char * path = rcutils_to_native_path("/foo/bar/baz", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
#ifdef _WIN32
    const char * ref_str = "\\foo\\bar\\baz";
#else
    const char * ref_str = "/foo/bar/baz";
#endif  // _WIN32
    ASSERT_FALSE(nullptr == path);
    EXPECT_STREQ(ref_str, path);
  }
  {
    char * path = rcutils_to_native_path("/foo//bar/baz", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
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
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_exists(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_exists(path));
  }
}

TEST_F(TestFilesystemFixture, is_directory) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_readable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_directory(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_directory(path));
  }
}

TEST_F(TestFilesystemFixture, is_file) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_readable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_file(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_file(path));
  }
}

TEST_F(TestFilesystemFixture, is_readable) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_readable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable(path));
  }
  {
    char * path =
      rcutils_join_path(this->test_path, "dummy_readable_writable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_nonexisting_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_readable(path));
  }
}

TEST_F(TestFilesystemFixture, is_writable) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_writable(path));
  }
  {
    char * path =
      rcutils_join_path(this->test_path, "dummy_readable_writable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_writable(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_nonexisting_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_writable(path));
  }
}

TEST_F(TestFilesystemFixture, is_readable_and_writable) {
  {
    char * path = rcutils_join_path(this->test_path, "dummy_folder", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable_and_writable(path));
  }
  {
    char * path =
      rcutils_join_path(this->test_path, "dummy_readable_writable_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_TRUE(rcutils_is_readable_and_writable(path));
  }
  {
    char * path = rcutils_join_path(this->test_path, "dummy_nonexisting_file.txt", g_allocator);
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT({
      g_allocator.deallocate(path, g_allocator.state);
    });
    ASSERT_FALSE(nullptr == path);
    EXPECT_FALSE(rcutils_is_readable_and_writable(path));
  }
}
