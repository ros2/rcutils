// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN32 || defined __CYGWIN__
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"

int rcutils_get_pid(void)
{
#if defined _WIN32 || defined __CYGWIN__
  return (int)GetCurrentProcessId();
#else
  return (int)getpid();
#endif
}

char *rcutils_get_program_name(rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return NULL);

#if defined __APPLE__
  const char * appname = getprogname();
#elif defined __GNUC__
  const char * appname = program_invocation_name;
#elif defined _WIN32 || defined __CYGWIN
  int32_t size;
  const char appname[MAX_PATH];
  size = GetModuleFileNameA(NULL, appname, MAX_PATH);
  // TODO(clalancette): deal with error
#else
#error "Unsupported OS"
#endif

  size_t len = strlen(appname);

  // Since the above memory may be static, and the caller may want to modify
  // the argument, make and return a copy here.
  char * basec = allocator.allocate(len + 1, allocator.state);
  if (NULL == basec) {
    return NULL;
  }
  memcpy(basec, appname, len);
  basec[len] = '\0';

  return basec;
}

#ifdef __cplusplus
}
#endif
