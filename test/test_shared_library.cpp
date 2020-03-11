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
    allocator = rcutils_get_default_allocator();
    lib = rcutils_get_zero_initialized_shared_library();
  }
  rcutils_allocator_t allocator;
  rcutils_shared_library_t lib;
};

TEST_F(TestSharedLibrary, getters_initialize_to_zero) {
  rcutils_ret_t ret;

  // checking rcutils_get_zero_initialized_shared_library
  ASSERT_STRNE(lib.library_path, "");
  EXPECT_TRUE(lib.lib_pointer == NULL);

  // Getting RMW_IMPLEMENTATION env var to get the shared library
  const char * env_var{};
  const char * err = rcutils_get_env("RMW_IMPLEMENTATION", &env_var);
  // Is there any error getting the env var?
  ASSERT_STRNE(err, "");
  // library path is not empty
  const std::string library_path = std::string("libdummy_shared_library.so");
  EXPECT_FALSE(library_path.empty());

  // allocating memory to
  lib.library_path = reinterpret_cast<char *>(allocator.allocate(
      (library_path.length() + 1) * sizeof(char),
      allocator.state));

  // checking allocation was fine
  ASSERT_NE(lib.library_path, nullptr);
  // copying string
  snprintf(lib.library_path, library_path.length() + 1, "%s", library_path.c_str());

  // getting shared library
  ret = rcutils_get_shared_library(&lib);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  // unload shared_library
  ret = rcutils_unload_library(&lib, allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  // checking if we have unloaded and freed memory
  ASSERT_STRNE(lib.library_path, "");
  EXPECT_TRUE(lib.lib_pointer == NULL);
}
