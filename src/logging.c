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
#include "rcutils/snprintf.h"

bool g_rcutils_logging_initialized = false;
char g_rcutils_logging_output_format_string[1024];
static char * rcutils_default_output_format =
  "[{severity}] [{name}]: {message} ({function_name}() at {file_name}:{line_number})";

rcutils_logging_output_handler_t g_rcutils_logging_output_handler = NULL;

int g_rcutils_logging_severity_threshold = 0;

void rcutils_logging_initialize()
{
  if (!g_rcutils_logging_initialized) {
    g_rcutils_logging_output_handler = &rcutils_logging_console_output_handler;
    g_rcutils_logging_severity_threshold = RCUTILS_LOG_SEVERITY_INFO;

    int max_output_format_len = sizeof(g_rcutils_logging_output_format_string);
    memset(g_rcutils_logging_output_format_string, '\0', max_output_format_len);
    // Check for the environment variable for custom output formatting
    const char * output_format;
    const char * ret;
    ret = rcutils_get_env("RCUTILS_CONSOLE_OUTPUT_FORMAT", &output_format);
    if (!ret && strcmp(output_format, "") != 0) {
      strncpy(g_rcutils_logging_output_format_string, output_format, max_output_format_len - 1);
    } else {
      strncpy(g_rcutils_logging_output_format_string, rcutils_default_output_format, \
        max_output_format_len - 1);
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

#define rcutils_logging_ensure_large_enough_buffer(required_output_buffer_size, output_buffer_size, allocator, output_buffer, static_output_buffer) \
      printf("Required output buffer size will be: %zu\n", required_output_buffer_size); \
      if (required_output_buffer_size >= output_buffer_size) { \
        do { \
          output_buffer_size *= 2; \
        } while (required_output_buffer_size >= output_buffer_size); \
        if (output_buffer == static_output_buffer) \
        { \
          printf("Allocating new memory\n"); \
          void * dynamic_output_buffer = allocator.allocate(output_buffer_size, allocator.state); \
          memset(dynamic_output_buffer, '\0', output_buffer_size); \
          strncpy(dynamic_output_buffer, output_buffer, strlen(output_buffer)); \
          output_buffer = (char *)dynamic_output_buffer; \
        } else { \
          printf("Reallocating memory\n"); \
          output_buffer = (char *)allocator.reallocate(output_buffer, output_buffer_size, allocator.state); \
        } \
      }

void rcutils_logging_console_output_handler(
  rcutils_log_location_t * location_,
  int severity, const char * name, const char * format, va_list * args)
{
  rcutils_log_location_t * location = location_;
  rcutils_log_location_t dummy_location = {"", "", 0};
  if (!location) {
    location = &dummy_location;
  }

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

  // Process the format string looking for known tokens
  char token_start_delimiter = '{';
  char token_end_delimiter = '}';
  char static_output_buffer[1024];
  memset(static_output_buffer, '\0', sizeof(static_output_buffer));
  // Start with a fixed size buffer and if during token expansion we need longer,
  // we'll dynamically allocate space.
  char * output_buffer = static_output_buffer;
  size_t output_buffer_size = sizeof(static_output_buffer);
  const char * str = g_rcutils_logging_output_format_string;
  size_t size = strlen(g_rcutils_logging_output_format_string);

  // Walk through the format string and expand tokens when they're encountered.
  // Each token expansion will be truncated, with the exception of the message.
  int n = 0;
  for (size_t i = 0; i < size; ++i) {
    printf("Current output buffer size: %zu\n", strlen(output_buffer));
    if (str[i] != token_start_delimiter) {
      // TODO(dhood) before merge: dynamically allocate more space if required.
      strncat(output_buffer, str + i, 1);
      continue;
    }
    // Found a token start delimiter: determine if there's a token or not.
    char token[1024];  // No token can be longer than the max format string length.
    memset(token, '\0', sizeof(token));
    char static_token_buffer[1024];  // Tokens can't expand to more than this; they'll be truncated.
    char * token_buffer = static_token_buffer;
    size_t j;
    // Look for a token end delimiter.
    for (j = i + 1; j < size && str[j] != token_end_delimiter; j++) {
    }
    if (j >= size) {
      // No end delimiters found in the remainder of the format string;
      // there won't be any more tokens so shortcut the rest of the checking.
      // TODO(dhood) before merge: dynamically allocate more space if required.
      strncat(output_buffer, token_buffer, n);
      break;
    }
    // Found what looks like a token; determine if it's recognized.
    size_t token_len = j - i - 1;  // not including delimiters
    strncpy(token, str + i + 1, token_len);
    if (strcmp("severity", token) == 0) {
      n = rcutils_snprintf(token_buffer, sizeof(static_token_buffer), "%s", severity_string);
    } else if (strcmp("name", token) == 0) {
      n = rcutils_snprintf(token_buffer, sizeof(static_token_buffer), "%s", name);
    } else if (strcmp("message", token) == 0) {
      // This has already been expanded above and is able to be larger than the static size.
      token_buffer = message_buffer;
      n = strlen(token_buffer);
    } else if (strcmp("function_name", token) == 0) {
      token_buffer = message_buffer;
      n = strlen(token_buffer);
      //n =
        //rcutils_snprintf(token_buffer, sizeof(static_token_buffer), "%s", location->function_name);
    } else if (strcmp("file_name", token) == 0) {
      n = rcutils_snprintf(token_buffer, sizeof(static_token_buffer), "%s", location->file_name);
    } else if (strcmp("line_number", token) == 0) {
      n = rcutils_snprintf(token_buffer, sizeof(static_token_buffer), "%zu", location->line_number);
    } else {
      // This wasn't a token; print the start delimiter and continue the search as usual
      // (the format string might contain more start delimiters)
      // TODO(dhood) before merge: dynamically allocate more space if required.
      strncat(output_buffer, str + i, 1);
      continue;
    }

    if (n >= 0) {
      size_t required_output_buffer_size = strlen(output_buffer) + n;
      printf("Token expansion size=%i\n", n);
      rcutils_logging_ensure_large_enough_buffer(required_output_buffer_size, output_buffer_size, allocator, output_buffer, static_output_buffer);
      strncat(output_buffer, token_buffer, n);
      i += token_len + 1;  // Skip ahead to avoid re-printing these characters
      continue;
    }
  }
  fprintf(stream, "%s\n", output_buffer);

  if (message_buffer != buffer) {
    allocator.deallocate(message_buffer, allocator.state);
  }

  if (output_buffer != static_output_buffer) {
    allocator.deallocate(output_buffer, allocator.state);
  }
}

#if __cplusplus
}
#endif
