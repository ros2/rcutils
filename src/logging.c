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
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/find.h"
#include "rcutils/format_string.h"
#include "rcutils/get_env.h"
#include "rcutils/logging.h"
#include "rcutils/snprintf.h"
#include "rcutils/strdup.h"
#include "rcutils/time.h"
#include "rcutils/types/string_map.h"

#define RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN 2048

const char * g_rcutils_log_severity_names[] = {
  [RCUTILS_LOG_SEVERITY_UNSET] = "UNSET",
  [RCUTILS_LOG_SEVERITY_DEBUG] = "DEBUG",
  [RCUTILS_LOG_SEVERITY_INFO] = "INFO",
  [RCUTILS_LOG_SEVERITY_WARN] = "WARN",
  [RCUTILS_LOG_SEVERITY_ERROR] = "ERROR",
  [RCUTILS_LOG_SEVERITY_FATAL] = "FATAL",
};

bool g_rcutils_logging_initialized = false;

char g_rcutils_logging_output_format_string[RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN];
static const char * g_rcutils_logging_default_output_format = "[{severity}] [{name}]: {message}";

static rcutils_allocator_t g_rcutils_logging_allocator;

rcutils_logging_output_handler_t g_rcutils_logging_output_handler = NULL;
static rcutils_string_map_t g_rcutils_logging_severities_map;

// If this is false, attempts to use the severities map will be skipped.
// This can happen if allocation of the map fails at initialization.
bool g_rcutils_logging_severities_map_valid = false;

int g_rcutils_logging_default_logger_level = 0;

bool g_force_stdout_line_buffered = false;
bool g_stdout_flush_failure_reported = false;


rcutils_ret_t rcutils_logging_initialize(void)
{
  const char * line_buffered;
  const char * ret_str = rcutils_get_env("RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED", &line_buffered);

  if (NULL == ret_str) {
    if (strcmp(line_buffered, "1")) {
      g_force_stdout_line_buffered = true;
    } else {
      if (!strcmp(line_buffered, "0")) {
        fprintf(stderr,
          "Warning: unexpected value [%s] specified for RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED. "
          "Default value 0 will be used. Valid values are 1 or 0.\n",
          line_buffered);
      }
    }
  } else {
    fprintf(stderr, "Error getting env. variable "
      "RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED: %s\n", ret_str);
  }
  return rcutils_logging_initialize_with_allocator(rcutils_get_default_allocator());
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

    // Check for the environment variable for custom output formatting
    const char * output_format;
    const char * ret_str = rcutils_get_env("RCUTILS_CONSOLE_OUTPUT_FORMAT", &output_format);
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
      memcpy(g_rcutils_logging_output_format_string, g_rcutils_logging_default_output_format,
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
    severity_string_upper[i] = toupper(severity_string_upper[i]);
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
  RCUTILS_LOGGING_AUTOINIT
  return g_rcutils_logging_output_handler;
}

void rcutils_logging_set_output_handler(rcutils_logging_output_handler_t function)
{
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  RCUTILS_LOGGING_AUTOINIT
  g_rcutils_logging_output_handler = function;
  // *INDENT-ON*
}

int rcutils_logging_get_default_logger_level(void)
{
  RCUTILS_LOGGING_AUTOINIT
  return g_rcutils_logging_default_logger_level;
}

void rcutils_logging_set_default_logger_level(int level)
{
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  RCUTILS_LOGGING_AUTOINIT
  if (RCUTILS_LOG_SEVERITY_UNSET == level) {
    // Restore the default
    level = RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL;
  }
  g_rcutils_logging_default_logger_level = level;
  // *INDENT-ON*
}

int rcutils_logging_get_logger_level(const char * name)
{
  RCUTILS_LOGGING_AUTOINIT
  return rcutils_logging_get_logger_leveln(name, strlen(name));
}

int rcutils_logging_get_logger_leveln(const char * name, size_t name_length)
{
  RCUTILS_LOGGING_AUTOINIT
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
    fprintf(
      stderr,
      "Logger has an invalid severity level: %s\n", severity_string);
    return -1;
  }
  return severity;
}

