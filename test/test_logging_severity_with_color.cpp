// Copyright 2025 Open Source Robotics Foundation, Inc.
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

#include <iostream>

#include "rcutils/error_handling.h"
#include "rcutils/logging.h"
#include "rcutils/types/rcutils_ret.h"

int main(int, char **)
{
  rcutils_ret_t ret = rcutils_logging_initialize();
  if (ret != RCUTILS_RET_OK) {
    fprintf(stderr, "error initializing logging: %s\n", rcutils_get_error_string().str);
    return -1;
  }

  rcutils_ret_t status = rcutils_logging_set_logger_level("name", RCUTILS_LOG_SEVERITY_DEBUG);
  if (status != RCUTILS_RET_OK) {
    fprintf(stderr, "error setting logger level: %s\n", rcutils_get_error_string().str);
    return -1;
  }

  // Log at all 5 severities to check the colorized severity in the log output.
  rcutils_log_location_t location = {"func", "file", 42u};
  rcutils_log(&location, RCUTILS_LOG_SEVERITY_DEBUG, "name", "Debug message");
  rcutils_log(&location, RCUTILS_LOG_SEVERITY_INFO, "name", "Info message");
  rcutils_log(&location, RCUTILS_LOG_SEVERITY_WARN, "name", "Warn message");
  rcutils_log(&location, RCUTILS_LOG_SEVERITY_ERROR, "name", "Error message");
  rcutils_log(&location, RCUTILS_LOG_SEVERITY_FATAL, "name", "Fatal message");

  std::cout.flush();

  return 0;
}
