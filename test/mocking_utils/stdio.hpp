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

#ifndef MOCKING_UTILS__STDIO_HPP_
#define MOCKING_UTILS__STDIO_HPP_

#include <stdio.h>
#include <stdarg.h>

#include "patch.hpp"

namespace mocking_utils
{

#define MOCKING_UTILS_CAN_PATCH_VSNPRINTF

#if !defined(MOCKING_UTILS_SUPPORT_VA_LIST)
#undef MOCKING_UTILS_CAN_PATCH_VSNPRINTF
#endif

#if defined(__MACH__)  // as vsnprintf binary API differs between C and C++ in MacOS
#undef MOCKING_UTILS_CAN_PATCH_VSNPRINTF
#endif

#if defined(_WIN32)  // as vsnprintf binary APIs are undocumented in Windows
#undef MOCKING_UTILS_CAN_PATCH_VSNPRINTF
#endif

#if defined(_WIN32)

using _vsnprintf_s_type =
  int (char *, size_t, size_t, const char *, va_list);  // NOLINT(readability/casting)

/// Patch _vsnprintf_s with the given `replacement` in the given `scope`.
// Signature must be explicitly provided to avoid ambiguity with template overloads.
#define patch__vsnprintf_s(scope, replacement) \
  make_patch<__COUNTER__, mocking_utils::_vsnprintf_s_type>( \
    MOCKING_UTILS_PATCH_TARGET(scope, _vsnprintf_s), MOCKING_UTILS_PATCH_PROXY(_vsnprintf_s) \
  ).then_call(replacement)

/// Patch _vscprintf with the given `replacement` in the given `scope`.
#define patch__vscprintf(scope, replacement) patch(scope, _vscprintf, replacement)

#endif  // defined(_WIN32)

}  // namespace mocking_utils

#endif  // MOCKING_UTILS__STDIO_HPP_
