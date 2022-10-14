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

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "./allocator_testing_utils.h"
#include "osrf_testing_tools_cpp/scope_exit.hpp"
#include "rcutils/logging.h"
#include "rcutils/strdup.h"

TEST(TestLogging, test_logging_initialization) {
  EXPECT_FALSE(g_rcutils_logging_initialized);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());

    // Ok to shutdown after it's already been shutdown
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });
  EXPECT_TRUE(g_rcutils_logging_initialized);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  EXPECT_TRUE(g_rcutils_logging_initialized);

  EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  rcutils_allocator_t empty_allocator = rcutils_get_zero_initialized_allocator();
  EXPECT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT, rcutils_logging_initialize_with_allocator(empty_allocator));

  // Testing with a bad allocator fails when allocating internal memory
  // for the string map relating severity level values to string
  rcutils_allocator_t failing_allocator = get_failing_allocator();
  EXPECT_EQ(
    RCUTILS_RET_ERROR, rcutils_logging_initialize_with_allocator(failing_allocator));
}

size_t g_log_calls = 0;

struct LogEvent
{
  const rcutils_log_location_t * location;
  int level;
  std::string name;
  rcutils_time_point_value_t timestamp;
  std::string message;
};
LogEvent g_last_log_event;

TEST(TestLogging, test_logging) {
  EXPECT_FALSE(g_rcutils_logging_initialized);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });
  EXPECT_TRUE(g_rcutils_logging_initialized);
  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_DEBUG);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_DEBUG, rcutils_logging_get_default_logger_level());

  auto rcutils_logging_console_output_handler = [](
    const rcutils_log_location_t * location,
    int level, const char * name, rcutils_time_point_value_t timestamp,
    const char * format, va_list * args) -> void
    {
      g_log_calls += 1;
      g_last_log_event.location = location;
      g_last_log_event.level = level;
      g_last_log_event.name = name ? name : "";
      g_last_log_event.timestamp = timestamp;
      char buffer[1024];
      vsnprintf(buffer, sizeof(buffer), format, *args);
      g_last_log_event.message = buffer;
    };

  rcutils_logging_output_handler_t original_function = rcutils_logging_get_output_handler();
  rcutils_logging_set_output_handler(rcutils_logging_console_output_handler);

  EXPECT_EQ(RCUTILS_LOG_SEVERITY_DEBUG, rcutils_logging_get_default_logger_level());

  // check all attributes for a debug log message
  rcutils_log_location_t location = {"func", "file", 42u};
  g_log_calls = 0;
  rcutils_log(&location, RCUTILS_LOG_SEVERITY_DEBUG, "name1", "message %d", 11);
  EXPECT_EQ(1u, g_log_calls);
  EXPECT_TRUE(g_last_log_event.location != NULL);
  if (g_last_log_event.location) {
    EXPECT_STREQ("func", g_last_log_event.location->function_name);
    EXPECT_STREQ("file", g_last_log_event.location->file_name);
    EXPECT_EQ(42u, g_last_log_event.location->line_number);
  }
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_DEBUG, g_last_log_event.level);
  EXPECT_EQ("name1", g_last_log_event.name);
  EXPECT_EQ("message 11", g_last_log_event.message);

  // check default level
  int original_level = rcutils_logging_get_default_logger_level();
  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_ERROR);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_ERROR, rcutils_logging_get_default_logger_level());
  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_INFO, "name2", "message %d", 22);
  EXPECT_EQ(1u, g_log_calls);
  // It shouldn't be possible to set the default logger's level to UNSET.
  // Setting unset to the default logger should result in the default being restored.
  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_UNSET);
  EXPECT_EQ(RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL, rcutils_logging_get_default_logger_level());

  // check other severity levels
  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_INFO, "name3", "message %d", 33);
  EXPECT_EQ(2u, g_log_calls);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_INFO, g_last_log_event.level);
  EXPECT_EQ("name3", g_last_log_event.name);
  EXPECT_EQ("message 33", g_last_log_event.message);

  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_WARN, "", "%s", "");
  EXPECT_EQ(3u, g_log_calls);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_WARN, g_last_log_event.level);

  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_ERROR, "", "%s", "");
  EXPECT_EQ(4u, g_log_calls);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_ERROR, g_last_log_event.level);

  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_FATAL, NULL, "%s", "");
  EXPECT_EQ(5u, g_log_calls);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_FATAL, g_last_log_event.level);

  // restore original state
  rcutils_logging_set_default_logger_level(original_level);
  rcutils_logging_set_output_handler(original_function);
}

