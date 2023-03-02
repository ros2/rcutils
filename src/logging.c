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

#ifdef __cplusplus
extern "C"
{
#endif

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
# include <io.h>
// When building with MSVC 19.28.29333.0 on Windows 10 (as of 2020-11-11),
// there appears to be a problem with winbase.h (which is included by
// Windows.h).  In particular, warnings of the form:
//
// warning C5105: macro expansion producing 'defined' has undefined behavior
//
// See https://developercommunity.visualstudio.com/content/problem/695656/wdk-and-sdk-are-not-compatible-with-experimentalpr.html
// for more information.  For now disable that warning when including windows.h
# pragma warning(push)
# pragma warning(disable : 5105)
# include <windows.h>
# pragma warning(pop)
#else
# include <unistd.h>
#endif

#include "rcutils/allocator.h"
#include "rcutils/env.h"
#include "rcutils/error_handling.h"
#include "rcutils/find.h"
#include "rcutils/format_string.h"
#include "rcutils/logging.h"
#include "rcutils/snprintf.h"
#include "rcutils/strdup.h"
#include "rcutils/strerror.h"
#include "rcutils/time.h"
#include "rcutils/types/string_map.h"


#define RCUTILS_LOGGING_SEPARATOR_CHAR '.'

#define RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN (2048)

#if defined(_WIN32)
// Used with setvbuf, and size must be 2 <= size <= INT_MAX. For more info, see:
// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setvbuf
#define RCUTILS_LOGGING_STREAM_BUFFER_SIZE (BUFSIZ)
#else
// Let the system choose the appropriate size
// glibc reference: https://sourceware.org/git/?p=glibc.git;a=blob;f=libio/iosetvbuf.c;hb=HEAD
// OSX reference: https://opensource.apple.com/source/Libc/Libc-166/stdio.subproj/setvbuf.c.auto.html
#define RCUTILS_LOGGING_STREAM_BUFFER_SIZE (0)
#endif

const char * const g_rcutils_log_severity_names[] = {
  [RCUTILS_LOG_SEVERITY_UNSET] = "UNSET",
  [RCUTILS_LOG_SEVERITY_DEBUG] = "DEBUG",
  [RCUTILS_LOG_SEVERITY_INFO] = "INFO",
  [RCUTILS_LOG_SEVERITY_WARN] = "WARN",
  [RCUTILS_LOG_SEVERITY_ERROR] = "ERROR",
  [RCUTILS_LOG_SEVERITY_FATAL] = "FATAL",
};

enum rcutils_colorized_output
{
  RCUTILS_COLORIZED_OUTPUT_FORCE_DISABLE = 0,
  RCUTILS_COLORIZED_OUTPUT_FORCE_ENABLE = 1,
  RCUTILS_COLORIZED_OUTPUT_AUTO = 2,
};

bool g_rcutils_logging_initialized = false;

char g_rcutils_logging_output_format_string[RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN];
static const char * g_rcutils_logging_default_output_format =
  "[{severity}] [{time}] [{name}]: {message}";

static rcutils_allocator_t g_rcutils_logging_allocator;

rcutils_logging_output_handler_t g_rcutils_logging_output_handler = NULL;
static rcutils_string_map_t g_rcutils_logging_severities_map;

// If this is false, attempts to use the severities map will be skipped.
// This can happen if allocation of the map fails at initialization.
bool g_rcutils_logging_severities_map_valid = false;

int g_rcutils_logging_default_logger_level = 0;

static FILE * g_output_stream = NULL;

enum rcutils_colorized_output g_colorized_output = RCUTILS_COLORIZED_OUTPUT_AUTO;

rcutils_ret_t rcutils_logging_initialize(void)
{
  return rcutils_logging_initialize_with_allocator(rcutils_get_default_allocator());
}

enum rcutils_get_env_retval
{
  RCUTILS_GET_ENV_ERROR = -1,
  RCUTILS_GET_ENV_ZERO = 0,
  RCUTILS_GET_ENV_ONE = 1,
  RCUTILS_GET_ENV_EMPTY = 2,
};

// A utility function to get zero or one from an environment variable.
// Returns RCUTILS_GET_ENV_ERROR if we failed to get the environment variable
// or if it was something we don't understand.
// Return RCUTILS_GET_ENV_ZERO if the value in the environment variable is "0",
// RCUTILS_GET_ENV_ONE if the value in the environment variable is "1", or
// RCUTILS_GET_ENV_EMPTY if the environment variables is empty.
static enum rcutils_get_env_retval rcutils_get_env_var_zero_or_one(
  const char * name, const char * zero_semantic,
  const char * one_semantic)
{
  const char * env_var_value = NULL;
  const char * ret_str = rcutils_get_env(name, &env_var_value);
  if (NULL != ret_str) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting environment variable %s: %s", name,
      ret_str);
    return RCUTILS_GET_ENV_ERROR;
  }

  if (strcmp(env_var_value, "") == 0) {
    return RCUTILS_GET_ENV_EMPTY;
  }
  if (strcmp(env_var_value, "0") == 0) {
    return RCUTILS_GET_ENV_ZERO;
  }
  if (strcmp(env_var_value, "1") == 0) {
    return RCUTILS_GET_ENV_ONE;
  }

  RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
    "Warning: unexpected value [%s] specified for %s. "
    "Valid values are 0 (%s) or 1 (%s).",
    env_var_value, name, zero_semantic, one_semantic);

  return RCUTILS_GET_ENV_ERROR;
}

