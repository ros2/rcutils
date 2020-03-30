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

#ifndef RCUTILS__DIRECTORY_AND_FILE_READER_H_
#define RCUTILS__DIRECTORY_AND_FILE_READER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/stat.h>
#include <sys/types.h>

#ifndef _WIN32
#include <dirent.h>
#include <limits.h>
# define RCUTILS_DIR_PATH_MAX PATH_MAX
#else
#include <windows.h>
#include <tchar.h>
#include <direct.h>
# define RCUTILS_DIR_PATH_MAX MAX_PATH
#endif  // _WIN32

#ifndef RCUTILS_DIR_PATH_MAX
#define RCUTILS_DIR_PATH_MAX 4096
#endif
#define RCUTILS_FILENAME_MAX 256

#include "rcutils/allocator.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// Handle files.
typedef struct rcutils_file_t
{
  /// path of the file
  char * path;
  /// filename
  char * name;
  /// is the file a directory ?
  int is_dir;
  //  information about the file
  #ifndef _WIN32
  struct stat _s;
  #else
  struct _stat _s;
  #endif
} rcutils_file_t;

/// Handle directories.
typedef struct rcutils_dir_t
{
  /// path of the directory
  char * path;
  // is there any file in the folder.
  int has_next;

  #ifndef _WIN32
  /// Directory struct
  DIR * dir;
  struct dirent * f;
  #else
  HANDLE dir;
  WIN32_FIND_DATA f;
  #endif

  /// allocator
  rcutils_allocator_t allocator;
} rcutils_dir_t;


/// Return an empty dir struct.
/*
 * This function returns an empty and zero initialized directory struct.
 *
 * Example:
 * rcutils_dir_t bar = rcutils_get_zero_initialized_dir();
 * rcutils_ret_t ret = rcutils_open_dir(&bar, "directory", rcutils_get_default_allocator());
 * ret = rcutils_next_dir(&bar);
 * ret = rcutils_close_dir(&bar);
 *
 * // Do not do this:
 * // rcutils_dir_t foo;
 * // rcutils_open_dir(&foo, "directory", rcutils_get_default_allocator()); // undefined behavior!
 * // rcutils_ret_t ret = retrcutils_next_dir(&foo); // undefined behavior!
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_dir_t
rcutils_get_zero_initialized_dir(void);

/// Return an empty file struct.
/*
 * This function returns an empty and zero initialized file struct.
 *
 * Example:
 * rcutils_dir_t bar = rcutils_get_zero_initialized_dir();
 * rcutils_file_t foo = rcutils_get_zero_initialized_file();
 * rcutils_ret_t ret = rcutils_open_dir(
 *          &bar, "directory", rcutils_get_default_allocator());
 * ret = rcutils_readfile(&bar, &foo)
 * ret = rcutils_next_dir(&bar);
 * ret = rcutils_close_dir(&bar);
 *
 * // Do not do this:
 * // rcutils_dir_t foo =  rcutils_get_zero_initialized_dir();
 * // rcutils_file_t file;
 * // rcutils_ret_t ret =rcutils_open_dir(
 *         &foo, "directory", rcutils_get_default_allocator());
 * // rcutils_ret_t ret = rcutils_readfile(&foo, &file); // undefined behavior!
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_file_t
rcutils_get_zero_initialized_file(void);

/// Destroy a file struct
/**
 * \param[in] file struct with file information
 * \param[in] allocator to be used to deallocate memory
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` for invalid arguments
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_file_fini(rcutils_file_t * file, rcutils_allocator_t allocator);

/// Open a directory
/**
 * \param[inout] dir struct with directory information
 * \param[in] path string with the path of the directory
 * \param[in] allocator to be used to allocate and deallocate memory
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_BAD_ALLOC` if memory allocation fails, or
 * \return `RCUTILS_RET_ERROR` if an unknown error occurs, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` for invalid arguments
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_open_dir(rcutils_dir_t * dir, const char * path, rcutils_allocator_t allocator);

/// Get next file in the directory.
/**
 * \param[inout] dir struct with directory information
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_ERROR` if an unknown error occurs, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` for invalid arguments
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_next_dir(rcutils_dir_t * dir);

/// Read file information
/**
* \param[inout] dir struct with directory information
* \param[inout] dile struct with file information
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_ERROR` if an unknown error occurs, or
 * \return `RCUTILS_RET_BAD_ALLOC` if memory allocation fails, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` for invalid arguments
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_readfile(rcutils_dir_t * dir, rcutils_file_t * file);

/// Close the directory
/**
* \param[in] dir struct with directory information
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_ERROR` if close fails, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` for invalid arguments
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_close_dir(rcutils_dir_t * dir);

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__DIRECTORY_AND_FILE_READER_H_