TEST(TestLogging, test_log_severity) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  int severity;
  // check supported severities
  ASSERT_EQ(
    RCUTILS_RET_OK, rcutils_logging_severity_level_from_string("UNSET", allocator, &severity));
  ASSERT_EQ(RCUTILS_LOG_SEVERITY_UNSET, severity);
  ASSERT_EQ(
    RCUTILS_RET_OK, rcutils_logging_severity_level_from_string("DEBUG", allocator, &severity));
  ASSERT_EQ(RCUTILS_LOG_SEVERITY_DEBUG, severity);
  ASSERT_EQ(
    RCUTILS_RET_OK, rcutils_logging_severity_level_from_string("INFO", allocator, &severity));
  ASSERT_EQ(RCUTILS_LOG_SEVERITY_INFO, severity);
  ASSERT_EQ(
    RCUTILS_RET_OK, rcutils_logging_severity_level_from_string("WARN", allocator, &severity));
  ASSERT_EQ(RCUTILS_LOG_SEVERITY_WARN, severity);
  ASSERT_EQ(
    RCUTILS_RET_OK, rcutils_logging_severity_level_from_string("ERROR", allocator, &severity));
  ASSERT_EQ(RCUTILS_LOG_SEVERITY_ERROR, severity);
  ASSERT_EQ(
    RCUTILS_RET_OK, rcutils_logging_severity_level_from_string("FATAL", allocator, &severity));
  ASSERT_EQ(RCUTILS_LOG_SEVERITY_FATAL, severity);
  // check case-insensitive severities
  ASSERT_EQ(
    RCUTILS_RET_OK, rcutils_logging_severity_level_from_string("info", allocator, &severity));
  ASSERT_EQ(RCUTILS_LOG_SEVERITY_INFO, severity);
  ASSERT_EQ(
    RCUTILS_RET_OK, rcutils_logging_severity_level_from_string("Info", allocator, &severity));
  ASSERT_EQ(RCUTILS_LOG_SEVERITY_INFO, severity);
  // check unknown severity
  ASSERT_EQ(
    RCUTILS_RET_LOGGING_SEVERITY_STRING_INVALID,
    rcutils_logging_severity_level_from_string("unknown", allocator, &severity));

  rcutils_allocator_t failing_allocator = get_failing_allocator();
  EXPECT_EQ(
    RCUTILS_RET_BAD_ALLOC,
    rcutils_logging_severity_level_from_string("Info", failing_allocator, &severity));
}

TEST(TestLogging, test_logger_severities) {
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });
  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_INFO);

  // check setting of acceptable severities
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_loggers", RCUTILS_LOG_SEVERITY_WARN));
  ASSERT_EQ(
    RCUTILS_LOG_SEVERITY_WARN,
    rcutils_logging_get_logger_level("rcutils_test_loggers"));
  rcutils_reset_error();
  ASSERT_EQ(
    RCUTILS_LOG_SEVERITY_WARN,
    rcutils_logging_get_logger_effective_level("rcutils_test_loggers"));
  rcutils_reset_error();
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_loggers", RCUTILS_LOG_SEVERITY_UNSET));
  ASSERT_EQ(
    rcutils_logging_get_default_logger_level(),
    rcutils_logging_get_logger_effective_level("rcutils_test_loggers"));

  // check setting of the default via empty-named logger
  int empty_name_severity = RCUTILS_LOG_SEVERITY_FATAL;
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level("", empty_name_severity));
  ASSERT_EQ(
    empty_name_severity,
    rcutils_logging_get_default_logger_level());
  ASSERT_EQ(
    empty_name_severity,
    rcutils_logging_get_logger_level(""));
  ASSERT_EQ(
    empty_name_severity,
    rcutils_logging_get_logger_effective_level(""));

  // check setting of invalid severities
  EXPECT_EQ(-1, rcutils_logging_get_logger_level(NULL));
  EXPECT_EQ(-1, rcutils_logging_get_logger_leveln(NULL, 5));
  EXPECT_EQ(-1, rcutils_logging_get_logger_effective_level(NULL));
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_logging_set_logger_level(NULL, 1));
  rcutils_reset_error();
  ASSERT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    rcutils_logging_set_logger_level("rcutils_test_loggers", -1));
  rcutils_reset_error();
  ASSERT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    rcutils_logging_set_logger_level("rcutils_test_loggers", 21));
  rcutils_reset_error();
  ASSERT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    rcutils_logging_set_logger_level("rcutils_test_loggers", 51));
  rcutils_reset_error();
  ASSERT_EQ(
    RCUTILS_RET_INVALID_ARGUMENT,
    rcutils_logging_set_logger_level("rcutils_test_loggers", 1000));
  rcutils_reset_error();

  EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  g_rcutils_logging_initialized = true;
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_UNSET, rcutils_logging_get_logger_level("rcutils_test_loggers"));
  EXPECT_EQ(
    RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID,
    rcutils_logging_set_logger_level("rcutils_test_loggers", 51));
}

