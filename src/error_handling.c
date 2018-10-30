// Copyright 2015 Open Source Robotics Foundation, Inc.
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

// Note: migrated from rmw/error_handling.c in 2017-04

#include <rcutils/error_handling.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rcutils/allocator.h>
#include <rcutils/macros.h>
#include <rcutils/strdup.h>

// RCUTILS_REPORT_ERROR_HANDLING_ERRORS and RCUTILS_WARN_ON_TRUNCATION are set in the header below
#include "./error_handling_helpers.h"

// g_ is to global variable, as gtls_ is to global thread-local storage variable
RCUTILS_THREAD_LOCAL bool gtls_rcutils_thread_local_initialized = false;
RCUTILS_THREAD_LOCAL rcutils_error_state_t gtls_rcutils_error_state;
RCUTILS_THREAD_LOCAL bool gtls_rcutils_error_string_is_formatted = false;
RCUTILS_THREAD_LOCAL rcutils_error_string_t gtls_rcutils_error_string;
RCUTILS_THREAD_LOCAL bool gtls_rcutils_error_is_set = false;

rcutils_ret_t
rcutils_initialize_error_handling_thread_local_storage(rcutils_allocator_t allocator)
{
  if (gtls_rcutils_thread_local_initialized) {
    return RCUTILS_RET_OK;
  }

  // check if the given allocator is valid
  if (!rcutils_allocator_is_valid(&allocator)) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    RCUTILS_SAFE_FWRITE_TO_STDERR(
      "[rcutils|error_handling.c:" RCUTILS_STRINGIFY(__LINE__)
      "] rcutils_initialize_error_handling_thread_local_storage() given invalid allocator\n");
#endif
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  // right now the allocator is not used for anything
  // but other future implementations may need to use it
  // e.g. pthread which could only provide thread-local pointers would need to
  // allocate memory to which those pointers would point

  // forcing the values back to their initial state should force the thread-local storage
  // to initialize and do any required memory allocation
  gtls_rcutils_thread_local_initialized = true;
  rcutils_reset_error();
  RCUTILS_SET_ERROR_MSG("no error - initializing thread-local storage")
  {  // this scope is to prevent uncrustify from moving the (void) to the previous line
    (void)rcutils_get_error_string();
  }
  rcutils_reset_error();

  // at this point the thread-local allocator, error state, and error string are all initialized
  return RCUTILS_RET_OK;
}

static
bool
__same_string(const char * str1, const char * str2, size_t count)
{
  return str1 == str2 || 0 == strncmp(str1, str2, count);
}

void
rcutils_set_error_state(
  const char * error_string,
  const char * file,
  size_t line_number)
{
  rcutils_error_state_t error_state;

  // The const is casted away to set the 'message' field of the error state (const for users only).
  __rcutils_copy_string(error_state.message, sizeof(error_state.message), error_string);
  __rcutils_copy_string(error_state.file, sizeof(error_state.file), file);
  error_state.line_number = line_number;
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
  // Only warn of overwritting if the new error is different from the old ones.
  size_t characters_to_compare = strnlen(error_string, RCUTILS_ERROR_MESSAGE_MAX_LENGTH);
  // assumption is that message length is <= max error string length
  static_assert(
    sizeof(gtls_rcutils_error_state.message) <= sizeof(gtls_rcutils_error_string.str),
    "expected error state's max message length to be less than or equal to error string max");
  if (
    gtls_rcutils_error_is_set &&
    __same_string(error_string, gtls_rcutils_error_string.str, characters_to_compare) &&
    __same_string(error_string, gtls_rcutils_error_state.message, characters_to_compare))
  {
    fprintf(
      stderr,
      "\n"
      ">>> [rcutils|error_handling.c:" RCUTILS_STRINGIFY(__LINE__) "] rcutils_set_error_state()\n"
      "This error state is being overwritten:\n"
      "\n"
      "  '%s'\n"
      "\n"
      "with this new error message:\n"
      "\n"
      "  '%s, at %s:%zu'\n"
      "\n"
      "rcutils_reset_error() should be called after error handling to avoid this.\n"
      "<<<\n",
      gtls_rcutils_error_string.str,
      error_string,
      file,
      line_number);
  }
#endif
  gtls_rcutils_error_state = error_state;
  gtls_rcutils_error_string_is_formatted = false;
  gtls_rcutils_error_string = (const rcutils_error_string_t) {0};
  gtls_rcutils_error_is_set = true;
}

bool
rcutils_error_is_set(void)
{
  return gtls_rcutils_error_is_set;
}

const rcutils_error_state_t *
rcutils_get_error_state(void)
{
  return &gtls_rcutils_error_state;
}

rcutils_error_string_t
rcutils_get_error_string(void)
{
  if (!gtls_rcutils_error_is_set) {
    return (rcutils_error_string_t) {"error not set"};  // NOLINT(readability/braces)
  }
  if (!gtls_rcutils_error_string_is_formatted) {
    __rcutils_format_error_string(&gtls_rcutils_error_string, &gtls_rcutils_error_state);
  }
  return gtls_rcutils_error_string;
}

void
rcutils_reset_error(void)
{
  gtls_rcutils_error_state = (const rcutils_error_state_t) {
    .message = {0}, .file = {0}, .line_number = 0
  };  // NOLINT(readability/braces)
  gtls_rcutils_error_string_is_formatted = false;
  gtls_rcutils_error_string = (const rcutils_error_string_t) {0};
  gtls_rcutils_error_is_set = false;
}