rcutils_ret_t rcutils_logging_initialize_with_allocator(rcutils_allocator_t allocator)
{
  rcutils_ret_t ret = RCUTILS_RET_OK;
  if (!g_rcutils_logging_initialized) {
    if (!rcutils_allocator_is_valid(&allocator)) {
      RCUTILS_SET_ERROR_MSG("Provided allocator is invalid.");
      return RCUTILS_RET_INVALID_ARGUMENT;
    }
    g_rcutils_logging_allocator = allocator;

    g_rcutils_logging_output_handler = &rcutils_logging_console_output_handler;
    g_rcutils_logging_default_logger_level = RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL;

    const char * line_buffered = NULL;
    const char * ret_str = rcutils_get_env("RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED", &line_buffered);
    if (NULL == ret_str) {
      if (strcmp(line_buffered, "") != 0) {
        RCUTILS_SAFE_FWRITE_TO_STDERR(
          "RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED is now ignored. "
          "Please set RCUTILS_LOGGING_USE_STDOUT and RCUTILS_LOGGING_BUFFERED_STREAM "
          "to control the stream and the buffering of log messages.\n");
      }
    } else {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Error getting environment variable RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED: %s", ret_str);
      return RCUTILS_RET_ERROR;
    }

    // Set the default output stream for all severities to stderr so that errors
    // are propagated immediately.
    // The user can choose to set the output stream to stdout by setting the
    // RCUTILS_LOGGING_USE_STDOUT environment variable to 1.
    enum rcutils_get_env_retval retval = rcutils_get_env_var_zero_or_one(
      "RCUTILS_LOGGING_USE_STDOUT", "use stderr", "use stdout");
    switch (retval) {
      case RCUTILS_GET_ENV_ERROR:
        return RCUTILS_RET_INVALID_ARGUMENT;
      case RCUTILS_GET_ENV_EMPTY:
      case RCUTILS_GET_ENV_ZERO:
        g_output_stream = stderr;
        break;
      case RCUTILS_GET_ENV_ONE:
        g_output_stream = stdout;
        break;
      default:
        RCUTILS_SET_ERROR_MSG(
          "Invalid return from environment fetch");
        return RCUTILS_RET_ERROR;
    }

    // Allow the user to choose how buffering on the stream works by setting
    // RCUTILS_LOGGING_BUFFERED_STREAM.
    // With an empty environment variable, use the default of the stream.
    // With a value of 0, force the stream to be unbuffered.
    // With a value of 1, force the stream to be line buffered.
    retval = rcutils_get_env_var_zero_or_one(
      "RCUTILS_LOGGING_BUFFERED_STREAM", "not buffered", "buffered");
    if (RCUTILS_GET_ENV_ERROR == retval) {
      return RCUTILS_RET_INVALID_ARGUMENT;
    }
    if (RCUTILS_GET_ENV_ZERO == retval || RCUTILS_GET_ENV_ONE == retval) {
      int mode = retval == RCUTILS_GET_ENV_ZERO ? _IONBF : _IOLBF;
      size_t buffer_size = (mode == _IOLBF) ? RCUTILS_LOGGING_STREAM_BUFFER_SIZE : 0;

      // buffer_size cannot be 0 on Windows with IOLBF, see comments above where it's #define'd
      if (setvbuf(g_output_stream, NULL, mode, buffer_size) != 0) {
        char error_string[1024];
        rcutils_strerror(error_string, sizeof(error_string));
        RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Error setting stream buffering mode: %s", error_string);
        return RCUTILS_RET_ERROR;
      }
    } else if (RCUTILS_GET_ENV_EMPTY != retval) {
      RCUTILS_SET_ERROR_MSG(
        "Invalid return from environment fetch");
      return RCUTILS_RET_ERROR;
    }

    retval = rcutils_get_env_var_zero_or_one(
      "RCUTILS_COLORIZED_OUTPUT", "force color",
      "force no color");
    switch (retval) {
      case RCUTILS_GET_ENV_ERROR:
        return RCUTILS_RET_INVALID_ARGUMENT;
      case RCUTILS_GET_ENV_EMPTY:
        g_colorized_output = RCUTILS_COLORIZED_OUTPUT_AUTO;
        break;
      case RCUTILS_GET_ENV_ZERO:
        g_colorized_output = RCUTILS_COLORIZED_OUTPUT_FORCE_DISABLE;
        break;
      case RCUTILS_GET_ENV_ONE:
        g_colorized_output = RCUTILS_COLORIZED_OUTPUT_FORCE_ENABLE;
        break;
      default:
        RCUTILS_SET_ERROR_MSG(
          "Invalid return from environment fetch");
        return RCUTILS_RET_ERROR;
    }

    // Check for the environment variable for custom output formatting
    const char * output_format;
    ret_str = rcutils_get_env("RCUTILS_CONSOLE_OUTPUT_FORMAT", &output_format);
    if (NULL == ret_str && strcmp(output_format, "") != 0) {
      size_t chars_to_copy = strlen(output_format);
      if (chars_to_copy > RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN - 1) {
        chars_to_copy = RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN - 1;
      }
      memcpy(g_rcutils_logging_output_format_string, output_format, chars_to_copy);
      g_rcutils_logging_output_format_string[chars_to_copy] = '\0';
    } else {
      if (NULL != ret_str) {
        RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Failed to get output format from env. variable [%s]. Using default output format.",
          ret_str);
        ret = RCUTILS_RET_INVALID_ARGUMENT;
      }
      memcpy(
        g_rcutils_logging_output_format_string, g_rcutils_logging_default_output_format,
        strlen(g_rcutils_logging_default_output_format) + 1);
    }

    g_rcutils_logging_severities_map = rcutils_get_zero_initialized_string_map();
    rcutils_ret_t string_map_ret = rcutils_string_map_init(
      &g_rcutils_logging_severities_map, 0, g_rcutils_logging_allocator);
    if (string_map_ret != RCUTILS_RET_OK) {
      // If an error message was set it will have been overwritten by rcutils_string_map_init.
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Failed to initialize map for logger severities [%s]. Severities will not be configurable.",
        rcutils_get_error_string().str);
      g_rcutils_logging_severities_map_valid = false;
      ret = RCUTILS_RET_STRING_MAP_INVALID;
    } else {
      g_rcutils_logging_severities_map_valid = true;
    }

    g_rcutils_logging_initialized = true;
  }
  return ret;
}

