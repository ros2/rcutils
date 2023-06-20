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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/strdup.h"
#include "rcutils/thread_attr.h"
#include "rcutils/types/rcutils_ret.h"

#define INIT_NUM_THREAD_ATTRIBUTE 0U

rcutils_thread_attrs_t
rcutils_get_zero_initialized_thread_attrs(void)
{
  rcutils_thread_attrs_t ret = {
    NULL,
  };
  return ret;
}

rcutils_ret_t
rcutils_thread_attrs_init(
  rcutils_thread_attrs_t * thread_attrs,
  rcutils_allocator_t allocator)
{
  return rcutils_thread_attrs_init_with_capacity(
    thread_attrs, allocator, INIT_NUM_THREAD_ATTRIBUTE);
}

rcutils_ret_t
rcutils_thread_attrs_init_with_capacity(
  rcutils_thread_attrs_t * thread_attrs,
  rcutils_allocator_t allocator,
  size_t capacity)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);

  thread_attrs->allocator = allocator;
  thread_attrs->num_attributes = 0U;
  thread_attrs->capacity_attributes = capacity;
  if (capacity > 0) {
    thread_attrs->attributes =
      allocator.zero_allocate(capacity, sizeof(rcutils_thread_attr_t), allocator.state);
    if (NULL == thread_attrs->attributes) {
      *thread_attrs = rcutils_get_zero_initialized_thread_attrs();
      RCUTILS_SET_ERROR_MSG("Failed to allocate memory for thread attributes");
      return RCUTILS_RET_BAD_ALLOC;
    }
  }
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_thread_attrs_fini(rcutils_thread_attrs_t * thread_attrs)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);
  rcutils_allocator_t * allocator = &thread_attrs->allocator;
  if (NULL == thread_attrs->attributes) {
    return RCUTILS_RET_OK;
  }
  // check the allocator only if attributes is available to avoid checking after zero-initialized
  RCUTILS_CHECK_ALLOCATOR(allocator, return RCUTILS_RET_INVALID_ARGUMENT);
  for (size_t i = 0; i < thread_attrs->num_attributes; ++i) {
    rcutils_thread_attr_t * attr = thread_attrs->attributes + i;
    if (NULL != attr->name) {
      allocator->deallocate((char *)attr->name, allocator->state);
    }
  }
  allocator->deallocate(thread_attrs->attributes, allocator->state);
  *thread_attrs = rcutils_get_zero_initialized_thread_attrs();

  return RCUTILS_RET_OK;
}

static inline rcutils_ret_t extend_thread_attrs_capacity(
  rcutils_thread_attrs_t * attrs,
  size_t new_cap)
{
  size_t cap = attrs->capacity_attributes;
  size_t size = cap * sizeof(rcutils_thread_attr_t);
  size_t new_size = new_cap * sizeof(rcutils_thread_attr_t);
  rcutils_thread_attr_t * new_attrs = attrs->allocator.reallocate(
    attrs->attributes, new_size, attrs->allocator.state);

  if (NULL == new_attrs) {
    RCUTILS_SET_ERROR_MSG("Failed to allocate memory for thread attributes");
    return RCUTILS_RET_BAD_ALLOC;
  }

  memset(new_attrs + cap, 0, new_size - size);

  attrs->capacity_attributes = new_cap;
  attrs->attributes = new_attrs;

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_thread_attrs_add_attr(
  rcutils_thread_attrs_t * thread_attrs,
  rcutils_thread_scheduling_policy_t sched_policy,
  int core_affinity,
  int priority,
  char const * name)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(name, RCUTILS_RET_INVALID_ARGUMENT);

  if (thread_attrs->num_attributes == thread_attrs->capacity_attributes) {
    size_t new_cap = 0;
    if (0 == thread_attrs->capacity_attributes) {
      new_cap = 1;
    } else {
      new_cap = thread_attrs->capacity_attributes * 2;
    }
    // Extend the capacity
    rcutils_ret_t ret = extend_thread_attrs_capacity(thread_attrs, new_cap);
    if (RCUTILS_RET_OK != ret) {
      return ret;
    }
  }

  char const * dup_name = rcutils_strdup(name, thread_attrs->allocator);
  if (NULL == dup_name) {
    return RCUTILS_RET_BAD_ALLOC;
  }

  rcutils_thread_attr_t * attr = thread_attrs->attributes + thread_attrs->num_attributes;
  attr->scheduling_policy = sched_policy;
  attr->core_affinity = core_affinity;
  attr->priority = priority;
  attr->name = dup_name;

  return RCUTILS_RET_OK;
}
