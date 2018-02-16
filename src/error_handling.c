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

#include <rcutils/error_handling.h>
#include <rcutils/macros.h>
#include <rcutils/strdup.h>

// When this define evaluates to true (default), then messages will printed to
// stderr when an error is encoutered while setting the error state.
// For example, when memory cannot be allocated or a previous error state is
// being overwritten.
#ifndef RCUTILS_REPORT_ERROR_HANDLING_ERRORS
#define RCUTILS_REPORT_ERROR_HANDLING_ERRORS 1
#endif

#ifdef RCUTILS_THREAD_LOCAL_PTHREAD
#include <pthread.h>
pthread_key_t __rcutils_error_state_key;
pthread_key_t __rcutils_error_string_key;
#else
RCUTILS_THREAD_LOCAL rcutils_error_state_t * __rcutils_error_state = NULL;
RCUTILS_THREAD_LOCAL char * __rcutils_error_string = NULL;
#endif

static const char __error_format_string[] = "%s, at %s:%zu";

bool
__rcutils_error_is_set(rcutils_error_state_t * error_state);

void
__rcutils_reset_error_string(char ** error_string_ptr, rcutils_allocator_t allocator);

void
__rcutils_reset_error(rcutils_error_state_t ** error_state_ptr_ptr);

rcutils_ret_t
rcutils_error_state_copy(const rcutils_error_state_t * src, rcutils_error_state_t * dst)
{
  dst->allocator = src->allocator;
  dst->message = rcutils_strdup(src->message, dst->allocator);
  if (NULL == dst->message) {
    return RCUTILS_RET_BAD_ALLOC;
  }
  dst->file = rcutils_strdup(src->file, dst->allocator);
  if (NULL == dst->file) {
    return RCUTILS_RET_BAD_ALLOC;
  }
  dst->line_number = src->line_number;
  return RCUTILS_RET_OK;
}

void
rcutils_error_state_fini(rcutils_error_state_t * error_state)
{
  error_state->allocator.deallocate((char *)error_state->message, error_state->allocator.state);
  error_state->allocator.deallocate((char *)error_state->file, error_state->allocator.state);
}

void
rcutils_set_error_state(
  const char * error_string,
  const char * file,
  size_t line_number,
  rcutils_allocator_t allocator)
{
  if (!rcutils_allocator_is_valid(&allocator)) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    // rcutils_allocator is invalid, logging to stderr instead
    RCUTILS_SAFE_FWRITE_TO_STDERR(
      "[rcutils|error_handling.c:" RCUTILS_STRINGIFY(__LINE__)
      "] provided allocator is invalid, error state not updated\n");
#endif
    return;
  }
#ifdef RCUTILS_THREAD_LOCAL_PTHREAD
  rcutils_error_state_t * __rcutils_error_state =
    (rcutils_error_state_t *)pthread_getspecific(__rcutils_error_state_key);
  char * __rcutils_error_string = (char *)pthread_getspecific(__rcutils_error_string_key);
#endif
  rcutils_error_state_t * old_error_state = __rcutils_error_state;
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
  const char * old_error_string = rcutils_get_error_string_safe();
#endif
  __rcutils_error_state = (rcutils_error_state_t *)allocator.allocate(
    sizeof(rcutils_error_state_t), allocator.state);
  if (NULL == __rcutils_error_state) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    // rcutils_allocate failed, but fwrite might work?
    RCUTILS_SAFE_FWRITE_TO_STDERR(
      "[rcutils|error_handling.c:" RCUTILS_STRINGIFY(__LINE__)
      "] failed to allocate memory for the error state struct\n");
#endif
    return;
  }
  __rcutils_error_state->allocator = allocator;

#ifdef RCUTILS_THREAD_LOCAL_PTHREAD
  pthread_setspecific(__rcutils_error_state_key, __rcutils_error_state);
#endif
  size_t error_string_length = strlen(error_string);
  // the memory must be one byte bigger to store the NULL character
  __rcutils_error_state->message =
    (char *)allocator.allocate(error_string_length + 1, allocator.state);
  if (NULL == __rcutils_error_state->message) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    // malloc failed, but fwrite might work?
    RCUTILS_SAFE_FWRITE_TO_STDERR(
      "[rcutils|error_handling.c:" RCUTILS_STRINGIFY(__LINE__)
      "] failed to allocate memory for the error message in the error state struct\n");
#endif
    rcutils_reset_error();  // This will free any allocations done so far.
    return;
  }
  // Cast the const away to set ->message initially.
#ifndef _WIN32
  snprintf((char *)__rcutils_error_state->message, error_string_length + 1, "%s", error_string);
#else
  auto retcode = strcpy_s(
    (char *)__rcutils_error_state->message, error_string_length + 1, error_string);
  if (retcode) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    RCUTILS_SAFE_FWRITE_TO_STDERR(
      "[rcutils|error_handling.c:" RCUTILS_STRINGIFY(__LINE__)
      "] failed to copy error message in the error state struct\n");
#endif
  }
#endif
  __rcutils_error_state->file = file;
  __rcutils_error_state->line_number = line_number;
  if (__rcutils_error_is_set(old_error_state)) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    // Only warn of overwritting if the new error string is different from the old ones.
    if (error_string != old_error_string && error_string != old_error_state->message) {
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
        old_error_string,
        error_string,
        file,
        line_number);
    }
#endif
    __rcutils_reset_error(&old_error_state);
  }
  __rcutils_reset_error_string(&__rcutils_error_string, __rcutils_error_state->allocator);
}