rcutils_ret_t rcutils_logging_shutdown(void)
{
  if (!g_rcutils_logging_initialized) {
    return RCUTILS_RET_OK;
  }
  rcutils_ret_t ret = RCUTILS_RET_OK;
  if (g_rcutils_logging_severities_map_valid) {
    rcutils_ret_t string_map_ret = rcutils_string_map_fini(&g_rcutils_logging_severities_map);
    if (string_map_ret != RCUTILS_RET_OK) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Failed to finalize map for logger severities: %s",
        rcutils_get_error_string().str);
      ret = RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID;
    }
    g_rcutils_logging_severities_map_valid = false;
  }
  g_rcutils_logging_initialized = false;
  return ret;
}

rcutils_ret_t
rcutils_logging_severity_level_from_string(
  const char * severity_string, rcutils_allocator_t allocator, int * severity)
{
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(severity_string, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(severity, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret = RCUTILS_RET_LOGGING_SEVERITY_STRING_INVALID;

  // Convert the input string to upper case (for case insensitivity).
  char * severity_string_upper = rcutils_strdup(severity_string, allocator);
  if (NULL == severity_string_upper) {
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for uppercase string");
    return RCUTILS_RET_BAD_ALLOC;
  }
  for (int i = 0; severity_string_upper[i]; ++i) {
    severity_string_upper[i] = (char)toupper(severity_string_upper[i]);
  }

  // Determine the severity value matching the severity name.
  for (size_t i = 0;
    i < sizeof(g_rcutils_log_severity_names) / sizeof(g_rcutils_log_severity_names[0]);
    ++i)
  {
    const char * severity_string_i = g_rcutils_log_severity_names[i];
    if (severity_string_i && strcmp(severity_string_i, severity_string_upper) == 0) {
      *severity = (enum RCUTILS_LOG_SEVERITY)i;
      ret = RCUTILS_RET_OK;
      break;
    }
  }
  allocator.deallocate(severity_string_upper, allocator.state);
  return ret;
}

rcutils_logging_output_handler_t rcutils_logging_get_output_handler(void)
{
  RCUTILS_LOGGING_AUTOINIT;
  return g_rcutils_logging_output_handler;
}

void rcutils_logging_set_output_handler(rcutils_logging_output_handler_t function)
{
  RCUTILS_LOGGING_AUTOINIT;
  g_rcutils_logging_output_handler = function;
}

int rcutils_logging_get_default_logger_level(void)
{
  RCUTILS_LOGGING_AUTOINIT;
  return g_rcutils_logging_default_logger_level;
}

void rcutils_logging_set_default_logger_level(int level)
{
  RCUTILS_LOGGING_AUTOINIT;
  if (RCUTILS_LOG_SEVERITY_UNSET == level) {
    // Restore the default
    level = RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL;
  }
  g_rcutils_logging_default_logger_level = level;
}

int rcutils_logging_get_logger_level(const char * name)
{
  RCUTILS_LOGGING_AUTOINIT;
  if (NULL == name) {
    return -1;
  }
  return rcutils_logging_get_logger_leveln(name, strlen(name));
}

int rcutils_logging_get_logger_leveln(const char * name, size_t name_length)
{
  RCUTILS_LOGGING_AUTOINIT;
  if (NULL == name) {
    return -1;
  }

  // Skip the map lookup if the default was requested,
  // as it can still be used even if the severity map is invalid.
  if (0 == name_length) {
    return g_rcutils_logging_default_logger_level;
  }
  if (!g_rcutils_logging_severities_map_valid) {
    return RCUTILS_LOG_SEVERITY_UNSET;
  }

  // TODO(dhood): replace string map with int map.
  const char * severity_string = rcutils_string_map_getn(
    &g_rcutils_logging_severities_map, name, name_length);
  if (NULL == severity_string) {
    if (rcutils_string_map_key_existsn(&g_rcutils_logging_severities_map, name, name_length)) {
      // The level has been specified but couldn't be retrieved.
      return -1;
    }
    return RCUTILS_LOG_SEVERITY_UNSET;
  }

  int severity;
  rcutils_ret_t ret = rcutils_logging_severity_level_from_string(
    severity_string, g_rcutils_logging_allocator, &severity);
  if (RCUTILS_RET_OK != ret) {
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
      "Logger has an invalid severity level: %s\n", severity_string);
    return -1;
  }
  return severity;
}

int rcutils_logging_get_logger_effective_level(const char * name)
{
  RCUTILS_LOGGING_AUTOINIT;
  if (NULL == name) {
    return -1;
  }
  size_t substring_length = strlen(name);
  while (true) {
    int severity = rcutils_logging_get_logger_leveln(name, substring_length);
    if (-1 == severity) {
      RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
        "Error getting effective level of logger '%s'\n", name);
      return -1;
    }
    if (severity != RCUTILS_LOG_SEVERITY_UNSET) {
      return severity;
    }
    // Determine the next ancestor's FQN by removing the child's name.
    size_t index_last_separator = rcutils_find_lastn(
      name, RCUTILS_LOGGING_SEPARATOR_CHAR, substring_length);
    if (SIZE_MAX == index_last_separator) {
      // There are no more separators in the substring.
      // The name we just checked was the last that we needed to, and it was unset.
      break;
    }
    // Shorten the substring to be the name of the ancestor (excluding the separator).
    substring_length = index_last_separator;
  }
  // Neither the logger nor its ancestors have had their level specified.
  return g_rcutils_logging_default_logger_level;
}

