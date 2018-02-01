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

#ifndef RCUTILS__TIME_H_
#define RCUTILS__TIME_H_

#if __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "rcutils/macros.h"
#include "rcutils/types.h"
#include "rcutils/visibility_control.h"

/// Convenience macro to convert seconds to nanoseconds.
#define RCUTILS_S_TO_NS(seconds) (seconds * (1000 * 1000 * 1000))
/// Convenience macro to convert milliseconds to nanoseconds.
#define RCUTILS_MS_TO_NS(milliseconds) (milliseconds * (1000 * 1000))
/// Convenience macro to convert microseconds to nanoseconds.
#define RCUTILS_US_TO_NS(microseconds) (microseconds * 1000)

/// Convenience macro to convert nanoseconds to seconds.
#define RCUTILS_NS_TO_S(nanoseconds) (nanoseconds / (1000 * 1000 * 1000))
/// Convenience macro to convert nanoseconds to milliseconds.
#define RCUTILS_NS_TO_MS(nanoseconds) (nanoseconds / (1000 * 1000))
/// Convenience macro to convert nanoseconds to microseconds.
#define RCUTILS_NS_TO_US(nanoseconds) (nanoseconds / 1000)

/// A single point in time, measured in nanoseconds since the Unix epoch.
typedef int64_t rcutils_time_point_value_t;
/// A duration of time, measured in nanoseconds.
typedef int64_t rcutils_duration_value_t;

/**
 * This function returns the time from a system clock.
 * The closest equivalent would be to std::chrono::system_clock::now();
 *
 * The resolution (e.g. nanoseconds vs microseconds) is not guaranteed.
 *
 * The now argument must point to an allocated rcutils_time_point_value_t object,
 * as the result is copied into this variable.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[out] now a datafield in which the current time is stored
 * \return `RCUTILS_RET_OK` if the current time was successfully obtained, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_system_time_now(rcutils_time_point_value_t * now);

/// Retrieve the current time as a rcutils_time_point_value_t object.
/**
 * This function returns the time from a monotonically increasing clock.
 * The closest equivalent would be to std::chrono::steady_clock::now();
 *
 * The resolution (e.g. nanoseconds vs microseconds) is not guaranteed.
 *
 * The now argument must point to an allocated rcutils_time_point_value_t object,
 * as the result is copied into this variable.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[out] now a struct in which the current time is stored
 * \return `RCUTILS_RET_OK` if the current time was successfully obtained, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_steady_time_now(rcutils_time_point_value_t * now);

#if __cplusplus
}
#endif

#endif  // RCUTILS__TIME_H_
