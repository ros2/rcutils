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
typedef void * rcutils_shared_library_handle_t;
#else
#include <windows.h>
typedef HINSTANCE rcutils_shared_library_handle_t;
#endif  // _WIN32

#include "rcutils/allocator.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// Handle to a loaded shared library.
typedef struct RCUTILS_PUBLIC_TYPE rcutils_shared_library_t
{
  /// The pointer to the shared library
  rcutils_shared_library_handle_t lib_pointer;
  /// The path of the shared_library
  char * library_path;
  /// allocator
  rcutils_allocator_t allocator;
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
 * rcutils_load_shared_library(&bar, "library_name"); // ok
 * ```
 * */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_shared_library_t
rcutils_get_zero_initialized_shared_library(void);

/// Return shared library pointer.
/**
 * \param[inout] lib struct with the shared library pointer and shared library path name
 * \param[in] library_path string with the path of the library
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_BAD_ALLOC` if memory allocation fails, or
 * \return `RCUTILS_RET_ERROR` if an unknown error occurs, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` for invalid arguments
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_load_shared_library(rcutils_shared_library_t * lib, const char * library_path);

/// Return shared library symbol pointer.
/**
 * \param[in] lib struct with the shared library pointer and shared library path name
 * \param[in] symbol_name name of the symbol inside the shared library
 * \return shared library symbol pointer, if the symbol doesn't exist then returns NULL.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
void *
rcutils_get_symbol(const rcutils_shared_library_t * lib, const char * symbol_name);

/// Return if the shared library contains a specific symbolname .
/**
 * \param[in] lib struct with the shared library pointer and shared library path name
 * \param[in] symbol_name name of the symbol inside the shared library
 * \return if symbols exists returns true, otherwise returns false.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool
rcutils_has_symbol(const rcutils_shared_library_t * lib, const char * symbol_name);

/// Unload the shared library.
/**
 * \param[inout] lib rcutils_shared_library_t to be finalized
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` for invalid arguments, or
 * \return `RCUTILS_RET_ERROR` if an unknown error occurs
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_unload_library(rcutils_shared_library_t * lib);

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__SHARED_LIBRARY_H_
