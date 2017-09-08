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

#if __cplusplus
extern "C"
{
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/get_env.h"
#include "rcutils/logging.h"

bool g_rcutils_logging_initialized = false;
const char * g_rcutils_logging_output_format_string = "[{severity}] [{name}]: {message} {line_number}";

rcutils_logging_output_handler_t g_rcutils_logging_output_handler = NULL;

int g_rcutils_logging_severity_threshold = 0;

void rcutils_logging_initialize()
{
  if (!g_rcutils_logging_initialized) {
    g_rcutils_logging_output_handler = &rcutils_logging_console_output_handler;
    g_rcutils_logging_severity_threshold = RCUTILS_LOG_SEVERITY_INFO;
    // Check for the environment variable to disable output formatting
    const char * output_format;
    const char * ret;
    ret = rcutils_get_env("RCUTILS_CONSOLE_OUTPUT_FORMAT", &output_format);
    if (!ret && strcmp(output_format, "") != 0) {
      g_rcutils_logging_output_format_string = output_format;
    }
    g_rcutils_logging_initialized = true;
  }
}

rcutils_logging_output_handler_t rcutils_logging_get_output_handler()
{
  return g_rcutils_logging_output_handler;
}

void rcutils_logging_set_output_handler(rcutils_logging_output_handler_t function)
{
  g_rcutils_logging_output_handler = function;
}

int rcutils_logging_get_severity_threshold()
{
  return g_rcutils_logging_severity_threshold;
}

void rcutils_logging_set_severity_threshold(int severity)
{
  RCUTILS_LOGGING_AUTOINIT
    g_rcutils_logging_severity_threshold = severity;
}

void rcutils_log(
  rcutils_log_location_t * location,
  int severity, const char * name, const char * format, ...)
{
  if (severity < g_rcutils_logging_severity_threshold) {
    return;
  }
  rcutils_logging_output_handler_t output_handler = g_rcutils_logging_output_handler;
  if (output_handler) {
    va_list args;
    va_start(args, format);
    (*output_handler)(location, severity, name, format, &args);
    va_end(args);
  }
}

void rcutils_logging_console_output_handler(
  rcutils_log_location_t * location,
  int severity, const char * name, const char * format, va_list * args)
{
  FILE * stream = NULL;
  const char * severity_string = "";
  switch (severity) {
    case RCUTILS_LOG_SEVERITY_DEBUG:
      stream = stdout;
      severity_string = "DEBUG";
      break;
    case RCUTILS_LOG_SEVERITY_INFO:
      stream = stdout;
      severity_string = "INFO";
      break;
    case RCUTILS_LOG_SEVERITY_WARN:
      stream = stderr;
      severity_string = "WARN";
      break;
    case RCUTILS_LOG_SEVERITY_ERROR:
      stream = stderr;
      severity_string = "ERROR";
      break;
    case RCUTILS_LOG_SEVERITY_FATAL:
      stream = stderr;
      severity_string = "FATAL";
      break;
    default:
      fprintf(stderr, "unknown severity level: %d\n", severity);
      return;
  }

  char buffer[1024];
  char * message_buffer = buffer;
  int written;
  {
    // use copy of args to keep args for potential later user
    va_list args_clone;
    va_copy(args_clone, *args);
    written = vsnprintf(buffer, sizeof(buffer), format, args_clone);
    va_end(args_clone);
  }
  if (written < 0) {
    fprintf(stderr, "failed to format message: '%s'\n", format);
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if ((size_t)written >= sizeof(buffer)) {
    // write was incomplete, allocate necessary memory dynamically
    size_t buffer_size = written + 1;
    void * dynamic_buffer = allocator.allocate(buffer_size, allocator.state);
    written = vsnprintf(dynamic_buffer, buffer_size, format, *args);
    if (written < 0 || (size_t)written >= buffer_size) {
      fprintf(
        stderr,
        "failed to format message (using dynamically allocated memory): '%s'\n",
        format);
      return;
    }
    message_buffer = (char *)dynamic_buffer;
  }

  if (!location) {
    // TODO before merge
    //location = empty_location;
  }

  // Process the format string looking for known tokens
  char token_start_delimiter = '{';
  char token_end_delimiter = '}';
  char output_buffer[1024];
  memset(output_buffer, '\0', sizeof(output_buffer));
  const char * str = g_rcutils_logging_output_format_string;
  size_t size = strlen(g_rcutils_logging_output_format_string);

  for (size_t i = 0; i < size; ++i) {
    if (str[i] == token_start_delimiter) {
      char token[20];
      memset(token, '\0', sizeof(token));
      size_t j = i + 1;
      // Look for a token end delimeter
      for (j = i + 1; j < size && str[j] != token_end_delimiter; j++);
      if (j >= size) {
          // No end delimiters found; there won't be any more tokens so shortcut the rest of the checking
          strcat(output_buffer, str + i);
          break;
      }
      size_t token_len = j - i - 1;  // not including delimeters
      strncpy(token, str + i+1 , token_len);
        char token_buffer[1024];
        int n = 0;
        bool known_token = false;
        if (strcmp("severity", token) == 0)
        {
          n = sprintf(token_buffer, "%s", severity_string);
          known_token = true;
        }
        if (!known_token && strcmp("line_number", token) == 0)
        {
          n = sprintf(token_buffer, "%zu", location->line_number);
          known_token = true;
        }
        if (!known_token && strcmp("message", token) == 0)
        {
          n = sprintf(token_buffer, "%s", message_buffer);
          known_token = true;
        }
        if (!known_token && strcmp("name", token) == 0)
        {
          n = sprintf(token_buffer, "%s", name);
          known_token = true;
        }
        if (known_token) {
          strncat(output_buffer, token_buffer, n);
          i += token_len + 1;  // Skip ahead to avoid re-printing these characters
          continue;
        }
    }
        strncat(output_buffer, str + i, 1);
  }
  fprintf(stream, "%s\n", output_buffer);

  if (message_buffer != buffer) {
    allocator.deallocate(message_buffer, allocator.state);
  }
}

#if __cplusplus
}
#endif
