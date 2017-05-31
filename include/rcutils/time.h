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
typedef uint64_t rcutils_time_point_value_t;
/// A duration of time, measured in nanoseconds.
typedef int64_t rcutils_duration_value_t;

/// Time source type, used to indicate the source of a time measurement.
enum rcutils_time_source_type_t
{
  RCUTILS_TIME_SOURCE_UNINITIALIZED = 0,
  RCUTILS_ROS_TIME,
  RCUTILS_SYSTEM_TIME,
  RCUTILS_STEADY_TIME
};

/// Encapsulation of a time source.
typedef struct rcutils_time_source_t
{
  enum rcutils_time_source_type_t type;
  void (* pre_update)(void);
  void (* post_update)(void);
  rcutils_ret_t (* get_now)(void * data, rcutils_time_point_value_t * now);
  // void (*set_now) (rcutils_time_point_value_t);
  void * data;
} rcutils_time_source_t;

struct rcutils_ros_time_source_storage_t;

/// A single point in time, measured in nanoseconds, the reference point is based on the source.
typedef struct rcutils_time_point_t
{
  rcutils_time_point_value_t nanoseconds;
  rcutils_time_source_t * time_source;
} rcutils_time_point_t;

/// A duration of time, measured in nanoseconds and its source.
typedef struct rcutils_duration_t
{
  rcutils_duration_value_t nanoseconds;
  rcutils_time_source_t * time_source;
} rcutils_duration_t;

// typedef struct rcutils_rate_t
// {
//   rcutils_time_point_value_t trigger_time;
//   int64_t period;
//   rcutils_time_source_t * time_source;
// } rcutils_rate_t;
// TODO(tfoote) integrate rate and timer implementations