TEST(TestLogging, test_logger_severity_hierarchy) {
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  // check resolving of effective thresholds in hierarchy of loggers
  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_INFO);
  int rcutils_test_logging_cpp_severity = RCUTILS_LOG_SEVERITY_WARN;
  int rcutils_test_logging_cpp_testing_severity = RCUTILS_LOG_SEVERITY_DEBUG;
  int rcutils_test_logging_cpp_testing_x_severity = RCUTILS_LOG_SEVERITY_ERROR;
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp", rcutils_test_logging_cpp_severity));
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp.testing", rcutils_test_logging_cpp_testing_severity));
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp.testing.x", rcutils_test_logging_cpp_testing_x_severity));

  EXPECT_EQ(
    rcutils_test_logging_cpp_testing_x_severity,
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp.testing.x"));
  EXPECT_EQ(
    rcutils_test_logging_cpp_testing_x_severity,
    rcutils_logging_get_logger_effective_level(
      "rcutils_test_logging_cpp.testing.x.y.x"));
  EXPECT_EQ(
    rcutils_test_logging_cpp_testing_severity,
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp.testing"));
  EXPECT_EQ(
    rcutils_test_logging_cpp_severity,
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp"));
  EXPECT_EQ(
    rcutils_test_logging_cpp_severity,
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp.testing2"));
  EXPECT_EQ(
    rcutils_logging_get_default_logger_level(),
    rcutils_logging_get_logger_effective_level(".name"));
  EXPECT_EQ(
    rcutils_logging_get_default_logger_level(),
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp_testing"));

  // check logger severities get cleared on logging restart
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  EXPECT_EQ(
    rcutils_logging_get_default_logger_level(),
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp"));

  // check hierarchies including trailing dots (considered as having an empty child name)
  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_INFO);
  int rcutils_test_logging_cpp_dot_severity = RCUTILS_LOG_SEVERITY_FATAL;
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp.", rcutils_test_logging_cpp_dot_severity));
  EXPECT_EQ(
    rcutils_test_logging_cpp_dot_severity,
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp.."));
}

TEST(TestLogging, test_logger_unset_change_ancestor) {
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  // check resolving of effective thresholds in hierarchy of loggers
  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_INFO);

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp", RCUTILS_LOG_SEVERITY_WARN));
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp.x", RCUTILS_LOG_SEVERITY_UNSET));

  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_WARN,
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp"));
  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_WARN,
    rcutils_logging_get_logger_effective_level(
      "rcutils_test_logging_cpp.x"));

  // Now change the logger level of the ancestor.  This should cause the
  // higher-level one to change as well (since it is unset).

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp", RCUTILS_LOG_SEVERITY_DEBUG));

  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_DEBUG,
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp"));
  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_DEBUG,
    rcutils_logging_get_logger_effective_level(
      "rcutils_test_logging_cpp.x"));
}

TEST(TestLogging, test_logger_set_change_ancestor) {
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  // check resolving of effective thresholds in hierarchy of loggers
  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_INFO);

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp", RCUTILS_LOG_SEVERITY_WARN));
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp.x", RCUTILS_LOG_SEVERITY_FATAL));

  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_WARN,
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp"));
  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_FATAL,
    rcutils_logging_get_logger_effective_level(
      "rcutils_test_logging_cpp.x"));

  // Now change the logger level of the ancestor.  This should not change
  // the level of the descendant, since it was set separately.

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "rcutils_test_logging_cpp", RCUTILS_LOG_SEVERITY_DEBUG));

  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_DEBUG,
    rcutils_logging_get_logger_effective_level("rcutils_test_logging_cpp"));
  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_FATAL,
    rcutils_logging_get_logger_effective_level(
      "rcutils_test_logging_cpp.x"));
}

