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

#include <string>

#include "./allocator_testing_utils.h"

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/shared_library.h"

#include "rcutils/get_env.h"

class TestSharedLibrary : public ::testing::Test
{
protected:
  void SetUp() final
  {
    // Reset rcutil error global state in case a previously
    // running test has failed.
    rcutils_reset_error();
    lib = rcutils_get_zero_initialized_shared_library();

    library_path = std::string("libdummy_shared_library") +
    #ifdef __linux__
      std::string(".so");
    #elif __APPLE__
      std::string(".dylib");
    #elif _WIN32
      std::string(".dll");
    #else
      #error "Unsupported OS, dynamic library suffix is unknown."
    #endif

  }
  rcutils_shared_library_t lib;
  std::string library_path;
};

TEST_F(TestSharedLibrary, basic_load) {
  rcutils_ret_t ret;

  // checking rcutils_get_zero_initialized_shared_library
  ASSERT_STRNE(lib.library_path, "");
  EXPECT_TRUE(lib.lib_pointer == NULL);

  // getting shared library
  ret = rcutils_load_shared_library(&lib, library_path.c_str(), rcutils_get_default_allocator());
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  // unload shared_library
  ret = rcutils_unload_shared_library(&lib);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  // checking if we have unloaded and freed memory
  ASSERT_STRNE(lib.library_path, "");
  EXPECT_TRUE(lib.lib_pointer == NULL);
}

TEST_F(TestSharedLibrary, load_two_times) {
  rcutils_ret_t ret;

  // getting shared library
  ret = rcutils_load_shared_library(&lib, library_path.c_str(), rcutils_get_default_allocator());
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  // getting shared library
  ret = rcutils_load_shared_library(&lib, library_path.c_str(), rcutils_get_default_allocator());
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  // unload shared_library
  ret = rcutils_unload_shared_library(&lib);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
}

TEST_F(TestSharedLibrary, error_load) {
  rcutils_ret_t ret;

  rcutils_shared_library_t lib_empty;
  ret = rcutils_load_shared_library(&lib_empty, NULL, rcutils_get_zero_initialized_allocator());
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  lib_empty = rcutils_get_zero_initialized_shared_library();
  ret = rcutils_load_shared_library(&lib_empty, NULL, rcutils_get_zero_initialized_allocator());
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);

  ret = rcutils_load_shared_library(
    &lib_empty,
    library_path.c_str(), rcutils_get_zero_initialized_allocator());
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);
}

TEST_F(TestSharedLibrary, error_unload) {
  rcutils_ret_t ret;

  ret = rcutils_load_shared_library(&lib, library_path.c_str(), rcutils_get_default_allocator());
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  // unload shared_library
  ret = rcutils_unload_shared_library(&lib);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  // unload again shared_library
  ret = rcutils_unload_shared_library(&lib);
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, ret);
}

TEST_F(TestSharedLibrary, error_symbol) {
  bool is_symbol = rcutils_has_symbol(&lib, "symbol");
  EXPECT_TRUE(is_symbol == false);

  void * symbol = rcutils_get_symbol(&lib, "print_name");
  EXPECT_TRUE(symbol == NULL);
}

TEST_F(TestSharedLibrary, basic_symbol) {
  void * symbol;
  bool ret;

  symbol = rcutils_get_symbol(nullptr, "symbol");
  EXPECT_TRUE(symbol == NULL);

  ret = rcutils_has_symbol(nullptr, "symbol");
  EXPECT_FALSE(ret);

  // getting shared library
  ret = rcutils_load_shared_library(&lib, library_path.c_str(), rcutils_get_default_allocator());
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  symbol = rcutils_get_symbol(&lib, "symbol");
  EXPECT_TRUE(symbol == NULL);

  symbol = rcutils_get_symbol(&lib, "print_name");
  EXPECT_TRUE(symbol != NULL);

  // unload shared_library
  ret = rcutils_unload_shared_library(&lib);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
}
