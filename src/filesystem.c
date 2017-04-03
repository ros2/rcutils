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
#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif
#include "c_utilities/concat.h"
#include "c_utilities/filesystem.h"

bool
utilities_get_cwd(char * buffer, size_t max_length)
{
#ifdef WIN32
  if (!_getcwd(buffer, (int)max_length)) {
    return false;
  }
#else
  if (!getcwd(buffer, max_length)) {
    return false;
  }
#endif
  return true;
}

bool
utilities_is_directory(const char * abs_path)
{
  struct stat buf;
  if (stat(abs_path, &buf) < 0) {
    return false;
  }
#ifdef WIN32
  return (buf.st_mode & S_IFDIR) == S_IFDIR;
#else
  return S_ISDIR(buf.st_mode);
#endif
}

bool
utilities_is_file(const char * abs_path)
{
  struct stat buf;
  if (stat(abs_path, &buf) < 0) {
    return false;
  }
#ifdef WIN32
  return (buf.st_mode & S_IFREG) == S_IFREG;
#else
  return S_ISREG(buf.st_mode);
#endif
}

bool
utilities_exists(const char * file_abs_path)
{
  struct stat buf;
  if (stat(file_abs_path, &buf) < 0) {
    return false;
  }
  return true;
}

bool
utilities_is_readable(const char * file_abs_path)
{
  struct stat buf;
  if (!utilities_exists(file_abs_path)) {
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
utilities_is_writable(const char * file_abs_path)
{
  struct stat buf;
  if (!utilities_exists(file_abs_path)) {
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
utilities_is_readable_and_writable(const char * file_abs_path)
{
  struct stat buf;
  if (!utilities_exists(file_abs_path)) {
    return false;
  }
  stat(file_abs_path, &buf);
#ifdef WIN32
  // NOTE(marguedas) on windows all writable files are readable
  // hence the following check is equivalent to "& _S_IWRITE"
  if (!((buf.st_mode & _S_IWRITE) && (buf.st_mode & _S_IREAD))) {
#else
  if (!((buf.st_mode & S_IWUSR) && (buf.st_mode & S_IRUSR))) {
#endif
    return false;
  }
  return true;
}

const char *
utilities_join_path(const char * lhs, const char * rhs)
{
  if (!lhs) {
    return NULL;
  }
  if (!rhs) {
    return NULL;
  }

#ifdef  WIN32
  const char * delimiter = "\\";
#else
  const char * delimiter = "/";
#endif

  return utilities_concat(lhs, rhs, delimiter);
}

#if __cplusplus
}
#endif