TEST(TestLogging, test_logger_allocated_names) {
  // This tests whether we properly store and free the logger names inside
  // of logging implementation.  It's best to run this under valgrind to
  // see that there are no errors and no leaked memory.

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  rcutils_logging_set_default_logger_level(RCUTILS_LOG_SEVERITY_INFO);

  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  const char * allocated_name = rcutils_strdup("rcutils_test_loggers", allocator);

  // check setting of acceptable severities
  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      allocated_name, RCUTILS_LOG_SEVERITY_WARN));

  allocator.deallocate(const_cast<char *>(allocated_name), allocator.state);

  ASSERT_EQ(
    RCUTILS_LOG_SEVERITY_WARN,
    rcutils_logging_get_logger_level("rcutils_test_loggers"));
  rcutils_reset_error();
}

TEST(TestLogging, test_root_logger_after_nonexistent)
{
  // This tests whether the root logger remains unset after setting the logger name for a
  // non-existent logger.

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  int original_severity = rcutils_logging_get_logger_effective_level("my_internal_logger_name");

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "my_internal_logger_name", RCUTILS_LOG_SEVERITY_DEBUG));

  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_DEBUG,
    rcutils_logging_get_logger_effective_level("my_internal_logger_name"));

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "my_internal_logger_name", original_severity));

  int original_root_severity = rcutils_logging_get_logger_effective_level("");

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "", RCUTILS_LOG_SEVERITY_UNSET));

  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_UNSET,
    rcutils_logging_get_logger_effective_level(""));

  ASSERT_EQ(
    RCUTILS_RET_OK,
    rcutils_logging_set_logger_level(
      "", original_root_severity));
}

TEST(TestLogging, test_logging_macro_thread_safety)
{
  // This tests whether or not using logging macros from multiple threads is safe or not.

  // This test is based on an issue found in the optimization of the logging macros,
  // and therefore has a very specific trigger scenario, which is described more
  // in the steps below.
  // See: https://github.com/ros2/rcutils/pull/393

  // This test is likely to be flakey false-positive, meaning it's possible that
  // it will pass even if the macros are not thread-safe and may require running
  // repeatedly to detect problems.

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_initialize());
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    EXPECT_EQ(RCUTILS_RET_OK, rcutils_logging_shutdown());
  });

  // One of the loggers needs to be set "by the user" to trigger the optimization.
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_logging_set_logger_level("", RCUTILS_LOG_SEVERITY_DEBUG));

  // In threads, in a loop do a log call on many different logger names.
  // The message doesn't matter.
  std::size_t loop_count = 10;
  auto task = [&loop_count](std::size_t thread_number) {
      for (std::size_t i = 0; i < loop_count; ++i) {
        rcutils_log_location_t location = {"func", "file", 42u};
        rcutils_log(
          &location,
          RCUTILS_LOG_SEVERITY_DEBUG,
          ("some_logger_name" + std::to_string(thread_number * i)).c_str(),
          "message %d", 11);
      }
    };

  // Create many thread to increase the chance of collisions.
  std::vector<std::thread> threads;
  std::size_t number_of_threads = std::thread::hardware_concurrency() * 10;
  for (std::size_t i = 0; i < number_of_threads; ++i) {
    threads.emplace_back(task, i + 1);
  }

  // Wait for threads to complete.
  for (auto & thread : threads) {
    thread.join();
  }
}
