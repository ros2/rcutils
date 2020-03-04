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

#include "./allocator_testing_utils.h"
#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/process.h"

TEST(TestProcess, test_get_pid) {
  EXPECT_NE(rcutils_get_pid(), 0);
}

TEST(TestProcess, test_get_executable_name) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_allocator_t failing_allocator = get_failing_allocator();

  EXPECT_STREQ(NULL, rcutils_get_executable_name(failing_allocator));
  char * exec_name = rcutils_get_executable_name(allocator);
  EXPECT_STREQ("test_process", exec_name);
  allocator.deallocate(exec_name, allocator.state);
}
