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

#if defined(_WIN32)
# error time_unix.c is not intended to be used with win32 based systems
#endif  // defined(_WIN32)

#include "rcutils/logging_macros.h"
#include "rcutils/time.h"

#if defined(__MACH__) && defined(__APPLE__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif  // defined(__MACH__) && defined(__APPLE__)
#include <math.h>

#if defined(__ZEPHYR__)
#include <version.h>
#if ZEPHYR_VERSION_CODE >= ZEPHYR_VERSION(3, 1, 0)
#include <zephyr/posix/time.h>  //  Points to Zephyr toolchain posix time implementation
#else
#include <posix/time.h>  //  Points to Zephyr toolchain posix time implementation
#endif
#else  //  #if KERNELVERSION >= ZEPHYR_VERSION(3, 1, 0)
#include <time.h>
#endif  //  defined(__ZEPHYR__)

#include <errno.h>
#include <unistd.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"

#if !defined(__MACH__) && !defined(__APPLE__)   // Assume clock_get_time is available on OS X.
// This is an appropriate check for clock_gettime() according to:
//   http://man7.org/linux/man-pages/man2/clock_gettime.2.html
# if !defined(_POSIX_TIMERS) || !_POSIX_TIMERS
#  error no monotonic clock function available
# endif  // !defined(_POSIX_TIMERS) || !_POSIX_TIMERS
#endif  // !defined(__MACH__) && !defined(__APPLE__)

static inline bool would_be_negative(const struct timespec * const now)
{
  return now->tv_sec < 0 || (now->tv_nsec < 0 && now->tv_sec == 0);
}

rcutils_ret_t
rcutils_system_time_now(rcutils_time_point_value_t * now)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(now, RCUTILS_RET_INVALID_ARGUMENT);
  struct timespec timespec_now;
  // Using clock_gettime(CLOCK_REALTIME) matches what both Linux and macOS use.
  // For macOS, see the clang implementation at
  // (https://github.com/llvm/llvm-project/blob/baebe12ad0d6f514cd33e418d6504075d3e79c0a/libcxx/src/chrono.cpp)
  if (clock_gettime(CLOCK_REALTIME, &timespec_now) < 0) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("Failed to get system time: %d", errno);
    return RCUTILS_RET_ERROR;
  }
  if (would_be_negative(&timespec_now)) {
    RCUTILS_SET_ERROR_MSG("unexpected negative time");
    return RCUTILS_RET_ERROR;
  }
  *now = RCUTILS_S_TO_NS((int64_t)timespec_now.tv_sec) + timespec_now.tv_nsec;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_steady_time_now(rcutils_time_point_value_t * now)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(now, RCUTILS_RET_INVALID_ARGUMENT);
  struct timespec timespec_now;
  clockid_t monotonic_clock = CLOCK_MONOTONIC;

#if defined(__MACH__) && defined(__APPLE__)
  // On macOS, use CLOCK_MONOTONIC_RAW, which matches the clang implementation
  // (https://github.com/llvm/llvm-project/blob/baebe12ad0d6f514cd33e418d6504075d3e79c0a/libcxx/src/chrono.cpp)
  monotonic_clock = CLOCK_MONOTONIC_RAW;
#endif  // defined(__MACH__) && defined(__APPLE__)

  if (clock_gettime(monotonic_clock, &timespec_now) < 0) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("Failed to get steady time: %d", errno);
    return RCUTILS_RET_ERROR;
  }
  if (would_be_negative(&timespec_now)) {
    RCUTILS_SET_ERROR_MSG("unexpected negative time");
    return RCUTILS_RET_ERROR;
  }
  *now = RCUTILS_S_TO_NS((int64_t)timespec_now.tv_sec) + timespec_now.tv_nsec;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_raw_steady_time_now(rcutils_time_point_value_t * now)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(now, RCUTILS_RET_INVALID_ARGUMENT);
  struct timespec timespec_now;

#if defined(CLOCK_MONOTONIC_RAW)
  clockid_t monotonic_raw_clock = CLOCK_MONOTONIC_RAW;
#else
  clockid_t monotonic_raw_clock = CLOCK_MONOTONIC;
  RCUTILS_LOG_WARN_ONCE(
    "CLOCK_MONOTONIC_RAW is not supported by the platform, using CLOCK_MONOTONIC "
    "instead. This may not provide the desired raw steady time behavior.");
#endif

  if (clock_gettime(monotonic_raw_clock, &timespec_now) < 0) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("Failed to get raw steady time: %d", errno);
    return RCUTILS_RET_ERROR;
  }
  if (would_be_negative(&timespec_now)) {
    RCUTILS_SET_ERROR_MSG("unexpected negative time");
    return RCUTILS_RET_ERROR;
  }
  *now = RCUTILS_S_TO_NS((int64_t)timespec_now.tv_sec) + timespec_now.tv_nsec;
  return RCUTILS_RET_OK;
}
