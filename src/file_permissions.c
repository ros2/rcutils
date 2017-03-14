// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#if __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "c_utilities/file_permissions.h"

bool
is_directory(const char * abs_path)
{
  struct stat buf;
  if (stat(abs_path, &buf) < 0) {
    return false;
  }
  return S_ISDIR(buf.st_mode);
}

bool
file_exists(const char * file_abs_path)
{
  struct stat buf;
  if (stat(file_abs_path, &buf) < 0) {
    return false;
  }
  return true;
}

bool
is_file_readable(const char * file_abs_path)
{
  struct stat buf;
  if (!file_exists(file_abs_path)) {
    return false;
  }
  stat(file_abs_path, &buf);
#ifdef WIN32
  if (!(buf.st_mode & _S_IREAD)) {
#else
  if (!(buf.st_mode & S_IRUSR)) {
#endif
    return false;
  }
  return true;
}

bool
is_file_writable(const char * file_abs_path)
{
  struct stat buf;
  if (!file_exists(file_abs_path)) {
    return false;
  }
  stat(file_abs_path, &buf);
#ifdef WIN32
  if (!(buf.st_mode & _S_IWRITE)) {
#else
  if (!(buf.st_mode & S_IWUSR)) {
#endif
    return false;
  }
  return true;
}

bool
is_file_readable_and_writable(const char * file_abs_path)
{
  struct stat buf;
  if (!file_exists(file_abs_path)) {
    return false;
  }
  stat(file_abs_path, &buf);
#ifdef WIN32
  // NOTE(marguedas) on windows all files are readable
  // hence the following check is equivalent to "& _S_IWRITE"
  if (!(buf.st_mode & (_S_IWRITE | _S_IREAD))) {
#else
  if (!(buf.st_mode & (S_IWUSR | S_IRUSR))) {
#endif
    return false;
  }
  return true;
}


#if __cplusplus
}
#endif
