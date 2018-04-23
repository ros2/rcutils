// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#include "rcutils/get_env.h"

#ifdef _WIN32
# define WINDOWS_ENV_BUFFER_SIZE 2048
static char __env_buffer[WINDOWS_ENV_BUFFER_SIZE];
#endif  // _WIN32


const char *
rcutils_get_env(const char * env_name, const char ** env_value)
{
  if (NULL == env_name) {
    return "argument env_name is null";
  }
  if (NULL == env_value) {
    return "argument env_value is null";
  }
  *env_value = NULL;
#ifdef _WIN32
  size_t required_size;
  errno_t ret = getenv_s(&required_size, __env_buffer, sizeof(__env_buffer), env_name);
  if (ret != 0) {
    return "unable to read environment variable";
  }
  __env_buffer[WINDOWS_ENV_BUFFER_SIZE - 1] = '\0';
  *env_value = __env_buffer;
#else
  *env_value = getenv(env_name);
  if (NULL == *env_value) {
    *env_value = "";
  }
#endif  // _WIN32
  return NULL;
}

#ifdef __cplusplus
}
#endif
