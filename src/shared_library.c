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
  return zero_initialized_shared_library;
}

rcutils_ret_t
rcutils_get_shared_library(rcutils_shared_library_t * lib)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib->library_path, RCUTILS_RET_INVALID_ARGUMENT);

#ifndef _WIN32
  lib->lib_pointer = dlopen(lib->library_path, RTLD_LAZY);
  if (!lib->lib_pointer) {
    RCUTILS_SET_ERROR_MSG(dlerror);
    return RCUTILS_RET_ERROR;
  }
#else
  lib->lib_pointer = LoadLibrary(lib->library_path);
  if (!lib->lib_pointer) {
    DWORD error = GetLastError();
    RCUTILS_SET_ERROR_MSG("LoadLibrary error");
    return RCUTILS_RET_ERROR;
  }
#endif  // _WIN32
  return RCUTILS_RET_OK;
}

void *
rcutils_get_symbol(rcutils_shared_library_t * lib, const char * symbol_name)
{
#ifndef _WIN32
  void * lib_symbol = dlsym(lib->lib_pointer, symbol_name);
#else
  void * lib_symbol = GetProcAddress(lib->lib_pointer, symbol_name);
#endif  // _WIN32
  if (!lib_symbol) {
    return NULL;
  }
  return lib_symbol;
}

bool
rcutils_has_symbol(rcutils_shared_library_t * lib, const char * symbol_name)
{
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
rcutils_unload_library(rcutils_shared_library_t * lib, rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib->lib_pointer, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib->library_path, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret = RCUTILS_RET_OK;
  int error_code = 0;
#ifndef _WIN32
  // The function dlclose() returns 0 on success, and nonzero on error.
  error_code = dlclose(lib->lib_pointer);
  const char * dlsym_error = dlerror();
  if (dlsym_error) {
    RCUTILS_SET_ERROR_MSG(dlerror());
    ret = RCUTILS_RET_ERROR;
  }
#else
  // zero if the function succeeds
  error_code = FreeLibrary(lib->lib_pointer);
  if (!error_code) {
    RCUTILS_SET_ERROR_MSG(GetLastError());
    ret = RCUTILS_RET_ERROR;
  }
#endif  // _WIN32

  allocator.deallocate(lib->library_path, allocator.state);
  lib->library_path = NULL;
  lib->lib_pointer = NULL;
  return ret;
}

#ifdef __cplusplus
}
#endif