rcutils_ret_t rcutils_logging_set_logger_level(const char * name, int level)
{
  RCUTILS_LOGGING_AUTOINIT;
  if (NULL == name) {
    RCUTILS_SET_ERROR_MSG("Invalid logger name");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  if (strlen(name) == 0) {
    g_rcutils_logging_default_logger_level = level;
    return RCUTILS_RET_OK;
  }
  if (!g_rcutils_logging_severities_map_valid) {
    RCUTILS_SET_ERROR_MSG("Logger severity level map is invalid");
    return RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID;
  }

  // Convert the severity value into a string for storage.
  // TODO(dhood): replace string map with int map.
  if (level < 0 ||
    level >=
    (int)(sizeof(g_rcutils_log_severity_names) / sizeof(g_rcutils_log_severity_names[0])))
  {
    RCUTILS_SET_ERROR_MSG("Invalid severity level specified for logger");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  const char * severity_string = g_rcutils_log_severity_names[level];
  if (NULL == severity_string) {
    RCUTILS_SET_ERROR_MSG("Unable to determine severity_string for severity");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  rcutils_ret_t string_map_ret = rcutils_string_map_set(
    &g_rcutils_logging_severities_map, name, severity_string);
  if (string_map_ret != RCUTILS_RET_OK) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error setting severity level for logger named '%s': %s",
      name, rcutils_get_error_string().str);
    return RCUTILS_RET_ERROR;
  }
  return RCUTILS_RET_OK;
}

bool rcutils_logging_logger_is_enabled_for(const char * name, int severity)
{
  RCUTILS_LOGGING_AUTOINIT;
  int logger_level = g_rcutils_logging_default_logger_level;
  if (name) {
    logger_level = rcutils_logging_get_logger_effective_level(name);
    if (-1 == logger_level) {
      RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
        "Error determining if logger '%s' is enabled for severity '%d'\n",
        name, severity);
      return false;
    }
  }
  return severity >= logger_level;
}
#define SAFE_FWRITE_TO_STDERR_AND(action) \
  RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str); \
  rcutils_reset_error(); \
  RCUTILS_SAFE_FWRITE_TO_STDERR("\n"); \
  action;

