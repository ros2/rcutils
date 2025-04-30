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
#include "./time_bomb_allocator_testing_utils.h"
#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/process.h"

static const char * const g_cmake_command = RCUTILS_STRINGIFY(CMAKE_COMMAND);

TEST(TestProcess, test_get_pid) {
  EXPECT_NE(rcutils_get_pid(), 0);
}

TEST(TestProcess, test_get_executable_name) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_allocator_t time_bomb_allocator = get_time_bomb_allocator();

  // Allocating executable_name fails
  set_time_bomb_allocator_malloc_count(time_bomb_allocator, 0);
  EXPECT_STREQ(NULL, rcutils_get_executable_name(time_bomb_allocator));

  // Allocating intermediate fails. This allocation doesn't happen on windows
#if defined __APPLE__ || defined __FreeBSD__ || (defined __GNUC__ && !defined(__MINGW64__))
  set_time_bomb_allocator_malloc_count(time_bomb_allocator, 1);
  EXPECT_STREQ(NULL, rcutils_get_executable_name(time_bomb_allocator));
#endif

  char * exec_name = rcutils_get_executable_name(allocator);
  EXPECT_STREQ("test_process", exec_name);
  allocator.deallocate(exec_name, allocator.state);
}

TEST(TestProcess, test_process_creation) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_allocator_t bad_allocator = rcutils_get_zero_initialized_allocator();
  rcutils_allocator_t time_bomb_allocator = get_time_bomb_allocator();
  rcutils_string_array_t args = rcutils_get_zero_initialized_string_array();
  rcutils_process_t * process = NULL;
  rcutils_ret_t ret = RCUTILS_RET_OK;
  int exit_code = 42;

  ret = rcutils_string_array_init(&args, 4, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  args.data[0] = strdup(g_cmake_command);
  args.data[1] = strdup("-E");
  args.data[2] = strdup("echo");
  args.data[3] = strdup("");

  EXPECT_EQ(nullptr, rcutils_start_process(NULL, &allocator));
  rcutils_reset_error();

  EXPECT_EQ(nullptr, rcutils_start_process(&args, NULL));
  rcutils_reset_error();

  EXPECT_EQ(nullptr, rcutils_start_process(&args, &bad_allocator));
  rcutils_reset_error();

  set_time_bomb_allocator_calloc_count(time_bomb_allocator, 0);
  set_time_bomb_allocator_malloc_count(time_bomb_allocator, 0);
  EXPECT_EQ(nullptr, rcutils_start_process(&args, &time_bomb_allocator));

  process = rcutils_start_process(&args, &allocator);
  EXPECT_NE(nullptr, process);

  ret = rcutils_process_wait(process, &exit_code);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(0, exit_code);

  rcutils_process_close(process);

  ret = rcutils_string_array_fini(&args);

  // cmake -E cat "file with space.txt" (returns 0)
  ret = rcutils_string_array_resize(&args, 4);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  args.data[0] = strdup(g_cmake_command);
  args.data[1] = strdup("-E");
  args.data[2] = strdup("cat");
  args.data[3] = strdup("file with space.txt");

  process = rcutils_start_process(&args, &allocator);
  EXPECT_NE(nullptr, process);

  ret = rcutils_process_wait(process, &exit_code);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(0, exit_code);

  rcutils_process_close(process);

  ret = rcutils_string_array_fini(&args);

  // cmake -E false (returns 1)
  ret = rcutils_string_array_resize(&args, 3);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  args.data[0] = strdup(g_cmake_command);
  args.data[1] = strdup("-E");
  allocator.deallocate(args.data[2], &allocator.state);
  args.data[2] = strdup("false");

  process = rcutils_start_process(&args, &allocator);
  EXPECT_NE(nullptr, process);

  ret = rcutils_process_wait(process, &exit_code);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(1, exit_code);

  rcutils_process_close(process);

  ret = rcutils_string_array_fini(&args);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
}
