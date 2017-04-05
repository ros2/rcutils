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

#include "gtest/gtest.h"

#include "c_utilities/types/string_array.h"

#define ENABLE_LOGGING 1

#if ENABLE_LOGGING
#define LOG(expected, actual) { \
    printf("Expected: %s Actual: %s\n", expected, actual); }
#else
#define LOG(X, arg) {}
#endif

TEST(test_string_array, boot_string_array) {
  // UNDEFIEND BEHAVIOR
  // string_array_t sa00;
  // utilities_string_array_fini(&sa00);

  string_array_t sa0 = utilities_get_zero_initialized_string_array();
  utilities_string_array_fini(&sa0);

  string_array_t sa1 = utilities_get_pre_initialized_string_array(3);
  utilities_string_array_fini(&sa1);

  string_array_t sa2 = utilities_get_pre_initialized_string_array(2);
  sa2.data[0] = strdup("Hello");
  sa2.data[1] = strdup("World");
  utilities_string_array_fini(&sa2);
}
