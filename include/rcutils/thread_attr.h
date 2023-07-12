// Copyright 2023 eSOL Co.,Ltd.
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

#ifndef RCUTILS__THREAD_ATTR_H_
#define RCUTILS__THREAD_ATTR_H_

#include "rcutils/visibility_control.h"

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum rcutils_thread_scheduling_policy_e
{
  RCUTILS_THREAD_SCHEDULING_POLICY_UNKNOWN  = 0,
  RCUTILS_THREAD_SCHEDULING_POLICY_FIFO     = 1,
  RCUTILS_THREAD_SCHEDULING_POLICY_RR       = 2,
  RCUTILS_THREAD_SCHEDULING_POLICY_SPORADIC = 3,
  RCUTILS_THREAD_SCHEDULING_POLICY_OTHER    = 4,
  RCUTILS_THREAD_SCHEDULING_POLICY_IDLE     = 5,
  RCUTILS_THREAD_SCHEDULING_POLICY_BATCH    = 6,
  RCUTILS_THREAD_SCHEDULING_POLICY_DEADLINE = 7
} rcutils_thread_scheduling_policy_t;

typedef struct rcutils_thread_attr_s
{
  /// Thread core affinity
  int core_affinity;
  /// Thread scheduling policy.
  rcutils_thread_scheduling_policy_t scheduling_policy;
  /// Thread priority.
  int priority;
  /// Thread name
  char const * name;
} rcutils_thread_attr_t;

/// Hold thread attribute rules.
typedef struct rcutils_thread_attrs_s
{
  /// Private implementation array.
  rcutils_thread_attr_t * attributes;
  /// Number of threads attribute
  size_t num_attributes;
  /// Number of threads attribute capacity
  size_t capacity_attributes;
  /// Allocator used to allocate objects in this struct
  rcutils_allocator_t allocator;
} rcutils_thread_attrs_t;

/**
 * \brief Return a rcutils_thread_attrs_t struct with members initialized to zero value.
 * \return a rcutils_thread_attrs_t struct with members initialized to zero value.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_thread_attrs_t
rcutils_get_zero_initialized_thread_attrs(void);

/**
 * \brief Initialize list of thread attributes.
 * \param[out] thread_attrs list of thread attributes to be initialized
 * \param[in] allocator memory allocator to be used
 * \return #RCUTILS_RET_OK if the structure was initialized succeessfully, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT if any function arguments are invalid, or
 * \return #RCUTILS_RET_BAD_ALLOC if allocating memory failed, or
 * \return #RCUTILS_RET_ERROR an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_thread_attrs_init(
  rcutils_thread_attrs_t * thread_attrs,
  rcutils_allocator_t allocator);

/**
 * \brief Initialize list of thread attributes with a capacity.
 * \param[out] thread_attrs list of thread attributes to be initialized
 * \param[in] allocator memory allocator to be used
 * \param[in] capacity initial capacity of the list of thread attributes
 * \return #RCUTILS_RET_OK if the structure was initialized succeessfully, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT if any function arguments are invalid, or
 * \return #RCUTILS_RET_BAD_ALLOC if allocating memory failed, or
 * \return #RCUTILS_RET_ERROR an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_thread_attrs_init_with_capacity(
  rcutils_thread_attrs_t * thread_attrs,
  rcutils_allocator_t allocator,
  size_t capacity);

/**
 * \brief Free list of thread attributes
 * \param[in] thread_attrs structure to be deallocated.
 * \return #RCUTILS_RET_OK if the memory was successfully freed, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT if any function arguments are invalid
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_thread_attrs_fini(
  rcutils_thread_attrs_t * thread_attrs);

/**
 * \brief Copy list of thread attributes
 * \param[in] thread_attrs Source list of thread attributes
 * \param[out] out_thread_attrs Destination location
 * \return #RCUTILS_RET_OK if the source list was succesfully copied to the destination, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT if may function arguments are invalid, or
 * \return #RCUTILS_RET_BAD_ALLOC if allocating memory failed
 */
rcutils_ret_t
rcutils_thread_attrs_copy(
  rcutils_thread_attrs_t const * thread_attrs,
  rcutils_thread_attrs_t * out_thread_attrs);

/**
 * \brief Add thread attribute to the list of thread attributes.
 * \param[inout] thread_attrs list of thread attributes to add a thread attribute to
 * \param[in] sched_policy thread scheduling policy of adding attribute
 * \param[in] core_affinity thread core affinity of adding attribute
 * \param[in] priority thread priority of adding attribute
 * \param[in] name thread name of adding attribute
 * \return #RCUTILS_RET_OK if the thread attribute was successfully added, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT if any function arguments are invalid, or
 * \return #RCUTILS_RET_BAD_ALLOC if allocating memory failed, or
 * \return #RCUTILS_RET_ERROR an unspecified error occur.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_thread_attrs_add_attr(
  rcutils_thread_attrs_t * thread_attrs,
  rcutils_thread_scheduling_policy_t sched_policy,
  int core_affinity,
  int priority,
  char const * name);

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__THREAD_ATTR_H_