#define OK_OR_RETURN_NULL(op) \
  if (op != RCUTILS_RET_OK) { \
    SAFE_FWRITE_TO_STDERR_AND(return NULL); \
  }

#define OK_OR_RETURN_EARLY(op) \
  if (op != RCUTILS_RET_OK) { \
    return op; \
  }

#define APPEND_AND_RETURN_LOG_OUTPUT(s) \
  OK_OR_RETURN_NULL(rcutils_char_array_strcat(logging_output, s)); \
  return logging_output->buffer;


void rcutils_log(
  const rcutils_log_location_t * location,
  int severity, const char * name, const char * format, ...)
{
  if (!rcutils_logging_logger_is_enabled_for(name, severity)) {
    return;
  }
  rcutils_time_point_value_t now;
  rcutils_ret_t ret = rcutils_system_time_now(&now);
  if (ret != RCUTILS_RET_OK) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to get timestamp while doing a console logging.\n");
    return;
  }
  rcutils_logging_output_handler_t output_handler = g_rcutils_logging_output_handler;
  if (output_handler != NULL) {
    va_list args;
    va_start(args, format);
    (*output_handler)(location, severity, name ? name : "", now, format, &args);
    va_end(args);
  }
}

typedef struct logging_input
{
  const char * name;
  const rcutils_log_location_t * location;
  const char * msg;
  int severity;
  rcutils_time_point_value_t timestamp;
} logging_input;

typedef const char * (* token_handler)(
  const logging_input * logging_input,
  rcutils_char_array_t * logging_output);

typedef struct token_map_entry
{
  const char * token;
  token_handler handler;
} token_map_entry;

const char * expand_time(
  const logging_input * logging_input, rcutils_char_array_t * logging_output,
  rcutils_ret_t (* time_func)(const rcutils_time_point_value_t *, char *, size_t))
{
  // Temporary, local storage for integer/float conversion to string
  // Note:
  //   32 characters enough, because the most it can be is 20 characters
  //   for the 19 possible digits in a signed 64-bit number plus the optional
  //   decimal point in the floating point seconds version
  char numeric_storage[32];
  OK_OR_RETURN_NULL(time_func(&logging_input->timestamp, numeric_storage, sizeof(numeric_storage)));
  APPEND_AND_RETURN_LOG_OUTPUT(numeric_storage);
}

const char * expand_time_as_seconds(
  const logging_input * logging_input,
  rcutils_char_array_t * logging_output)
{
  return expand_time(logging_input, logging_output, rcutils_time_point_value_as_seconds_string);
}

const char * expand_time_as_nanoseconds(
  const logging_input * logging_input,
  rcutils_char_array_t * logging_output)
{
  return expand_time(logging_input, logging_output, rcutils_time_point_value_as_nanoseconds_string);
}