int rcutils_logging_get_logger_effective_level(const char * name)
{
  RCUTILS_LOGGING_AUTOINIT
  if (NULL == name) {
    return -1;
  }
  size_t substring_length = strlen(name);
  while (true) {
    int severity = rcutils_logging_get_logger_leveln(name, substring_length);
    if (-1 == severity) {
      fprintf(
        stderr,
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
  RCUTILS_LOGGING_AUTOINIT
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
  RCUTILS_LOGGING_AUTOINIT
  int logger_level = g_rcutils_logging_default_logger_level;
  if (name) {
    logger_level = rcutils_logging_get_logger_effective_level(name);
    if (-1 == logger_level) {
      fprintf(
        stderr,
        "Error determining if logger '%s' is enabled for severity '%d'\n", name, severity);
      return false;
    }
  }
  return severity >= logger_level;
}

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

/// Ensure that the logging buffer is large enough.
/**
 * Macro for (re)allocating a dynamic buffer if the ouput buffer doesn't have
 * enough space for an additional n characters and null terminator.
 * Whether to allocate or re-allocate is determined by if the output_buffer
 * points to the original static_output_buffer or not.
 * If a new buffer is allocated it will be null-terminated.
 *
 * /param n Number of characters requiring space (not including null terminator)
 * /param output_buffer_size Size allocated for the output buffer
 * /param output_buffer The output buffer to ensure has enough space
 * /param static_output_buffer The static buffer initially used as the output
 */
#define RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER( \
    n, \
    output_buffer_size, \
    output_buffer, \
    static_output_buffer \
) \
  size_t old_output_buffer_len = strlen(output_buffer); \
  size_t required_output_buffer_size = old_output_buffer_len + n + 1; \
  /* ensure that required size calculation did not overflow */ \
  if (required_output_buffer_size <= old_output_buffer_len) { \
    fprintf(stderr, "required buffer for logging output too large\n"); \
    goto cleanup; \
  } \
  if (required_output_buffer_size > output_buffer_size) { \
    do { \
      if (output_buffer_size <= (SIZE_MAX / 2)) { \
        output_buffer_size *= 2; \
      } else { \
        output_buffer_size = SIZE_MAX; \
      } \
    } while (required_output_buffer_size > output_buffer_size); \
    if (output_buffer == static_output_buffer) { \
      void * dynamic_output_buffer = g_rcutils_logging_allocator.allocate( \
        output_buffer_size, g_rcutils_logging_allocator.state); \
      if (NULL == dynamic_output_buffer) { \
        fprintf(stderr, "failed to allocate buffer for logging output\n"); \
        goto cleanup; \
      } \
      memcpy(dynamic_output_buffer, output_buffer, old_output_buffer_len); \
      output_buffer = (char *)dynamic_output_buffer; \
    } else { \
      void * new_dynamic_output_buffer = g_rcutils_logging_allocator.reallocate( \
        output_buffer, output_buffer_size, g_rcutils_logging_allocator.state); \
      if (NULL == new_dynamic_output_buffer) { \
        fprintf(stderr, "failed to reallocate buffer for logging output\n"); \
        goto cleanup; \
      } \
      output_buffer = (char *)new_dynamic_output_buffer; \
    } \
    output_buffer[old_output_buffer_len] = '\0'; \
  }

void rcutils_logging_console_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * format, va_list * args)
{
  if (!g_rcutils_logging_initialized) {
    fprintf(
      stderr,
      "logging system isn't initialized: " \
      "call to rcutils_logging_console_output_handler failed.\n");
    return;
  }
  FILE * stream = NULL;
  const char * severity_string = "";
  switch (severity) {
    case RCUTILS_LOG_SEVERITY_DEBUG:
      stream = stdout;
      break;
    case RCUTILS_LOG_SEVERITY_INFO:
      stream = stdout;
      break;
    case RCUTILS_LOG_SEVERITY_WARN:
      stream = stderr;
      break;
    case RCUTILS_LOG_SEVERITY_ERROR:
      stream = stderr;
      break;
    case RCUTILS_LOG_SEVERITY_FATAL:
      stream = stderr;
      break;
    default:
      fprintf(stderr, "unknown severity level: %d\n", severity);
      return;
  }
  severity_string = g_rcutils_log_severity_names[severity];
  if (NULL == severity_string) {
    fprintf(stderr, "couldn't determine name for severity level: %d\n", severity);
    return;
  }

  // Declare variables that will be needed for cleanup ahead of time.
  char static_output_buffer[1024];
  char * output_buffer = NULL;

  // Start with a fixed size message buffer and if during message formatting we need longer, we'll
  // dynamically allocate space.
  char static_message_buffer[1024];
  char * message_buffer = static_message_buffer;

  int written;
  {
    // use copy of args to keep args for potential later user
    va_list args_clone;
    va_copy(args_clone, *args);
    written = vsnprintf(static_message_buffer, sizeof(static_message_buffer), format, args_clone);
    va_end(args_clone);
  }
  if (written < 0) {
    fprintf(stderr, "failed to format message: '%s'\n", format);
    return;
  }
  if ((size_t)written >= sizeof(static_message_buffer)) {
    // write was incomplete, allocate necessary memory dynamically
    size_t message_buffer_size = written + 1;
    void * dynamic_message_buffer = g_rcutils_logging_allocator.allocate(
      message_buffer_size, g_rcutils_logging_allocator.state);
    if (NULL == dynamic_message_buffer) {
      fprintf(stderr, "failed to allocate buffer for message\n");
      return;
    }
    written = vsnprintf(dynamic_message_buffer, message_buffer_size, format, *args);
    message_buffer = (char *)dynamic_message_buffer;
    if (written < 0 || (size_t)written >= message_buffer_size) {
      fprintf(
        stderr,
        "failed to format message (using dynamically allocated memory): '%s'\n",
        format);
      goto cleanup;
    }
  }

  // Process the format string looking for known tokens.
  const char token_start_delimiter = '{';
  const char token_end_delimiter = '}';
  // Start with a fixed size output buffer and if during token expansion we need longer, we'll
  // dynamically allocate space.
  output_buffer = static_output_buffer;
  output_buffer[0] = '\0';
  size_t output_buffer_size = sizeof(static_output_buffer);
  const char * str = g_rcutils_logging_output_format_string;
  size_t size = strlen(g_rcutils_logging_output_format_string);

  // Walk through the format string and expand tokens when they're encountered.
  size_t i = 0;
  while (i < size) {
    // Print everything up to the next token start delimiter.
    size_t chars_to_start_delim = rcutils_find(str + i, token_start_delimiter);
    size_t remaining_chars = size - i;
    if (chars_to_start_delim > 0) {
      if (chars_to_start_delim > remaining_chars) {
        // No start delimiters found; don't allow printing more of than what's left if.
        chars_to_start_delim = remaining_chars;
      }
      RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER(
        chars_to_start_delim, output_buffer_size, output_buffer, static_output_buffer)
      memcpy(output_buffer + old_output_buffer_len, str + i, chars_to_start_delim);
      output_buffer[old_output_buffer_len + chars_to_start_delim] = '\0';
      i += chars_to_start_delim;
      if (i >= size) {
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
      RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER(
        remaining_chars, output_buffer_size, output_buffer, static_output_buffer)
      memcpy(output_buffer + old_output_buffer_len, str + i, remaining_chars + 1);
      break;
    }
    // Found what looks like a token; determine if it's recognized.
    size_t token_len = chars_to_end_delim - 1;  // Not including delimiters.
    memcpy(token, str + i + 1, token_len);  // Skip the start delimiter.
    token[token_len] = '\0';
    // Expand known tokens into their content strings.
    // The resulting token_expansion string must always be null-terminated.
    const char * token_expansion = NULL;
    // Temporary, local storage for integer/float conversion to string
    // Note:
    //   32 characters enough, because the most it can be is 20 characters
    //   for the 19 possible digits in a signed 64-bit number plus the optional
    //   decimal point in the floating point seconds version
    char numeric_storage[32];
    // Allow 9 digits for the expansion of the line number (otherwise, truncate).
    char line_number_expansion[10];
    if (strcmp("severity", token) == 0) {
      token_expansion = severity_string;
    } else if (strcmp("name", token) == 0) {
      token_expansion = name;
    } else if (strcmp("message", token) == 0) {
      token_expansion = message_buffer;
    } else if (strcmp("time", token) == 0) {
      rcutils_ret_t ret = rcutils_time_point_value_as_seconds_string(
        &timestamp,
        numeric_storage, sizeof(numeric_storage));
      if (ret != RCUTILS_RET_OK) {
        RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
        rcutils_reset_error();
        RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
        goto cleanup;
      }
      token_expansion = numeric_storage;
    } else if (strcmp("time_as_nanoseconds", token) == 0) {
      rcutils_ret_t ret = rcutils_time_point_value_as_nanoseconds_string(
        &timestamp,
        numeric_storage, sizeof(numeric_storage));
      if (ret != RCUTILS_RET_OK) {
        RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
        rcutils_reset_error();
        RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
        goto cleanup;
      }
      token_expansion = numeric_storage;
    } else if (strcmp("function_name", token) == 0) {
      token_expansion = location ? location->function_name : "\"\"";
    } else if (strcmp("file_name", token) == 0) {
      token_expansion = location ? location->file_name : "\"\"";
    } else if (strcmp("line_number", token) == 0) {
      if (location) {
        // Even in the case of truncation the result will still be null-terminated.
        written = rcutils_snprintf(
          line_number_expansion, sizeof(line_number_expansion), "%zu", location->line_number);
        if (written < 0) {
          fprintf(
            stderr,
            "failed to format line number: '%zu'\n",
            location->line_number);
          goto cleanup;
        }
        token_expansion = line_number_expansion;
      } else {
        token_expansion = "0";
      }
    } else {
      // This wasn't a token; print the start delimiter and continue the search as usual
      // (the substring might contain more start delimiters).
      RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER(
        1, output_buffer_size, output_buffer, static_output_buffer)
      memcpy(output_buffer + old_output_buffer_len, str + i, 1);
      output_buffer[old_output_buffer_len + 1] = '\0';
      i++;
      continue;
    }
    size_t n = strlen(token_expansion);
    RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER(
      n, output_buffer_size, output_buffer, static_output_buffer)
    memcpy(output_buffer + old_output_buffer_len, token_expansion, n + 1);
    // Skip ahead to avoid re-processing the token characters (including the 2 delimiters).
    i += token_len + 2;
  }
  fprintf(stream, "%s\n", output_buffer);

  if (g_force_stdout_line_buffered && stream == stdout) {
    int flush_result = fflush(stream);
    if (flush_result != 0 && !g_stdout_flush_failure_reported) {
      g_stdout_flush_failure_reported = true;
      fprintf(stderr, "Error: failed to perform fflush on stdout, fflush return code is: %d\n",
        flush_result);
    }
  }

cleanup:
  if (message_buffer && message_buffer != static_message_buffer) {
    g_rcutils_logging_allocator.deallocate(message_buffer, g_rcutils_logging_allocator.state);
  }

  if (output_buffer && output_buffer != static_output_buffer) {
    g_rcutils_logging_allocator.deallocate(output_buffer, g_rcutils_logging_allocator.state);
  }
}

#ifdef __cplusplus
}
#endif
