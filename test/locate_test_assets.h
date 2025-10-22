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

#ifndef LOCATE_TEST_ASSETS_H_
#define LOCATE_TEST_ASSETS_H_

#include "rcutils/env.h"
#include "rcutils/filesystem.h"

#include <stdio.h>

bool get_test_asset_dir(char * buffer, size_t max_length) {
  const char * runfiles_dir = NULL;
  const char * runfiles_dir_result = rcutils_get_env("RUNFILES_DIR", &runfiles_dir);
  if (runfiles_dir_result == NULL) {
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    char * runfile_path = rcutils_join_path(runfiles_dir, BAZEL_CURRENT_REPOSITORY, allocator);
    char * test_path = rcutils_join_path(runfile_path, "test", allocator);
    snprintf(buffer, max_length, "%s", test_path);
    return true;
  }
  return false;
}

#endif  // LOCATE_TEST_ASSETS_H_
