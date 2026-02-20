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

#include <cstdlib>
#include <string>
#include <vector>

#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcutils/logging.h"
#include "rcutils/types/char_array.h"

#ifdef _WIN32
#define test_setenv(name, value) _putenv_s(name, value)
#define test_unsetenv(name) _putenv_s(name, "")
#else
#define test_setenv(name, value) setenv(name, value, 1)
#define test_unsetenv(name) unsetenv(name)
#endif

static void call_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * format, ...)
{
  va_list args;
  va_start(args, format);
  rcutils_logging_console_output_handler(location, severity, name, timestamp, format, &args);
  va_end(args);
}

// There are no outputs of the handler function, and the only result are fprintf() calls.
// This is just a smoke test to check that the code can handle simple inputs cleanly.
TEST(TestLoggingConsoleOutputHandler, typical_inputs) {
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  rcutils_log_location_t log_location = {
    "test_function",
    "test_file",
    1,
  };
  const char * log_name = "test_name";
  rcutils_time_point_value_t timestamp = 1;
  const char * format = "%s - %s";
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_DEBUG, log_name, timestamp, format, "part1", "part2");
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_INFO, log_name, timestamp, format, "part1", "part2");
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_WARN, log_name, timestamp, format, "part1", "part2");
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_ERROR, log_name, timestamp, format, "part1", "part2");
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_FATAL, log_name, timestamp, format, "part1", "part2");
}

// There are no outputs of the handler function, and the only result are fprintf() calls.
// This is just a smoke test to check that the code can handle bad inputs cleanly.
TEST(TestLoggingConsoleOutputHandler, bad_inputs) {
  rcutils_log_location_t log_location = {
    "test_function",
    "test_file",
    1,
  };
  const char * log_name = "test_name";
  rcutils_time_point_value_t timestamp = 1;
  const char * format = "%s - %s";

  // Check !g_rcutils_logging_initialized
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_DEBUG, log_name, timestamp, format, "part1", "part2");

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  call_handler(
    nullptr, RCUTILS_LOG_SEVERITY_INFO, log_name, timestamp, format, "part1", "part2");
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_UNSET, log_name, timestamp, format, "part1", "part2");
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_INFO, nullptr, timestamp, format, "part1", "part2");
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_INFO, log_name, 0, format, "part1", "part2");

  // If format is NULL, this call will segfault on some (but not all) systems
  call_handler(
    &log_location, RCUTILS_LOG_SEVERITY_INFO, log_name, timestamp, "bad format", "part1", "part2");
}

TEST(TestLoggingConsoleOutputHandler, short_file_name_extracts_basename) {
  // Set the output format to use {short_file_name} before initializing
  test_setenv("RCUTILS_CONSOLE_OUTPUT_FORMAT", "{short_file_name}");
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    test_unsetenv("RCUTILS_CONSOLE_OUTPUT_FORMAT");
  });

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_char_array_t output_buf;
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_char_array_init(&output_buf, 1024, &allocator));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_char_array_fini(&output_buf));
  });

  rcutils_log_location_t location = {
    "test_function",
    "/some/long/path/to/my_source_file.cpp",
    42,
  };

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_format_message(
      &location, RCUTILS_LOG_SEVERITY_INFO, "test_logger", 0,
      "hello", &output_buf));

  std::string output(output_buf.buffer);
  EXPECT_NE(std::string::npos, output.find("my_source_file.cpp"))
    << "Expected basename in output: " << output;
  EXPECT_EQ(std::string::npos, output.find("/some/long/path/to/"))
    << "Full path should not appear in output: " << output;
}

TEST(TestLoggingConsoleOutputHandler, short_file_name_without_path_unchanged) {
  test_setenv("RCUTILS_CONSOLE_OUTPUT_FORMAT", "{short_file_name}");
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    test_unsetenv("RCUTILS_CONSOLE_OUTPUT_FORMAT");
  });

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_char_array_t output_buf;
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_char_array_init(&output_buf, 1024, &allocator));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_char_array_fini(&output_buf));
  });

  rcutils_log_location_t location = {
    "test_function",
    "bare_file.cpp",
    10,
  };

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_format_message(
      &location, RCUTILS_LOG_SEVERITY_INFO, "test_logger", 0,
      "test", &output_buf));

  std::string output(output_buf.buffer);
  EXPECT_NE(std::string::npos, output.find("bare_file.cpp"))
    << "Expected bare filename in output: " << output;
}
