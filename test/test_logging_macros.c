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

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/logging_macros.h"
#include "rcutils/types/rcutils_ret.h"

size_t g_log_calls = 0;

struct LogEvent
{
  const rcutils_log_location_t * location;
  int severity;
  const char * name;
  char * message;
};
struct LogEvent g_last_log_event;

void custom_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, const char * format, va_list * args)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  g_log_calls += 1;
  g_last_log_event.location = location;
  g_last_log_event.severity = severity;
  g_last_log_event.name = name ? name : "";
  if (g_last_log_event.message) {
    allocator.deallocate(g_last_log_event.message, allocator.state);
  }
  const size_t size = 1024;
  g_last_log_event.message = allocator.allocate(size, allocator.state);
  vsnprintf(g_last_log_event.message, size, format, *args);
}

int main(int argc, char ** argv)
{
  (void)argc;
  (void)argv;

  g_log_calls = 0;
  if (g_rcutils_logging_initialized) {
    return 1;
  }
  rcutils_ret_t ret = rcutils_logging_initialize();
  if (ret != RCUTILS_RET_OK || !g_rcutils_logging_initialized) {
    return 2;
  }

  rcutils_logging_output_handler_t previous_output_handler = \
    rcutils_logging_get_output_handler();
  rcutils_logging_set_output_handler(custom_handler);

  size_t line_number = __LINE__; RCUTILS_LOG_INFO("empty message");
  if (g_log_calls != 1u) {
    return 3;
  }
  if (NULL == g_last_log_event.location) {
    return 4;
  }
  if (strcmp(g_last_log_event.location->function_name, "main")) {
    return 5;
  }
  if (g_last_log_event.location->line_number != line_number) {
    return 6;
  }
  if (g_last_log_event.severity != RCUTILS_LOG_SEVERITY_INFO) {
    return 7;
  }
  if (strcmp(g_last_log_event.name, "")) {
    return 8;
  }
  if (strcmp(g_last_log_event.message, "empty message")) {
    return 9;
  }

  line_number = __LINE__; RCUTILS_LOG_INFO("message %s", "foo");
  if (g_log_calls != 2u) {
    return 10;
  }
  if (NULL == g_last_log_event.location) {
    return 11;
  }
  if (strcmp(g_last_log_event.location->function_name, "main")) {
    return 12;
  }
  if (g_last_log_event.location->line_number != line_number) {
    return 13;
  }
  if (g_last_log_event.severity != RCUTILS_LOG_SEVERITY_INFO) {
    return 14;
  }
  if (strcmp(g_last_log_event.name, "")) {
    return 15;
  }
  if (strcmp(g_last_log_event.message, "message foo")) {
    return 16;
  }

  rcutils_logging_set_output_handler(previous_output_handler);
  if (g_last_log_event.message) {
    free(g_last_log_event.message);
  }

  ret = rcutils_logging_shutdown();
  if (ret != RCUTILS_RET_OK || g_rcutils_logging_initialized) {
    return 17;
  }
}
