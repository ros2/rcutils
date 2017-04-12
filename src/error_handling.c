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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c_utilities/error_handling.h>
#include <c_utilities/macros.h>

#ifndef UTILITIES_REPORT_ERROR_HANDLING_ERRORS
#define UTILITIES_REPORT_ERROR_HANDLING_ERRORS 1
#endif

#define SAFE_FWRITE_TO_STDERR(msg) fwrite(msg, sizeof(char), sizeof(msg), stderr)

#ifdef UTILITIES_THREAD_LOCAL_PTHREAD
#include <pthread.h>
pthread_key_t __utilities_error_state_key;
pthread_key_t __utilities_error_string_key;
#else
UTILITIES_THREAD_LOCAL utilities_error_state_t * __utilities_error_state = NULL;
UTILITIES_THREAD_LOCAL char * __utilities_error_string = NULL;
#endif

static const char __error_format_string[] = "%s, at %s:%zu";

bool
__utilities_error_is_set(utilities_error_state_t * error_state);

void
__utilities_reset_error_string(char ** error_string_ptr, utilities_allocator_t allocator);

void
__utilities_reset_error(utilities_error_state_t ** error_state_ptr_ptr);

void
utilities_set_error_state(
  const char * error_string,
  const char * file,
  size_t line_number,
  utilities_allocator_t allocator)
{
#ifdef UTILITIES_THREAD_LOCAL_PTHREAD
  utilities_error_state_t * __utilities_error_state =
    (utilities_error_state_t *)pthread_getspecific(__utilities_error_state_key);
  char * __utilities_error_string = (char *)pthread_getspecific(__utilities_error_string_key);
#endif
  utilities_error_state_t * old_error_state = __utilities_error_state;
#if UTILITIES_REPORT_ERROR_HANDLING_ERRORS
  const char * old_error_string = utilities_get_error_string_safe();
#endif
  __utilities_error_state = (utilities_error_state_t *)allocator.allocate(
    sizeof(utilities_error_state_t), allocator.state);
  if (!__utilities_error_state) {
#if UTILITIES_REPORT_ERROR_HANDLING_ERRORS
    // utilities_allocate failed, but fwrite might work?
    SAFE_FWRITE_TO_STDERR(
      "[rmw|error_handling.c:" UTILITIES_STRINGIFY(__LINE__)
      "] failed to allocate memory for the error state struct\n");
#endif
    return;
  }
  __utilities_error_state->allocator = allocator;

#ifdef UTILITIES_THREAD_LOCAL_PTHREAD
  pthread_setspecific(__utilities_error_state_key, __utilities_error_state);
#endif
  size_t error_string_length = strlen(error_string);
  // the memory must be one byte bigger to store the NULL character
  __utilities_error_state->message =
    (char *)allocator.allocate(error_string_length + 1, allocator.state);
  if (!__utilities_error_state->message) {
#if UTILITIES_REPORT_ERROR_HANDLING_ERRORS
    // malloc failed, but fwrite might work?
    SAFE_FWRITE_TO_STDERR(
      "[rmw|error_handling.c:" UTILITIES_STRINGIFY(__LINE__)
      "] failed to allocate memory for the error message in the error state struct\n");
#endif
    utilities_reset_error();  // This will free any allocations done so far.
    return;
  }
  // Cast the const away to set ->message initially.
#ifndef _WIN32
  snprintf((char *)__utilities_error_state->message, error_string_length + 1, "%s", error_string);
#else
  auto retcode = strcpy_s(
    (char *)__utilities_error_state->message, error_string_length + 1, error_string);
  if (retcode) {
#if UTILITIES_REPORT_ERROR_HANDLING_ERRORS
    SAFE_FWRITE_TO_STDERR(
      "[rmw|error_handling.c:" UTILITIES_STRINGIFY(__LINE__)
      "] failed to copy error message in the error state struct\n");
#endif
  }
#endif
  __utilities_error_state->file = file;
  __utilities_error_state->line_number = line_number;
  if (__utilities_error_is_set(old_error_state)) {
#if UTILITIES_REPORT_ERROR_HANDLING_ERRORS
    // Only warn of overwritting if the new error string is different from the old ones.
    if (error_string != old_error_string && error_string != old_error_state->message) {
      fprintf(
        stderr,
        "[rmw|error_handling.c:" UTILITIES_STRINGIFY(__LINE__) "] utilities_set_error_state():"
        "error string being overwritten: %s\n",
        old_error_string);
    }
#endif
    __utilities_reset_error(&old_error_state);
  }
  __utilities_reset_error_string(&__utilities_error_string, __utilities_error_state->allocator);
}

