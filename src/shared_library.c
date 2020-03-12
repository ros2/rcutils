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

#include "rcutils/shared_library.h"
#include "rcutils/error_handling.h"


rcutils_shared_library_t
rcutils_get_zero_initialized_shared_library(void)
{
  rcutils_shared_library_t zero_initialized_shared_library;
  zero_initialized_shared_library.library_path = NULL;
  zero_initialized_shared_library.lib_pointer = NULL;
  zero_initialized_shared_library.allocator = rcutils_get_default_allocator();
  return zero_initialized_shared_library;
}

rcutils_ret_t
rcutils_load_shared_library(rcutils_shared_library_t * lib, const char * library_path)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib, RCUTILS_RET_INVALID_ARGUMENT);

  // allocating memory to
  lib->library_path = (char *)(lib->allocator.allocate(
      (strlen(library_path) + 1) * sizeof(char),
      lib->allocator.state));

  if (!lib->library_path) {
    RCUTILS_SET_ERROR_MSG("unable to allocate memory");
    return RCUTILS_RET_BAD_ALLOC;
  }

  // copying string
  snprintf(lib->library_path, strlen(library_path) + 1, "%s", library_path);

#ifndef _WIN32
  lib->lib_pointer = dlopen(lib->library_path, RTLD_LAZY);
  if (!lib->lib_pointer) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("dlclose error: %s", dlerror());
    return RCUTILS_RET_ERROR;
  }
#else
  lib->lib_pointer = LoadLibrary(lib->library_path);
  if (!lib->lib_pointer) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("LoadLibrary error: %lu", GetLastError());
    return RCUTILS_RET_ERROR;
  }
#endif  // _WIN32
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
#else
  void * lib_symbol = GetProcAddress(lib->lib_pointer, symbol_name);
#endif  // _WIN32
  if (!lib_symbol) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "symbol '%s' doesnt' exit in library '%s'",
      symbol_name, lib->library_path);
    return NULL;
  }
  return lib_symbol;
}

bool
rcutils_has_symbol(const rcutils_shared_library_t * lib, const char * symbol_name)
{
  if (!lib || !lib->lib_pointer || symbol_name) {
    return false;
  }

#ifndef _WIN32
  void * lib_symbol = dlsym(lib->lib_pointer, symbol_name);
#else
  void * lib_symbol = GetProcAddress(lib->lib_pointer, symbol_name);
#endif  // _WIN32
  if (!lib_symbol) {
    return false;
  }
  return true;
}

rcutils_ret_t
rcutils_unload_library(rcutils_shared_library_t * lib)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib->lib_pointer, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib->library_path, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret = RCUTILS_RET_OK;
#ifndef _WIN32
  // The function dlclose() returns 0 on success, and nonzero on error.
  int error_code = dlclose(lib->lib_pointer);
  if (error_code) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("dlclose error: %s", dlerror());
    ret = RCUTILS_RET_ERROR;
  }
#else
  // zero if the function succeeds
  int error_code = FreeLibrary(lib->lib_pointer);
  if (!error_code) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("FreeLibrary error: %lu", GetLastError());
    ret = RCUTILS_RET_ERROR;
  }
#endif  // _WIN32

  lib->allocator.deallocate(lib->library_path, lib->allocator.state);
  lib->library_path = NULL;
  lib->lib_pointer = NULL;
  return ret;
}

#ifdef __cplusplus
}
#endif
