// Copyright (c) 2020 Robert Bosch GmbH
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

#ifndef RCUTILS__THREAD_H_
#define RCUTILS__THREAD_H_

#include "rcutils/visibility_control.h"
#include "rcutils/types/rcutils_ret.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// Enum for simple configuration of threads in two priority classes.
enum ThreadPriority
{
  THREAD_PRIORITY_LOW,
  THREAD_PRIORITY_MEDIUM,
  THREAD_PRIORITY_HIGH
};

/// Calculates an OS specific thread priority from a ThreadPriority value.
/**
 * \param[in] thread_priority thread priority of type ThreadPriority
 * \param[out] os_priority OS specific thread priority
 * \return RCUTILS_RET_OK on systems that support POSIX
 */
RCUTILS_LOCAL
rcutils_ret_t calculate_os_thread_priority(
  const int thread_priority,
  int * os_priority);

/// Sets the priority and cpu affinity of the given native thread.
/**
 * This function intentionally only works on operating systems which support a FIFO thread scheduler.
 * Note for Linux: using this function requires elevated privileges and a kernel with realtime patch.
 *
 * Implementation note: For setting thread priorities which are intended for a non-realtime/fair thread
 * scheduler a new utility function should be implemented in order to not mix up different use cases.
 *
 * \param[in] native_handle native thread handle
 * \param[in] priority priority of type ThreadPriority to be set for the given thread
 * \param[in] cpu_bitmask cpu core bitmask for the given thread; use (unsigned) -1 for all cores
 * \return RCUTILS_RET_OK on success
 */
RCUTILS_PUBLIC
rcutils_ret_t configure_native_realtime_thread(
  unsigned long int native_handle, const int priority,
  const unsigned int cpu_bitmask);

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__THREAD_H_