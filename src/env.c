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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "rcutils/env.h"
#include "rcutils/error_handling.h"

bool
rcutils_set_env(const char * env_name, const char * env_value)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(false);

  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    env_name, "env_name is null", return false);

#ifdef _WIN32
  if (NULL == env_value) {
    env_value = "";
  }
  if (0 != _putenv_s(env_name, env_value)) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("_putenv_s failed: %d", errno);
    return false;
  }
#else
  if (NULL == env_value) {
    if (0 != unsetenv(env_name)) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("unsetenv failed: %d", errno);
      return false;
    }
  } else {
    if (0 != setenv(env_name, env_value, 1)) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("setenv failed: %d", errno);
      return false;
    }
  }
#endif

  return true;
}

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

const char *
rcutils_get_env(const char * env_name, const char ** env_value)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF("some string error");

  if (NULL == env_name) {
    return "argument env_name is null";
  }
  if (NULL == env_value) {
    return "argument env_value is null";
  }

  // TODO(Suyash458): getenv is deprecated on Windows; consider using getenv_s instead
  *env_value = getenv(env_name);

  if (NULL == *env_value) {
    *env_value = "";
  }
  return NULL;
}

#ifdef _WIN32
#pragma warning(pop)
#endif

const char *
rcutils_get_home_dir(void)
{
  const char * homedir;

  if (rcutils_get_env("HOME", &homedir) == NULL && *homedir != '\0') {
    // The HOME environment variable was set and is non-empty, return it.
    return homedir;
  }

#ifdef _WIN32
  // We didn't find a HOME variable, try USERPROFILE on Windows.
  if (rcutils_get_env("USERPROFILE", &homedir) == NULL && *homedir != '\0') {
    // The USERPROFILE environment variable was set and is non-empty, return it.
    return homedir;
  }
#endif

  // Couldn't get the home directory, return NULL.
  return NULL;
}

#ifdef __cplusplus
}
#endif