const char * expand_line_number(
  const logging_input * logging_input,
  rcutils_char_array_t * logging_output)
{
  // Allow 9 digits for the expansion of the line number (otherwise, truncate).
  char line_number_expansion[10];

  const rcutils_log_location_t * location = logging_input->location;

  if (!location) {
    OK_OR_RETURN_NULL(rcutils_char_array_strcpy(logging_output, "0"));
    return logging_output->buffer;
  }

  // Even in the case of truncation the result will still be null-terminated.
  int written = rcutils_snprintf(
    line_number_expansion, sizeof(line_number_expansion), "%zu", location->line_number);
  if (written < 0) {
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
      "failed to format line number: '%zu'\n", location->line_number);
    return NULL;
  }

  APPEND_AND_RETURN_LOG_OUTPUT(line_number_expansion);
}

const char * expand_severity(
  const logging_input * logging_input,
  rcutils_char_array_t * logging_output)
{
  const char * severity_string = g_rcutils_log_severity_names[logging_input->severity];
  APPEND_AND_RETURN_LOG_OUTPUT(severity_string);
}

const char * expand_name(const logging_input * logging_input, rcutils_char_array_t * logging_output)
{
  if (NULL != logging_input->name) {
    APPEND_AND_RETURN_LOG_OUTPUT(logging_input->name);
  }
  return logging_output->buffer;
}

const char * expand_message(
  const logging_input * logging_input,
  rcutils_char_array_t * logging_output)
{
  OK_OR_RETURN_NULL(rcutils_char_array_strcat(logging_output, logging_input->msg));
  return logging_output->buffer;
}

const char * expand_function_name(
  const logging_input * logging_input,
  rcutils_char_array_t * logging_output)
{
  if (logging_input->location) {
    APPEND_AND_RETURN_LOG_OUTPUT(logging_input->location->function_name);
  }
  return logging_output->buffer;
}

const char * expand_file_name(
  const logging_input * logging_input,
  rcutils_char_array_t * logging_output)
{
  if (logging_input->location) {
    APPEND_AND_RETURN_LOG_OUTPUT(logging_input->location->file_name);
  }
  return logging_output->buffer;
}

static const token_map_entry tokens[] = {
  {.token = "severity", .handler = expand_severity},
  {.token = "name", .handler = expand_name},
  {.token = "message", .handler = expand_message},
  {.token = "function_name", .handler = expand_function_name},
  {.token = "file_name", .handler = expand_file_name},
  {.token = "time", .handler = expand_time_as_seconds},
  {.token = "time_as_nanoseconds", .handler = expand_time_as_nanoseconds},
  {.token = "line_number", .handler = expand_line_number},
};

token_handler find_token_handler(const char * token)
{
  int token_number = sizeof(tokens) / sizeof(tokens[0]);
  for (int token_index = 0; token_index < token_number; token_index++) {
    if (strcmp(token, tokens[token_index].token) == 0) {
      return tokens[token_index].handler;
    }
  }
  return NULL;
}

rcutils_ret_t rcutils_logging_format_message(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * msg, rcutils_char_array_t * logging_output)
{
  rcutils_ret_t status = RCUTILS_RET_OK;
  // Process the format string looking for known tokens.
  const char token_start_delimiter = '{';
  const char token_end_delimiter = '}';

  const char * str = g_rcutils_logging_output_format_string;
  size_t size = strlen(g_rcutils_logging_output_format_string);

  const logging_input logging_input = {
    .location = location,
    .severity = severity,
    .name = name,
    .timestamp = timestamp,
    .msg = msg
  };

  // Walk through the format string and expand tokens when they're encountered.
  size_t i = 0;
  while (i < size) {
    // Print everything up to the next token start delimiter.
    size_t chars_to_start_delim = rcutils_find(str + i, token_start_delimiter);
    size_t remaining_chars = size - i;

    if (chars_to_start_delim > 0) {  // there are stuff before a token start delimiter
      size_t chars_to_copy = chars_to_start_delim >
        remaining_chars ? remaining_chars : chars_to_start_delim;
      status = rcutils_char_array_strncat(logging_output, str + i, chars_to_copy);
      OK_OR_RETURN_EARLY(status);
      i += chars_to_copy;
      if (i >= size) {  // perhaps no start delimiter was found
        break;
      }
    }

    // We are at a token start delimiter: determine if there's a known token or not.
    // Potential tokens can't possibly be longer than the format string itself.
    char token[RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN];

    // Look for a token end delimiter.
    size_t chars_to_end_delim = rcutils_find(str + i, token_end_delimiter);
    remaining_chars = size - i;

    if (chars_to_end_delim > remaining_chars) {
      // No end delimiters found in the remainder of the format string;
      // there won't be any more tokens so shortcut the rest of the checking.
      status = rcutils_char_array_strncat(logging_output, str + i, remaining_chars);
      OK_OR_RETURN_EARLY(status);
      break;
    }

    // Found what looks like a token; determine if it's recognized.
    size_t token_len = chars_to_end_delim - 1;  // Not including delimiters.
    memcpy(token, str + i + 1, token_len);  // Skip the start delimiter.
    token[token_len] = '\0';

    token_handler expand_token = find_token_handler(token);

    if (!expand_token) {
      // This wasn't a token; print the start delimiter and continue the search as usual
      // (the substring might contain more start delimiters).
      status = rcutils_char_array_strncat(logging_output, str + i, 1);
      OK_OR_RETURN_EARLY(status);
      i++;
      continue;
    }

    if (!expand_token(&logging_input, logging_output)) {
      return RCUTILS_RET_ERROR;
    }
    // Skip ahead to avoid re-processing the token characters (including the 2 delimiters).
    i += token_len + 2;
  }

  return status;
}

