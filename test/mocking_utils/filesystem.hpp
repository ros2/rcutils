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

#ifndef MOCKING_UTILS__FILESYSTEM_HPP_
#define MOCKING_UTILS__FILESYSTEM_HPP_

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#else
#include <windows.h>
#endif

#include <map>
#include <string>
#include <type_traits>

#include "rcutils/macros.h"

#include "patch.hpp"

namespace mocking_utils
{
namespace filesystem
{

/// Platform-independent set of file type constants.
struct FileTypes
{
  static constexpr mode_t REGULAR_FILE = S_IFREG;
  static constexpr mode_t DIRECTORY = S_IFDIR;
};

/// Platform-independent set of file permission constants.
struct Permissions
{
#ifndef _WIN32
  static constexpr mode_t USER_READABLE = S_IRUSR;
  static constexpr mode_t USER_WRITABLE = S_IWUSR;
#else
  static constexpr mode_t USER_READABLE = _S_IREAD;
  static constexpr mode_t USER_WRITABLE = _S_IWRITE;
#endif
};

/// Helper class for patching the filesystem API.
/**
 * \tparam ID Numerical identifier for this patches. Ought to be unique.
 */
template<size_t ID>
class FileSystem
{
public:
  /// Construct mocked filesystem.
  /**
   * \param[in] scope Scope target string, using Mimick syntax.
   *   \see mocking_utils::Patch documentation for further reference.
   */
  explicit FileSystem(const std::string & scope)
#ifndef _WIN32
  : opendir_mock_(MOCKING_UTILS_PATCH_TARGET(scope, opendir),
      MOCKING_UTILS_PATCH_PROXY(opendir)),
#else
  : find_first_file_mock_(MOCKING_UTILS_PATCH_TARGET(scope, FindFirstFile),
      MOCKING_UTILS_PATCH_PROXY(FindFirstFile)),
#endif
#ifndef _GNU_SOURCE
    stat_mock_(MOCKING_UTILS_PATCH_TARGET(scope, stat),
      MOCKING_UTILS_PATCH_PROXY(stat))
#else
    __xstat_mock_(MOCKING_UTILS_PATCH_TARGET(scope, __xstat),
      MOCKING_UTILS_PATCH_PROXY(__xstat))
#endif
  {
#ifndef _WIN32
    opendir_mock_.then_call(std::bind(&FileSystem::do_opendir, this, std::placeholders::_1));
#else
    find_first_file_mock_.then_call(
      std::bind(
        &FileSystem::do_FindFirstFile, this,
        std::placeholders::_1, std::placeholders::_2))
#endif
#ifndef _GNU_SOURCE
    stat_mock_.then_call(
      std::bind(
        &FileSystem::do_stat, this,
        std::placeholders::_1, std::placeholders::_2));
#else
    __xstat_mock_.then_call(
      std::bind(
        &FileSystem::do___xstat, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
#endif
  }

  /// Force APIs that return file descriptors or handles to fail as if these had been exhausted.
  void exhaust_file_descriptors()
  {
#ifdef _WIN32
    forced_errno_ = ERROR_NO_MORE_SEARCH_HANDLES;
#else
    forced_errno_ = EMFILE;
#endif
  }

  /// Get information from file in the mocked filesystem.
  /**
   * \param[in] path Path to the file whose information is to be retrieved.
   *   If file is not found, one will be added.
   * \return mutable reference to file information.
   */
  struct stat & file_info(const std::string & path)
  {
    return files_info_[path];
  }

private:
#ifndef _WIN32
  DIR * do_opendir(const char *)
  {
    if (forced_errno_ != 0) {
      errno = forced_errno_;
      return NULL;
    }
    errno = ENOENT;
    return NULL;
  }
  MOCKING_UTILS_PATCH_TYPE(ID, opendir) opendir_mock_;
#else
  HANDLE do_FindFirstFile(LPCSTR, LPWIN32_FIND_DATAA)
  {
    if (forced_errno_ != 0) {
      SetLastError(forced_errno_);
      return INVALID_HANDLE_VALUE;
    }
    SetLastError(ERROR_FILE_NOT_FOUND);
    return INVALID_HANDLE_VALUE;
  }

  MOCKING_UTILS_PATCH_TYPE(ID, FindFirstFile) find_first_file_mock_;
#endif

#ifndef _GNU_SOURCE
  int do_stat(const char * path, struct stat * info)
  {
#else
  int do___xstat(int, const char * path, struct stat * info)
  {
#endif
    if (files_info_.count(path) == 0) {
      errno = ENOENT;
      return -1;
    }
    *info = files_info_[path];
    return 0;
  }

#ifndef _GNU_SOURCE
  MOCKING_UTILS_PATCH_TYPE(ID, stat) stat_mock_;
#else
  MOCKING_UTILS_PATCH_TYPE(ID, __xstat) __xstat_mock_;
#endif

  int forced_errno_{0};
  std::map<std::string, struct stat> files_info_;
};

}  // namespace filesystem

/// Patch filesystem API in a given `scope`.
#define patch_filesystem(scope) filesystem::FileSystem<__COUNTER__>(scope)

}  // namespace mocking_utils

#endif  // MOCKING_UTILS__FILESYSTEM_HPP_
