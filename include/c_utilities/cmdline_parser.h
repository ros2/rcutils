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

#ifndef C_UTILITIES__CMDLINE_PARSER_H_
#define C_UTILITIES__CMDLINE_PARSER_H_

#if __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "c_utilities/visibility_control.h"

C_UTILITIES_PUBLIC
bool
cli_option_exist(char ** begin, char ** end, const char * option);

C_UTILITIES_PUBLIC
char *
cli_get_option(char ** begin, char ** end, const char * option);

#if __cplusplus
}
#endif

#endif  // C_UTILITIES__CMDLINE_PARSER_H_
