// Copyright 2014 Open Source Robotics Foundation, Inc.
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

// Note: migrated from rmw/error_handling.h in 2017-04

#ifndef C_UTILITIES__ERROR_HANDLING_H_
#define C_UTILITIES__ERROR_HANDLING_H_

#if __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>

#include "c_utilities/allocator.h"
#include "c_utilities/macros.h"
#include "c_utilities/visibility_control.h"

/// Struct which encapsulates the error state set by UTILITIES_SET_ERROR_MSG().
typedef struct utilities_error_state_t
{
  const char * message;
  const char * file;
  size_t line_number;
  utilities_allocator_t allocator;
} utilities_error_state_t;

/// Set the error message, as well as the file and line on which it occurred.
/**
 * This is not meant to be used directly, but instead via the
 * UTILITIES_SET_ERROR_MSG(msg) macro.
 *
 * The error_msg parameter is copied into the internal error storage and must
 * be null terminated.
 * The file parameter is not copied, but instead is assumed to be a global as
 * it should be set to the __FILE__ preprocessor literal when used with the
 * UTILITIES_SET_ERROR_MSG() macro.
 * It should also be null terminated.
 *
 * The allocator is kept within the error state so that it can be used to
 * deallocate it in the future.
 * Therefore the allocator state needs to exist until after the last time
 * utilities_reset_error() is called.
 *
 * \param[in] error_msg The error message to set.
 * \param[in] file The path to the file in which the error occurred.
 * \param[in] line_number The line number on which the error occurred.
 * \param[in] allocator The allocator to be used when allocating space for the error state.
 */
C_UTILITIES_PUBLIC
void
utilities_set_error_state(
  const char * error_msg, const char * file, size_t line_number, utilities_allocator_t allocator);

/// Set the error message, as well as append the current file and line number.
/**
 * If an error message was previously set, and utilities_reset_error() was not called
 * afterwards, and this library was built with UTILITIES_REPORT_ERROR_HANDLING_ERRORS
 * turned on, then the previously set error message will be printed to stderr.
 * Error state storage is thread local and so all error related functions are
 * also thread local.
 *
 * \param[in] msg The error message to be set.
 * \param[in] allocator The allocator to be used when allocating space for the error state.
 */
#define UTILITIES_SET_ERROR_MSG(msg, allocator) \
  utilities_set_error_state(msg, __FILE__, __LINE__, allocator);

/// Return `true` if the error is set, otherwise `false`.
C_UTILITIES_PUBLIC
bool
utilities_error_is_set(void);

/// Return an utilities_error_state_t which was set with utilities_set_error_state().
/**
 * The returned pointer will be NULL if no error has been set in this thread.
 *
 * The returned pointer is valid until UTILITIES_SET_ERROR_MSG, utilities_set_error_state,
 * or utilities_reset_error are called in the same thread.
 *
 * \return A pointer to the current error state struct.
 */
C_UTILITIES_PUBLIC
const utilities_error_state_t *
utilities_get_error_state(void);

/// Return the error message followed by `, at <file>:<line>`, or `NULL`.
/**
 * The returned pointer is valid until UTILITIES_SET_ERROR_MSG(),
 * utilities_set_error_state(), or utilities_reset_error() are called from the same thread.
 *
 * \return The current formatted error string, or NULL if not set.
 */
C_UTILITIES_PUBLIC
const char *
utilities_get_error_string(void);

/// Return the error message followed by `, at <file>:<line>` if set, else "error not set".
/**
 * This function is guaranteed to return a valid c-string.
 *
 * The returned pointer is valid until UTILITIES_SET_ERROR_MSG,
 * utilities_set_error_state, or utilities_reset_error are called in the same thread.
 *
 * \return The current error string, with file and line number, or "error not set" if not set.
 */
C_UTILITIES_PUBLIC
const char *
utilities_get_error_string_safe(void);

/// Resets the error state by clearing any previously set error state.
C_UTILITIES_PUBLIC
void
utilities_reset_error(void);

#if __cplusplus
}
#endif

#endif  // C_UTILITIES__ERROR_HANDLING_H_