/// Check if the time_source has valid values.
/**
 * This function returns true if the time source appears to be valid.
 * It will check that the type is not uninitialized, and that pointers
 * are not invalid.
 * Note that if data is uninitialized it may give a false positive.
 *
 * \param[in] time_source the handle to the time_source which is being queried
 * \return true if the source is believed to be valid, otherwise return false.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool
rcutils_time_source_valid(rcutils_time_source_t * time_source);

/// Initialize a time_source based on the passed type.
/**
 * This will allocate all necessary internal structures, and initialize variables.
 *
 * \param[in] time_source_type the type identifying the time source to provide
 * \param[in] time_source the handle to the time_source which is being initialized
 * \return `RCUTILS_RET_OK` if the time source was successfully initialized, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_time_source_init(
  enum rcutils_time_source_type_t time_source_type, rcutils_time_source_t * time_source
);

/// Finalize a time_source.
/**
 * This will deallocate all necessary internal structures, and clean up any variables.
 * It can be combined with any of the init functions.
 *
 * Passing a time_source with type RCUTILS_TIME_SOURCE_UNINITIALIZED will result in
 * RCUTILS_RET_INVALID_ARGUMENT being returned.
 *
 * \param[in] time_source the handle to the time_source which is being finalized
 * \return `RCUTILS_RET_OK` if the time source was successfully finalized, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_time_source_fini(rcutils_time_source_t * time_source);

/// Initialize a time_source as a RCUTILS_ROS_TIME time source.
/**
 * This will allocate all necessary internal structures, and initialize variables.
 * It is specifically setting up a RCUTILS_ROS_TIME time source.
 *
 * \param[in] time_source the handle to the time_source which is being initialized
 * \return `RCUTILS_RET_OK` if the time source was successfully initialized, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_ros_time_source_init(rcutils_time_source_t * time_source);

/// Finalize a time_source as a `RCUTILS_ROS_TIME` time source.
/**
 * This will deallocate all necessary internal structures, and clean up any variables.
 * It is specifically setting up a `RCUTILS_ROS_TIME` time source. It is expected
 * to be paired with the init fuction.
 *
 * \param[in] time_source the handle to the time_source which is being initialized
 * \return `RCUTILS_RET_OK` if the time source was successfully finalized, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_ros_time_source_fini(rcutils_time_source_t * time_source);

/// Initialize a time_source as a `RCUTILS_STEADY_TIME` time source.
/**
 * This will allocate all necessary internal structures, and initialize variables.
 * It is specifically setting up a `RCUTILS_STEADY_TIME` time source.
 *
 * \param[in] time_source the handle to the time_source which is being initialized
 * \return `RCUTILS_RET_OK` if the time source was successfully initialized, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_steady_time_source_init(rcutils_time_source_t * time_source);

/// Finalize a time_source as a `RCUTILS_STEADY_TIME` time source.
/**
 * Finalize the time_source as a `RCUTILS_STEADY_TIME` time source.
 *
 * This will deallocate all necessary internal structures, and clean up any variables.
 * It is specifically setting up a steady time source. It is expected to be
 * paired with the init fuction.
 *
 * \param[in] time_source the handle to the time_source which is being initialized
 * \return `RCUTILS_RET_OK` if the time source was successfully finalized, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_steady_time_source_fini(rcutils_time_source_t * time_source);

/// Initialize a time_source as a `RCUTILS_SYSTEM_TIME` time source.
/**
 * Initialize the time_source as a `RCUTILS_SYSTEM_TIME` time source.
 *
 * This will allocate all necessary internal structures, and initialize variables.
 * It is specifically setting up a system time source.
 *
 * \param[in] time_source the handle to the time_source which is being initialized
 * \return `RCUTILS_RET_OK` if the time source was successfully initialized, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_system_time_source_init(rcutils_time_source_t * time_source);

/// Finalize a time_source as a `RCUTILS_SYSTEM_TIME` time source.
/**
 * Finalize the time_source as a `RCUTILS_SYSTEM_TIME` time source.
 *
 * This will deallocate all necessary internal structures, and clean up any variables.
 * It is specifically setting up a system time source. It is expected to be paired with
 * the init fuction.
 *
 * \param[in] time_source the handle to the time_source which is being initialized.
 * \return `RCUTILS_RET_OK` if the time source was successfully finalized, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_system_time_source_fini(rcutils_time_source_t * time_source);

/// Initialize a time point using the time_source.
/**
 * This function will initialize the time_point using the time_source
 * as a reference.
 * If the time_source is null it will use the system default time_source.
 *
 * This will allocate all necessary internal structures, and initialize variables.
 * The time_source may be of types `RCUTILS_ROS_TIME`, `RCUTILS_STEADY_TIME`, or
 * `RCUTILS_SYSTEM_TIME`.
 *
 * \param[in] time_point the handle to the time_source which is being initialized.
 * \param[in] time_source the handle to the time_source will be used for reference.
 * \return `RCUTILS_RET_OK` if the last call time was retrieved successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_time_point_init(rcutils_time_point_t * time_point, rcutils_time_source_t * time_source);

/// Finalize a time_point
/**
 * Finalize the time_point such that it is ready for deallocation.
 *
 * This will deallocate all necessary internal structures, and clean up any variables.
 *
 * \param[in] time_point the handle to the time_source which is being finalized.
 * \return `RCUTILS_RET_OK` if the last call time was retrieved successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_time_point_fini(rcutils_time_point_t * time_point);

/// Initialize a duration using the time_source.
/**
 * This function will initialize the duration using the time_source as a reference.
 * If the time_source is null it will use the system default time_source.
 *
 * This will allocate all necessary internal structures, and initialize variables.
 * The time_source may be of types ros, steady, or system.
 *
 * \param[in] duration the handle to the duration which is being initialized.
 * \param[in] time_source the handle to the time_source will be used for reference.
 * \return `RCUTILS_RET_OK` if the last call time was retrieved successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_duration_init(rcutils_duration_t * duration, rcutils_time_source_t * time_source);

/// Finalize a duration
/**
 * Finalize the duration such that it is ready for deallocation.
 *
 * This will deallocate all necessary internal structures, and clean up any variables.
 *
 * \param[in] duration the handle to the duration which is being finalized.
 * \return `RCUTILS_RET_OK` if the last call time was retrieved successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_duration_fini(rcutils_duration_t * duration);

/// Get the default `RCUTILS_ROS_TIME` time source
/**
 * This function will get the process default time source.
 * This time source is specifically of the ROS time abstraction,
 * and may be overridden by updates.
 *
 * If the default has not yet been used it will allocate
 * and initialize the time source.
 *
 * \return rcutils_time_source_t if it successfully found or allocated a
 *         time source. If an error occurred it will return NULL.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_time_source_t *
rcutils_get_default_ros_time_source(void);

/// Get the default `RCUTILS_STEADY_TIME` time source
/**
 * This function will get the process default time source.
 * This time source is specifically of the steady time abstraction,
 * it should not be able to be overridden..
 *
 * If the default has not yet been used it will allocate
 * and initialize the time source.
 *
 * \return rcutils_time_source_t if it successfully found or allocated a
 *         time source. If an error occurred it will return NULL.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_time_source_t *
rcutils_get_default_steady_time_source(void);

/// Get the default `RCUTILS_SYSTEM_TIME` time source
/**
 * This function will get the process default time source.
 * This time source is specifically of the system time abstraction,
 * and may be overridden by updates to the system clock.
 *
 * If the default has not yet been used it will allocate
 * and initialize the time source.
 *
 * \return rcutils_time_source_t if it successfully found or allocated a
 *         time source. If an error occurred it will return NULL.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_time_source_t *
rcutils_get_default_system_time_source(void);

/// Set the current time on the `RCUTILS_ROS_TIME` time source
/**
 * This function is used to set the time on a ROS time source.
 * It will error if passed a different time source.
 *
 * This should not block, except on Windows. One caveat is that
 * if the ROS time abstraction is active, it will invoke the user
 * defined callbacks, for pre and post update notifications. The
 * callbacks are supposed to be short running and non-blocking.
 *
 * \param[in] process_time_source The time source on which to set the value.
 * \return `RCUTILS_RET_OK` if the value was set successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_set_default_ros_time_source(rcutils_time_source_t * process_time_source);

/// Compute the difference between two time points
/**
 * This function takes two time points and computes the duration between them.
 * The two time points must be using the same time abstraction, and the
 * resultant duration will also be of the same abstraction.
 *
 * The value will be computed as duration = finish - start. If start is after
 * finish the duration will be negative.
 *
 * \param[in] start The time point for the start of the duration.
 * \param[in] finish The time point for the end of the duration.
 * \param[out] delta The duration between the start and finish.
 * \return `RCUTILS_RET_OK` if the difference was computed successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_difference_times(rcutils_time_point_t * start, rcutils_time_point_t * finish,
  rcutils_duration_t * delta);

/// Fill the time point with the current value of the associated clock.
/**
 * This function will populate the data of the time_point object with the
 * current value from it's associated time abstraction.
 *
 * \param[out] time_point The time_point to populate.
 * \return `RCUTILS_RET_OK` if the last call time was retrieved successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_time_point_get_now(rcutils_time_point_t * time_point);


/// Enable the ROS time abstraction override.
/**
 * This method will enable the ROS time abstraction override values,
 * such that the time source will report the set value instead of falling
 * back to system time.
 *
 * \param[in] time_source The time_source to enable.
 * \return `RCUTILS_RET_OK` if the time source was enabled successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_enable_ros_time_override(rcutils_time_source_t * time_source);

/// Disable the ROS time abstraction override.
/**
 * This method will disable the `RCUTILS_ROS_TIME` time abstraction override values,
 * such that the time source will report the system time even if a custom
 * value has been set.
 *
 * \param[in] time_source The time_source to disable.
 * \return `RCUTILS_RET_OK` if the time source was disabled successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_disable_ros_time_override(rcutils_time_source_t * time_source);


/// Check if the `RCUTILS_ROS_TIME` time source has the override enabled.
/**
 * This will populate the is_enabled object to indicate if the
 * time overide is enabled. If it is enabled, the set value will be returned.
 * Otherwise this time source will return the equivalent to system time abstraction.
 *
 * \param[in] time_source The time_source to query.
 * \param[out] is_enabled Whether the override is enabled..
 * \return `RCUTILS_RET_OK` if the time source was queried successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_is_enabled_ros_time_override(rcutils_time_source_t * time_source,
  bool * is_enabled);

/// Set the current time for this `RCUTILS_ROS_TIME` time source.
/**
 * This function will update the internal storage for the `RCUTILS_ROS_TIME`
 * time source.
 * If queried and override enabled the time source will return this value,
 * otherwise it will return the system time.
 *
 * \param[in] time_source The time_source to update.
 * \param[in] time_value The new current time.
 * \return `RCUTILS_RET_OK` if the time source was set successfully, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCUTILS_RET_ERROR` an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_set_ros_time_override(rcutils_time_source_t * time_source,
  rcutils_time_point_value_t time_value);

/// Retrieve the current time as a rcutils_time_point_value_t.
/**
 * This function returns the time from a system clock.
 * The closest equivalent would be to std::chrono::system_clock::now();
 *
 * The resolution (e.g. nanoseconds vs microseconds) is not guaranteed.
 *
 * The now argument must point to an allocated rcutils_system_time_point_t struct,
 * as the result is copied into this variable.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes [1]
 * <i>[1] if `atomic_is_lock_free()` returns true for `atomic_int_least64_t`</i>
 *
 * \todo TODO(tfoote): consider moving this to rmw for more reuse
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
 * Lock-Free          | Yes [1]
 * <i>[1] if `atomic_is_lock_free()` returns true for `atomic_int_least64_t`</i>
 *
 * \todo TODO(tfoote): consider moving this to rmw for more reuse
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
