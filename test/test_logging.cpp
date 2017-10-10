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
#include <vector>

#include "rcutils/logging.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

TEST(CLASSNAME(TestLogging, RMW_IMPLEMENTATION), test_logging_initialization) {
  EXPECT_FALSE(g_rcutils_logging_initialized);
  rcutils_logging_initialize();
  EXPECT_TRUE(g_rcutils_logging_initialized);
  rcutils_logging_initialize();
  EXPECT_TRUE(g_rcutils_logging_initialized);
  g_rcutils_logging_initialized = false;
  EXPECT_FALSE(g_rcutils_logging_initialized);
}

size_t g_log_calls = 0;

struct LogEvent
{
  rcutils_log_location_t * location;
  int level;
  std::string name;
  std::string message;
};
LogEvent g_last_log_event;

TEST(CLASSNAME(TestLogging, RMW_IMPLEMENTATION), test_logging) {
  EXPECT_FALSE(g_rcutils_logging_initialized);
  rcutils_logging_initialize();
  EXPECT_TRUE(g_rcutils_logging_initialized);
  g_rcutils_logging_root_logger_severity_threshold = RCUTILS_LOG_SEVERITY_DEBUG;
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_DEBUG, g_rcutils_logging_root_logger_severity_threshold);

  auto rcutils_logging_console_output_handler = [](
    rcutils_log_location_t * location,
    int level, const char * name, const char * format, va_list * args) -> void
    {
      g_log_calls += 1;
      g_last_log_event.location = location;
      g_last_log_event.level = level;
      g_last_log_event.name = name ? name : "";
      char buffer[1024];
      vsnprintf(buffer, sizeof(buffer), format, *args);
      g_last_log_event.message = buffer;
    };

  rcutils_logging_output_handler_t original_function = rcutils_logging_get_output_handler();
  rcutils_logging_set_output_handler(rcutils_logging_console_output_handler);

  EXPECT_EQ(RCUTILS_LOG_SEVERITY_DEBUG, rcutils_logging_get_logger_severity_threshold(""));

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

  // check global severity threshold
  int original_severity_threshold = rcutils_logging_get_logger_severity_threshold("");
  rcutils_logging_set_logger_severity_threshold("", RCUTILS_LOG_SEVERITY_INFO);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_INFO, rcutils_logging_get_logger_severity_threshold(""));
  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_DEBUG, "name2", "message %d", 22);
  EXPECT_EQ(1u, g_log_calls);

  // check other severity levels
  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_INFO, "name3", "message %d", 33);
  EXPECT_EQ(2u, g_log_calls);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_INFO, g_last_log_event.level);
  EXPECT_EQ("name3", g_last_log_event.name);
  EXPECT_EQ("message 33", g_last_log_event.message);

  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_WARN, "", "");
  EXPECT_EQ(3u, g_log_calls);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_WARN, g_last_log_event.level);

  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_ERROR, "", "");
  EXPECT_EQ(4u, g_log_calls);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_ERROR, g_last_log_event.level);

  rcutils_log(NULL, RCUTILS_LOG_SEVERITY_FATAL, NULL, "");
  EXPECT_EQ(5u, g_log_calls);
  EXPECT_EQ(RCUTILS_LOG_SEVERITY_FATAL, g_last_log_event.level);

  // check resolving of effective thresholds in hierarchy of loggers
  rcutils_logging_set_logger_severity_threshold(
    "rcutils_test_logging_cpp", RCUTILS_LOG_SEVERITY_WARN);
  rcutils_logging_set_logger_severity_threshold(
    "rcutils_test_logging_cpp.testing", RCUTILS_LOG_SEVERITY_DEBUG);
  rcutils_logging_set_logger_severity_threshold(
    "rcutils_test_logging_cpp.testing.x", RCUTILS_LOG_SEVERITY_ERROR);

  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_ERROR,
    rcutils_logging_get_logger_effective_threshold("rcutils_test_logging_cpp.testing.x"));
  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_ERROR,
    rcutils_logging_get_logger_effective_threshold("rcutils_test_logging_cpp.testing.x.y.x"));
  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_DEBUG,
    rcutils_logging_get_logger_effective_threshold("rcutils_test_logging_cpp.testing"));
  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_WARN,
    rcutils_logging_get_logger_effective_threshold("rcutils_test_logging_cpp"));
  EXPECT_EQ(
    RCUTILS_LOG_SEVERITY_WARN,
    rcutils_logging_get_logger_effective_threshold("rcutils_test_logging_cpp.testing2"));

  // restore original state
  rcutils_logging_set_logger_severity_threshold("", original_severity_threshold);
  rcutils_logging_set_output_handler(original_function);
  g_rcutils_logging_initialized = false;
  EXPECT_FALSE(g_rcutils_logging_initialized);
}
