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
#include "rcutils/shared_library.h"

#include <stdio.h>
#include <stdlib.h>

rcutils_shared_library_t *
rcutils_get_shared_library(const char * library_path)
{
  rcutils_shared_library_t * lib = (rcutils_shared_library_t *)
    malloc(sizeof(rcutils_shared_library_t));

#ifndef _WIN32
  lib->lib_pointer = dlopen(library_path, RTLD_LAZY);
#else
  lib->lib_pointer = LoadLibrary(library_path);
#endif  // _WIN32
  if (!lib->lib_pointer) {
    free(lib);
    return NULL;
  }

  // +1 to accomodate for the null terminator
  lib->library_path = (char *) malloc((strlen(library_path) + 1) * sizeof(char));
  snprintf(lib->library_path, strlen(library_path), "%s", library_path);

  return lib;
}

void *
rcutils_get_symbol(rcutils_shared_library_t * lib, const char * symbol_name)
{
#ifndef _WIN32
  void * lib_symbol = dlsym(lib->lib_pointer, symbol_name);
  const char * dlsym_error = dlerror();
  if (dlsym_error) {
    fprintf(
      stderr, "Cannot load symbol '%s' in shared library '%s'",
      symbol_name, lib->library_path);
    dlclose(lib->lib_pointer);
    free(lib->library_path);
    free(lib);
    return NULL;
  }
#else
  void * lib_symbol = GetProcAddress(lib->lib_pointer, symbol_name);
  if (!lib_symbol) {
    fprintf(
      stderr, "Cannot load symbol '%s' in shared library '%s'",
      symbol_name, lib->library_path);
    FreeLibrary(lib->lib_pointer);
    free(lib->library_path);
    free(lib);
    return NULL;
  }
#endif  // _WIN32
  return lib_symbol;
}

void
rcutils_unload_library(rcutils_shared_library_t * lib)
{
#ifndef _WIN32
  dlclose(lib->lib_pointer);
#else
  FreeLibrary(lib->lib_pointer);
#endif  // _WIN32
  free(lib->library_path);
  free(lib);
}

#ifdef __cplusplus
}
#endif
