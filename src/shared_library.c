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

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <stdlib.h>

#include "rcutils/error_handling.h"
#include "rcutils/shared_library.h"
#include "rcutils/strdup.h"

rcutils_shared_library_t
rcutils_get_zero_initialized_shared_library(void)
{
  rcutils_shared_library_t zero_initialized_shared_library;
  zero_initialized_shared_library.library_path = NULL;
  zero_initialized_shared_library.lib_pointer = NULL;
  zero_initialized_shared_library.allocator = rcutils_get_zero_initialized_allocator();
  return zero_initialized_shared_library;
}

rcutils_ret_t
rcutils_load_shared_library(
  rcutils_shared_library_t * lib,
  const char * library_path,
  rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(library_path, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&allocator, return RCUTILS_RET_INVALID_ARGUMENT);

  lib->allocator = allocator;

  lib->library_path = rcutils_strdup(library_path, lib->allocator);
  if (NULL == lib->library_path) {
    RCUTILS_SET_ERROR_MSG("unable to allocate memory");
    return RCUTILS_RET_BAD_ALLOC;
  }

#ifndef _WIN32
  lib->lib_pointer = dlopen(lib->library_path, RTLD_LAZY);
  if (!lib->lib_pointer) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("LoadLibrary error: %s", dlerror());
#else
  lib->lib_pointer = LoadLibrary(lib->library_path);
  if (!lib->lib_pointer) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("LoadLibrary error: %lu", GetLastError());
#endif  // _WIN32
    lib->allocator.deallocate(lib->library_path, lib->allocator.state);
    lib->library_path = NULL;
    return RCUTILS_RET_ERROR;
  }
  return RCUTILS_RET_OK;
}

void *
rcutils_get_symbol(const rcutils_shared_library_t * lib, const char * symbol_name)
{
  if (!lib || !lib->lib_pointer || (symbol_name == NULL)) {
    RCUTILS_SET_ERROR_MSG("invalid inputs arguments");
    return NULL;
  }

#ifndef _WIN32
  void * lib_symbol = dlsym(lib->lib_pointer, symbol_name);
  char * error = dlerror();
  if (error != NULL) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting the symbol '%s'. Error '%s'",
      symbol_name, error);
    return NULL;
  }
#else
  void * lib_symbol = GetProcAddress(lib->lib_pointer, symbol_name);
  if (lib_symbol == NULL) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting the symbol '%s'. Error '%d'",
      symbol_name, GetLastError());
    return NULL;
  }
#endif  // _WIN32
  if (!lib_symbol) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "symbol '%s' does not exist in the library '%s'",
      symbol_name, lib->library_path);
    return NULL;
  }
  return lib_symbol;
}

bool
rcutils_has_symbol(const rcutils_shared_library_t * lib, const char * symbol_name)
{
  if (!lib || !lib->lib_pointer || symbol_name == NULL) {
    return false;
  }

#ifndef _WIN32
  // the correct way to test for an error is to call dlerror() to clear any old error conditions,
  // then call dlsym(), and then call dlerror() again, saving its return value into a variable,
  // and check whether this saved value is not NULL.
  dlerror(); /* Clear any existing error */
  void * lib_symbol = dlsym(lib->lib_pointer, symbol_name);
  return dlerror() == NULL && lib_symbol != 0;
#else
  void * lib_symbol = GetProcAddress(lib->lib_pointer, symbol_name);
  return GetLastError() == 0 && lib_symbol != 0;
#endif  // _WIN32
}

rcutils_ret_t
rcutils_unload_shared_library(rcutils_shared_library_t * lib)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib->lib_pointer, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib->library_path, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&lib->allocator, return RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret = RCUTILS_RET_OK;
#ifndef _WIN32
  // The function dlclose() returns 0 on success, and nonzero on error.
  int error_code = dlclose(lib->lib_pointer);
  if (error_code) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("dlclose error: %s", dlerror());
#else
  // If the function succeeds, the return value is nonzero.
  int error_code = FreeLibrary(lib->lib_pointer);
  if (!error_code) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("FreeLibrary error: %lu", GetLastError());
#endif  // _WIN32
    ret = RCUTILS_RET_ERROR;
  }

  lib->allocator.deallocate(lib->library_path, lib->allocator.state);
  lib->library_path = NULL;
  lib->lib_pointer = NULL;
  lib->allocator = rcutils_get_zero_initialized_allocator();
  return ret;
}

#ifdef __cplusplus
}
#endif
