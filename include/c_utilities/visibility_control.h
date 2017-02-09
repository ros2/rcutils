// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifndef C_UTILITIES__VISIBILITY_CONTROL_H_
#define C_UTILITIES__VISIBILITY_CONTROL_H_

#if __cplusplus
extern "C"
{
#endif

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define C_UTILITIES_EXPORT __attribute__ ((dllexport))
    #define C_UTILITIES_IMPORT __attribute__ ((dllimport))
  #else
    #define C_UTILITIES_EXPORT __declspec(dllexport)
    #define C_UTILITIES_IMPORT __declspec(dllimport)
  #endif
  #ifdef C_UTILITIES_BUILDING_DLL
    #define C_UTILITIES_PUBLIC C_UTILITIES_EXPORT
  #else
    #define C_UTILITIES_PUBLIC C_UTILITIES_IMPORT
  #endif
  #define C_UTILITIES_PUBLIC_TYPE C_UTILITIES_PUBLIC
  #define C_UTILITIES_LOCAL
#else
  #define C_UTILITIES_EXPORT __attribute__ ((visibility("default")))
  #define C_UTILITIES_IMPORT
  #if __GNUC__ >= 4
    #define C_UTILITIES_PUBLIC __attribute__ ((visibility("default")))
    #define C_UTILITIES_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define C_UTILITIES_PUBLIC
    #define C_UTILITIES_LOCAL
  #endif
  #define C_UTILITIES_PUBLIC_TYPE
#endif

#if __cplusplus
}
#endif

#endif  // C_UTILITIES__VISIBILITY_CONTROL_H_