#ifdef _WIN32
# define COLOR_NORMAL 7
# define COLOR_RED 4
# define COLOR_GREEN 2
# define COLOR_YELLOW 6
# define IS_STREAM_A_TTY(stream) (_isatty(_fileno(stream)) != 0)
#else
# define COLOR_NORMAL "\033[0m"
# define COLOR_RED "\033[31m"
# define COLOR_GREEN "\033[32m"
# define COLOR_YELLOW "\033[33m"
# define IS_STREAM_A_TTY(stream) (isatty(fileno(stream)) != 0)
#endif

#define IS_OUTPUT_COLORIZED(is_colorized) \
  { \
    if (g_colorized_output == RCUTILS_COLORIZED_OUTPUT_FORCE_ENABLE) { \
      is_colorized = true; \
    } else if (g_colorized_output == RCUTILS_COLORIZED_OUTPUT_FORCE_DISABLE) { \
      is_colorized = false; \
    } else { \
      is_colorized = IS_STREAM_A_TTY(g_output_stream); \
    } \
  }
#define SET_COLOR_WITH_SEVERITY(status, severity, color) \
  { \
    switch (severity) { \
      case RCUTILS_LOG_SEVERITY_DEBUG: \
        color = COLOR_GREEN; \
        break; \
      case RCUTILS_LOG_SEVERITY_INFO: \
        color = COLOR_NORMAL; \
        break; \
      case RCUTILS_LOG_SEVERITY_WARN: \
        color = COLOR_YELLOW; \
        break; \
      case RCUTILS_LOG_SEVERITY_ERROR: \
      case RCUTILS_LOG_SEVERITY_FATAL: \
        color = COLOR_RED; \
        break; \
      default: \
        RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING( \
          "unknown severity level: %d\n", severity); \
        status = RCUTILS_RET_INVALID_ARGUMENT; \
    } \
  }
#ifdef _WIN32
# define SET_OUTPUT_COLOR_WITH_COLOR(status, color, handle) \
  { \
    if (RCUTILS_RET_OK == status) { \
      if (!SetConsoleTextAttribute(handle, color)) { \
        DWORD error = GetLastError(); \
        RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING( \
          "SetConsoleTextAttribute failed with error code %lu.\n", error); \
        status = RCUTILS_RET_ERROR; \
      } \
    } \
  }
# define GET_HANDLE_FROM_STREAM(status, handle) \
  { \
    if (RCUTILS_RET_OK == status) { \
      if (g_output_stream == stdout) { \
        handle = GetStdHandle(STD_OUTPUT_HANDLE); \
      } else { \
        handle = GetStdHandle(STD_ERROR_HANDLE); \
      } \
      if (INVALID_HANDLE_VALUE == handle) { \
        DWORD error = GetLastError(); \
        RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING( \
          "GetStdHandle failed with error code %lu.\n", error); \
        status = RCUTILS_RET_ERROR; \
      } \
    } \
  }
# define SET_OUTPUT_COLOR_WITH_SEVERITY(status, severity, output_array) \
  { \
    WORD color = COLOR_NORMAL; \
    HANDLE handle = INVALID_HANDLE_VALUE; \
    SET_COLOR_WITH_SEVERITY(status, severity, color) \
    GET_HANDLE_FROM_STREAM(status, handle) \
    SET_OUTPUT_COLOR_WITH_COLOR(status, color, handle) \
  }
