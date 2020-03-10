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
#else
#include <windows.h>
#endif  // _WIN32

#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// The structure
typedef struct rcutils_shared_library_t
{
  /// The pointer to the shared library.
  #ifndef _WIN32
  void * lib_pointer;
  #else
  HINSTANCE lib_pointer;
  #endif
  /// The path of the shared_library
  char * library_path;
} rcutils_shared_library_t;

/// Return shared library pointer.
/**
 * \param[in] library_path path to the library
 * \return void* shared library pointer.
 *                nullptr if library doesn't exist
 */
RCUTILS_PUBLIC
rcutils_shared_library_t *
rcutils_get_shared_library(const char * library_path);

/// Return shared library symbol pointer.
/**
 * \param[in] symbol_name name of the symbol inside the shared library
 * \return void* symbol pointer.
 *                nullptr if symbol doesn't exist
 */
RCUTILS_PUBLIC
void *
rcutils_get_symbol(rcutils_shared_library_t * lib, const char * symbol_name);

/// Unload the shared library.
/**
 * \param[in] lib struct to the shared library
 */
RCUTILS_PUBLIC
void
rcutils_unload_library(rcutils_shared_library_t * lib);

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__SHARED_LIBRARY_H_