const utilities_error_state_t *
utilities_get_error_state()
{
#ifdef UTILITIES_THREAD_LOCAL_PTHREAD
  return (utilities_error_state_t *)pthread_getspecific(__utilities_error_state_key);
#else
  return __utilities_error_state;
#endif
}

static void
format_error_string()
{
#ifdef UTILITIES_THREAD_LOCAL_PTHREAD
  utilities_error_state_t * __utilities_error_state =
    (utilities_error_state_t *)pthread_getspecific(__utilities_error_state_key);
  char * __utilities_error_string = (char *)pthread_getspecific(__utilities_error_string_key);
#endif
  if (!__utilities_error_is_set(__utilities_error_state)) {
    return;
  }
  size_t bytes_it_would_have_written = snprintf(
    NULL, 0,
    __error_format_string,
    __utilities_error_state->message,
    __utilities_error_state->file,
    __utilities_error_state->line_number);
  utilities_allocator_t allocator = __utilities_error_state->allocator;
  __utilities_error_string =
    (char *)allocator.allocate(bytes_it_would_have_written + 1, allocator.state);
#ifdef UTILITIES_THREAD_LOCAL_PTHREAD
  pthread_setspecific(__utilities_error_string_key, __utilities_error_string);
#endif
  if (!__utilities_error_string) {
#if UTILITIES_REPORT_ERROR_HANDLING_ERRORS
    // utilities_allocate failed, but fwrite might work?
    SAFE_FWRITE_TO_STDERR(
      "[rmw|error_handling.c:" UTILITIES_STRINGIFY(__LINE__)
      "] failed to allocate memory for the error string\n");
#endif
    return;
  }
  snprintf(
    __utilities_error_string, bytes_it_would_have_written + 1,
    __error_format_string,
    __utilities_error_state->message,
    __utilities_error_state->file,
    __utilities_error_state->line_number);
  // The Windows version of snprintf does not null terminate automatically in all cases.
  __utilities_error_string[bytes_it_would_have_written] = '\0';
}

const char *
utilities_get_error_string()
{
#ifdef UTILITIES_THREAD_LOCAL_PTHREAD
  char * __utilities_error_string = (char *)pthread_getspecific(__utilities_error_string_key);
#endif
  if (!__utilities_error_string) {
    format_error_string();
  }
  return __utilities_error_string;
}

bool
__utilities_error_is_set(utilities_error_state_t * error_state)
{
  return error_state != NULL;
}

bool
utilities_error_is_set()
{
#ifdef UTILITIES_THREAD_LOCAL_PTHREAD
  utilities_error_state_t * __utilities_error_state =
    (utilities_error_state_t *)pthread_getspecific(__utilities_error_state_key);
#endif
  return __utilities_error_is_set(__utilities_error_state);
}

const char *
utilities_get_error_string_safe()
{
  if (!utilities_error_is_set()) {
    return "error not set";
  }
  return utilities_get_error_string();
}

void
__utilities_reset_error_string(char ** error_string_ptr, utilities_allocator_t allocator)
{
  char * error_string = *error_string_ptr;
  if (error_string) {
    if (error_string) {
      allocator.deallocate(error_string, allocator.state);
    }
  }
  *error_string_ptr = NULL;
}

void
__utilities_reset_error(utilities_error_state_t ** error_state_ptr_ptr)
{
  if (error_state_ptr_ptr) {
    utilities_error_state_t * error_state_ptr = *error_state_ptr_ptr;
    if (error_state_ptr) {
      utilities_allocator_t allocator = error_state_ptr->allocator;
      if (error_state_ptr->message) {
        // Cast const away to delete previously allocated memory.
        allocator.deallocate((char *)error_state_ptr->message, allocator.state);
      }
      allocator.deallocate(error_state_ptr, allocator.state);
    }
    *error_state_ptr_ptr = NULL;
  }
}

void
utilities_reset_error()
{
#ifdef UTILITIES_THREAD_LOCAL_PTHREAD
  utilities_error_state_t * __utilities_error_state =
    (utilities_error_state_t *)pthread_getspecific(__utilities_error_state_key);
  char * __utilities_error_string = (char *)pthread_getspecific(__utilities_error_string_key);
#endif
  __utilities_reset_error_string(&__utilities_error_string, __utilities_error_state->allocator);
  __utilities_reset_error(&__utilities_error_state);
}