# define SET_STANDARD_COLOR_IN_STREAM(is_colorized, status) \
  { \
    if (is_colorized) { \
      HANDLE handle = INVALID_HANDLE_VALUE; \
      GET_HANDLE_FROM_STREAM(status, handle) \
      SET_OUTPUT_COLOR_WITH_COLOR(status, COLOR_NORMAL, handle) \
    } \
  }
# define SET_STANDARD_COLOR_IN_BUFFER(is_colorized, status, output_array)
#else
# define SET_OUTPUT_COLOR_WITH_COLOR(status, color, output_array) \
  { \
    if (RCUTILS_RET_OK == status) { \
      status = rcutils_char_array_strncat(&output_array, color, strlen(color)); \
      if (RCUTILS_RET_OK != status) { \
        RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING( \
          "Error: rcutils_char_array_strncat failed with: %d\n", \
          status); \
      } \
    } \
  }
# define SET_OUTPUT_COLOR_WITH_SEVERITY(status, severity, output_array) \
  { \
    const char * color = NULL; \
    SET_COLOR_WITH_SEVERITY(status, severity, color) \
    SET_OUTPUT_COLOR_WITH_COLOR(status, color, output_array) \
  }
# define SET_STANDARD_COLOR_IN_BUFFER(is_colorized, status, output_array) \
  { \
    if (is_colorized) { \
      SET_OUTPUT_COLOR_WITH_COLOR(status, COLOR_NORMAL, output_array) \
    } \
  }
# define SET_STANDARD_COLOR_IN_STREAM(is_colorized, status)
#endif

void rcutils_logging_console_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * format, va_list * args)
{
  rcutils_ret_t status = RCUTILS_RET_OK;
  bool is_colorized = false;

  if (!g_rcutils_logging_initialized) {
    RCUTILS_SAFE_FWRITE_TO_STDERR(
      "logging system isn't initialized: "
      "call to rcutils_logging_console_output_handler failed.\n");
    return;
  }
  switch (severity) {
    case RCUTILS_LOG_SEVERITY_DEBUG:
    case RCUTILS_LOG_SEVERITY_INFO:
    case RCUTILS_LOG_SEVERITY_WARN:
    case RCUTILS_LOG_SEVERITY_ERROR:
    case RCUTILS_LOG_SEVERITY_FATAL:
      break;
    default:
      RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
        "unknown severity level: %d\n", severity);
      return;
  }

  IS_OUTPUT_COLORIZED(is_colorized)

  char msg_buf[1024] = "";
  rcutils_char_array_t msg_array = {
    .buffer = msg_buf,
    .owns_buffer = false,
    .buffer_length = 0u,
    .buffer_capacity = sizeof(msg_buf),
    .allocator = g_rcutils_logging_allocator
  };

  char output_buf[1024] = "";
  rcutils_char_array_t output_array = {
    .buffer = output_buf,
    .owns_buffer = false,
    .buffer_length = 0u,
    .buffer_capacity = sizeof(output_buf),
    .allocator = g_rcutils_logging_allocator
  };

  if (is_colorized) {
    SET_OUTPUT_COLOR_WITH_SEVERITY(status, severity, output_array)
  }

  if (RCUTILS_RET_OK == status) {
    status = rcutils_char_array_vsprintf(&msg_array, format, *args);
    if (RCUTILS_RET_OK != status) {
      RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
        "Error: rcutils_char_array_vsprintf failed with: %d\n", status);
    }
  }

  if (RCUTILS_RET_OK == status) {
    status = rcutils_logging_format_message(
      location, severity, name, timestamp, msg_array.buffer, &output_array);
    if (RCUTILS_RET_OK != status) {
      RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
        "Error: rcutils_logging_format_message failed with: %d\n", status);
    }
  }

  // Does nothing in windows
  SET_STANDARD_COLOR_IN_BUFFER(is_colorized, status, output_array)

  if (RCUTILS_RET_OK == status) {
    fprintf(g_output_stream, "%s\n", output_array.buffer);
  }

  // Only does something in windows
  // cppcheck-suppress uninitvar  // suppress cppcheck false positive
  SET_STANDARD_COLOR_IN_STREAM(is_colorized, status)

  status = rcutils_char_array_fini(&msg_array);
  if (RCUTILS_RET_OK != status) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to fini array.\n");
  }
  status = rcutils_char_array_fini(&output_array);
  if (RCUTILS_RET_OK != status) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to fini array.\n");
  }
}

#ifdef __cplusplus
}
#endif