const rcutils_error_state_t *
rcutils_get_error_state(void)
{
#ifdef RCUTILS_THREAD_LOCAL_PTHREAD
  return (rcutils_error_state_t *)pthread_getspecific(__rcutils_error_state_key);
#else
  return __rcutils_error_state;
#endif
}

static void
format_error_string(void)
{
#ifdef RCUTILS_THREAD_LOCAL_PTHREAD
  rcutils_error_state_t * __rcutils_error_state =
    (rcutils_error_state_t *)pthread_getspecific(__rcutils_error_state_key);
  char * __rcutils_error_string = (char *)pthread_getspecific(__rcutils_error_string_key);
#endif
  if (!__rcutils_error_is_set(__rcutils_error_state)) {
    return;
  }
  size_t bytes_it_would_have_written = snprintf(
    NULL, 0,
    __error_format_string,
    __rcutils_error_state->message,
    __rcutils_error_state->file,
    __rcutils_error_state->line_number);
  rcutils_allocator_t allocator = __rcutils_error_state->allocator;
  __rcutils_error_string =
    (char *)allocator.allocate(bytes_it_would_have_written + 1, allocator.state);
#ifdef RCUTILS_THREAD_LOCAL_PTHREAD
  pthread_setspecific(__rcutils_error_string_key, __rcutils_error_string);
#endif
  if (NULL == __rcutils_error_string) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    // rcutils_allocate failed, but fwrite might work?
    RCUTILS_SAFE_FWRITE_TO_STDERR(
      "[rcutils|error_handling.c:" RCUTILS_STRINGIFY(__LINE__)
      "] failed to allocate memory for the error string\n");
#endif
    return;
  }
  snprintf(
    __rcutils_error_string, bytes_it_would_have_written + 1,
    __error_format_string,
    __rcutils_error_state->message,
    __rcutils_error_state->file,
    __rcutils_error_state->line_number);
  // The Windows version of snprintf does not null terminate automatically in all cases.
  __rcutils_error_string[bytes_it_would_have_written] = '\0';
}

const char *
rcutils_get_error_string(void)
{
#ifdef RCUTILS_THREAD_LOCAL_PTHREAD
  char * __rcutils_error_string = (char *)pthread_getspecific(__rcutils_error_string_key);
#endif
  if (NULL == __rcutils_error_string) {
    format_error_string();
  }
  return __rcutils_error_string;
}

bool
__rcutils_error_is_set(rcutils_error_state_t * error_state)
{
  return error_state != NULL;
}

bool
rcutils_error_is_set(void)
{
#ifdef RCUTILS_THREAD_LOCAL_PTHREAD
  rcutils_error_state_t * __rcutils_error_state =
    (rcutils_error_state_t *)pthread_getspecific(__rcutils_error_state_key);
#endif
  return __rcutils_error_is_set(__rcutils_error_state);
}

const char *
rcutils_get_error_string_safe(void)
{
  if (!rcutils_error_is_set()) {
    return "error not set";
  }
  return rcutils_get_error_string();
}

void
__rcutils_reset_error_string(char ** error_string_ptr, rcutils_allocator_t allocator)
{
  if (NULL == error_string_ptr) {
    return;
  }

  rcutils_allocator_t local_allocator = allocator;
  if (!rcutils_allocator_is_valid(&allocator)) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    RCUTILS_SAFE_FWRITE_TO_STDERR(
      "[rcutils|error_handling.c:" RCUTILS_STRINGIFY(__LINE__) "]: "
      "invalid allocator\n");
#endif
    local_allocator = rcutils_get_default_allocator();
  }
  char * error_string = *error_string_ptr;
  if (error_string != NULL) {
    local_allocator.deallocate(error_string, local_allocator.state);
  }
  *error_string_ptr = NULL;
}

void
__rcutils_reset_error(rcutils_error_state_t ** error_state_ptr_ptr)
{
  if (error_state_ptr_ptr != NULL) {
    rcutils_error_state_t * error_state_ptr = *error_state_ptr_ptr;
    if (error_state_ptr != NULL) {
      rcutils_allocator_t allocator = error_state_ptr->allocator;
      if (NULL == allocator.deallocate) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
        RCUTILS_SAFE_FWRITE_TO_STDERR(
          "[rcutils|error_handling.c:" RCUTILS_STRINGIFY(__LINE__) "]: "
          "invalid allocator, deallocate function pointer is null\n");
#endif
        allocator = rcutils_get_default_allocator();
      }
      if (error_state_ptr->message != NULL) {
        // Cast const away to delete previously allocated memory.
        allocator.deallocate((char *)error_state_ptr->message, allocator.state);
      }
      allocator.deallocate(error_state_ptr, allocator.state);
    }
    *error_state_ptr_ptr = NULL;
  }
}

void
rcutils_reset_error(void)
{
#ifdef RCUTILS_THREAD_LOCAL_PTHREAD
  rcutils_error_state_t * __rcutils_error_state =
    (rcutils_error_state_t *)pthread_getspecific(__rcutils_error_state_key);
  char * __rcutils_error_string = (char *)pthread_getspecific(__rcutils_error_string_key);
#endif
  if (__rcutils_error_state != NULL) {
    __rcutils_reset_error_string(&__rcutils_error_string, __rcutils_error_state->allocator);
  }
  __rcutils_reset_error(&__rcutils_error_state);
}
