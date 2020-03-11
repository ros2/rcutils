// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS__SHARED_LIBRARY_H_
#define RCUTILS__SHARED_LIBRARY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>

#ifndef _WIN32
#include <dlfcn.h>
typedef void * shared_library_type;
#else
#include <windows.h>
typedef HINSTANCE shared_library_type;
#endif  // _WIN32

#include "rcutils/allocator.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// Handle to a loaded shared library.
typedef struct RCUTILS_PUBLIC_TYPE rcutils_shared_library_t
{
  /// The pointer to the shared library
  shared_library_type lib_pointer;
  /// The path of the shared_library
  char * library_path;
} rcutils_shared_library_t;

/// Return an empty shared library struct.
/*
 * This function returns an empty and zero initialized shared library struct.
 *
 * Example:
 *
 * ```c
 * // Do not do this:
 * // rcutils_shared_library_t foo;
 * // rcutils_allocator_t allocator = rcutils_get_default_allocator();
 * // rcutils_unload_library(&foo, allocator); // undefined behavior!
 *
 * // Do this instead:
 * rcutils_shared_library_t bar = rcutils_get_zero_initialized_shared_library();
 * rcutils_allocator_t allocator = rcutils_get_default_allocator();
 * rcutils_unload_library(&bar, allocator); // ok
 * ```
 * */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_shared_library_t
rcutils_get_zero_initialized_shared_library(void);

/// Return shared library pointer.
/**
 *
 * the memory of the library path inside the rcutils_shared_library_t struct should be
 * reserved and defined outside this function
 *
 * \param[in] lib struct with the shared library pointer and shared library path name
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_ERROR` if an unknown error occurs
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_get_shared_library(rcutils_shared_library_t * lib);

/// Return shared library symbol pointer.
/**
 * \param[in] lib struct with the shared library pointer and shared library path name
 * \param[in] symbol_name name of the symbol inside the shared library
 * \return shared library symbol pointer.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
void *
rcutils_get_symbol(rcutils_shared_library_t * lib, const char * symbol_name);

/// Return if the shared library contains a specific symbolname .
/**
 * \param[in] lib struct with the shared library pointer and shared library path name
 * \param[in] symbol_name name of the symbol inside the shared library
 * \return returns true on success, and false otherwise.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool
rcutils_has_symbol(rcutils_shared_library_t * lib, const char * symbol_name);

/// Unload the shared library.
/**
 * \param[inout] lib rcutils_shared_library_t to be finalized
 * \param[in] allocator the allocator to use for allocation
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` for invalid arguments, or
 * \return `RCUTILS_RET_ERROR` if an unknown error occurs
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_unload_library(rcutils_shared_library_t * lib, rcutils_allocator_t allocator);

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__SHARED_LIBRARY_H_
