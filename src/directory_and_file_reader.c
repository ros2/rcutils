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

#include "rcutils/directory_and_file_reader.h"
#include "rcutils/error_handling.h"
#include "rcutils/filesystem.h"
#include "rcutils/strdup.h"

rcutils_dir_t
rcutils_get_zero_initialized_dir(void)
{
  rcutils_dir_t zero_initialized_dir;
  zero_initialized_dir.path = NULL;
  zero_initialized_dir.has_next = 0;
  zero_initialized_dir.dir = NULL;
  #ifndef _WIN32
  zero_initialized_dir.f = NULL;
  #endif
  zero_initialized_dir.allocator = rcutils_get_zero_initialized_allocator();
  return zero_initialized_dir;
}

rcutils_file_t
rcutils_get_zero_initialized_file(void)
{
  rcutils_file_t zero_initialized_file;
  zero_initialized_file.path = NULL;
  zero_initialized_file.name = NULL;
  zero_initialized_file.is_dir = 0;
  return zero_initialized_file;
}

rcutils_ret_t
rcutils_file_fini(rcutils_file_t * file, rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(file, RCUTILS_RET_INVALID_ARGUMENT);

  if (file->path != NULL) {
    allocator.deallocate(file->path, allocator.state);
  }
  if (file->name != NULL) {
    allocator.deallocate(file->name, allocator.state);
  }

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_open_dir(rcutils_dir_t * dir, const char * path, rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dir, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(path, RCUTILS_RET_INVALID_ARGUMENT);

  if (strlen(path) == 0) {
    RCUTILS_SET_ERROR_MSG("The size of the path is 0");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  if (strlen(path) >= RCUTILS_DIR_PATH_MAX) {
    RCUTILS_SET_ERROR_MSG("The size of the path is too long");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  dir->allocator = allocator;

  if (dir->path != NULL) {
    dir->allocator.deallocate(dir->path, dir->allocator.state);
  }

  if (dir->dir != NULL) {
    errno = 0;
    #ifndef _WIN32
    int error = closedir(dir->dir);
    // The closedir() function returns 0 on success
    if (error) {
      #else
    // If the function succeeds, the return value is nonzero.
    if (!FindClose(dir->dir)) {
    #endif
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("error closing the directory: '%s'", dir->path);
      return RCUTILS_RET_ERROR;
    }
  }

  dir->path = rcutils_strdup(path, dir->allocator);
  if (NULL == dir->path) {
    RCUTILS_SET_ERROR_MSG("unable to allocate memory");
    return RCUTILS_RET_BAD_ALLOC;
  }
  errno = 0;
#ifndef _WIN32
  dir->dir = opendir(dir->path);
  if (dir->dir == NULL) {
#else
  char * path_buf[RCUTILS_DIR_PATH_MAX];
  TCHAR * pathp = &dir->path[strlen(dir->path) - 1];
  while (pathp != dir->path && (*pathp == _TEXT('\\') || *pathp == _TEXT('/')))
  {
    *pathp = _TEXT('\0');
    pathp++;
  }

  if (rcutils_snprintf(path_buf, "%s%s", dir->path, _TEXT("\\*")) < 0)
  {
    RCUTILS_SET_ERROR_MSG("failed concat strings");
    return RCUTILS_RET_ERROR;
  }

  dir->dir = FindFirstFile(path_buf, &dir->f);
  if (dir->dir == INVALID_HANDLE_VALUE) {
#endif
    int errsv = errno;
    if (errsv == EACCES) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("permission denied '%s'", dir->path);
    } else if (errsv == EBADF) {
      RCUTILS_SET_ERROR_MSG("fd is not a valid file descriptor opened for reading.");
    } else if (errsv == EMFILE) {
      RCUTILS_SET_ERROR_MSG("Too many file descriptors in use by proces.");
    } else if (errsv == ENFILE) {
      RCUTILS_SET_ERROR_MSG("Too many files are currently open in the system.");
    } else if (errsv == ENOENT) {
      RCUTILS_SET_ERROR_MSG("Directory does not exist, or name is an empty string.");
    } else if (errsv == ENOMEM) {
      RCUTILS_SET_ERROR_MSG("Insufficient memory to complete the operation.");
    } else if (errsv == ENOTDIR) {
      RCUTILS_SET_ERROR_MSG("name is not a directory.");
    }
    dir->allocator.deallocate(dir->path, dir->allocator.state);
    return RCUTILS_RET_ERROR;
  }
  dir->has_next = 1;

#ifndef _WIN32
  dir->f = readdir(dir->dir);
  if (dir->f == NULL) {
    dir->has_next = 0;
  }
#else
  WIN32_FIND_DATA _temp_f;
  if (!FindNextFile(dir->dir, &_temp_f)) {
    dir->has_next = 0;
  }
#endif
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_readfile(rcutils_dir_t * dir, rcutils_file_t * file)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dir, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dir->path, RCUTILS_RET_INVALID_ARGUMENT);
#ifndef _WIN32
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dir->f, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dir->f->d_name, RCUTILS_RET_INVALID_ARGUMENT);
#endif
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(file, RCUTILS_RET_INVALID_ARGUMENT);

  const char * filename =
#ifndef _WIN32
    dir->f->d_name;
#else
    dir->f.cFileName;
#endif

  if (file->path != NULL) {
    dir->allocator.deallocate(file->path, dir->allocator.state);
  }
  file->path = rcutils_strdup(dir->path, dir->allocator);
  if (NULL == file->path) {
    RCUTILS_SET_ERROR_MSG("unable to allocate memory");
    return RCUTILS_RET_BAD_ALLOC;
  }

  if (file->name != NULL) {
    dir->allocator.deallocate(file->name, dir->allocator.state);
  }
  file->name = rcutils_strdup(filename, dir->allocator);
  if (NULL == file->name) {
    dir->allocator.deallocate(file->name, dir->allocator.state);
    RCUTILS_SET_ERROR_MSG("unable to allocate memory");
    return RCUTILS_RET_BAD_ALLOC;
  }

#ifndef _WIN32
  if (lstat(
#else
  if (_tstat(
#endif
    file->path, &file->_s) == -1){
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "error reading information about the file: '%s'",
      file->path);
    dir->allocator.deallocate(file->path, dir->allocator.state);
    dir->allocator.deallocate(file->name, dir->allocator.state);
    return RCUTILS_RET_ERROR;
  }

  file->is_dir = rcutils_is_directory(file->path);

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_next_dir(rcutils_dir_t * dir)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dir, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dir->dir, RCUTILS_RET_INVALID_ARGUMENT);

  if (!dir->has_next) {
    return RCUTILS_RET_ERROR;
  }

#ifndef _WIN32
  dir->f = readdir(dir->dir);
  if (dir->f == NULL) {
    dir->has_next = 0;
    return RCUTILS_RET_ERROR;
  }
#else
  if (!FindNextFile(dir->dir, &dir->f)) {
    dir->has_next = 0;
    if (GetLastError() != ERROR_SUCCESS &&
        GetLastError() != ERROR_NO_MORE_FILES) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("error reading next directory: '%s'", dir->path);
      if (rcutils_close_dir(dir) != RCUTILS_RET_OK) {
        RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "error reading next directory and closing file: '%s'",
          dir->path);
      }
      return RCUTILS_RET_ERROR;
    }
  }
#endif
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_close_dir(rcutils_dir_t * dir)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dir, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret = RCUTILS_RET_OK;

  if (dir->path != NULL)
    dir->allocator.deallocate(dir->path, dir->allocator.state);
  dir->path = NULL;

  dir->has_next = 0;
  errno = 0;
#ifndef _WIN32
  int error = closedir(dir->dir);
  // The closedir() function returns 0 on success
  if (error) {
#else
  // If the function succeeds, the return value is nonzero.
  if (!FindClose(dir->dir)) {
#endif
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("error closing the directory: '%s'", dir->path);
    ret = RCUTILS_RET_ERROR;
  }
  dir->dir = NULL;
#ifndef _WIN32
  dir->f = NULL;
#endif
  dir->allocator = rcutils_get_zero_initialized_allocator();
  return ret;
}

#ifdef __cplusplus
}
#endif
