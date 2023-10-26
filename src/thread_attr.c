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
#include <stdbool.h>
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

rcutils_ret_t
rcutils_thread_attrs_copy(
  rcutils_thread_attrs_t const * thread_attrs,
  rcutils_thread_attrs_t * out_thread_attrs)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(out_thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&thread_attrs->allocator, return RCUTILS_RET_INVALID_ARGUMENT);
  if (NULL != out_thread_attrs->attributes) {
    RCUTILS_SET_ERROR_MSG("The destination must be zero initialized");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  if (0 == thread_attrs->num_attributes) {
    return rcutils_thread_attrs_init(out_thread_attrs, thread_attrs->allocator);
  }

  rcutils_ret_t ret;
  size_t i;
  rcutils_allocator_t allocator = thread_attrs->allocator;
  size_t new_size = thread_attrs->num_attributes * sizeof(rcutils_thread_attr_t);
  rcutils_thread_attr_t * new_attrs = allocator.allocate(new_size, allocator.state);

  if (NULL == new_attrs) {
    ret = RCUTILS_RET_BAD_ALLOC;
    goto error;
  }

  for (i = 0; i < thread_attrs->num_attributes; ++i) {
    char * dup_name = rcutils_strdup(thread_attrs->attributes[i].name, allocator);
    if (NULL == dup_name) {
      ret = RCUTILS_RET_BAD_ALLOC;
      goto error;
    }
    new_attrs[i] = thread_attrs->attributes[i];
    new_attrs[i].name = dup_name;
  }
  *out_thread_attrs = *thread_attrs;
  out_thread_attrs->attributes = new_attrs;

  return RCUTILS_RET_OK;

error:
  if (NULL != new_attrs) {
    for (size_t j = 0; j < i; ++j) {
      allocator.deallocate((char *)new_attrs[i].name, allocator.state);
    }
    allocator.deallocate(new_attrs, allocator.state);
  }
  return ret;
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
  rcutils_thread_core_affinity_t const * core_affinity,
  int priority,
  char const * name)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(thread_attrs, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(name, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(core_affinity, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret;
  char const * dup_name = NULL;
  rcutils_thread_core_affinity_t new_affinity = rcutils_get_zero_initialized_thread_core_affinity();

  if (thread_attrs->num_attributes == thread_attrs->capacity_attributes) {
    size_t new_cap = 0;
    if (0 == thread_attrs->capacity_attributes) {
      new_cap = 1;
    } else {
      new_cap = thread_attrs->capacity_attributes * 2;
    }
    // Extend the capacity
    ret = extend_thread_attrs_capacity(thread_attrs, new_cap);
    if (RCUTILS_RET_OK != ret) {
      goto error;
    }
  }

  dup_name = rcutils_strdup(name, thread_attrs->allocator);
  if (NULL == dup_name) {
    goto error;
  }

  rcutils_thread_core_affinity_t src_affinity = *core_affinity;
  src_affinity.allocator = thread_attrs->allocator;
  ret = rcutils_thread_core_affinity_copy(&src_affinity, &new_affinity);
  if (ret != RCUTILS_RET_OK) {
    goto error;
  }

  rcutils_thread_attr_t * attr = thread_attrs->attributes + thread_attrs->num_attributes;
  attr->scheduling_policy = sched_policy;
  attr->core_affinity = new_affinity;
  attr->priority = priority;
  attr->name = dup_name;

  ++thread_attrs->num_attributes;

  return RCUTILS_RET_OK;

error:
  if (NULL != dup_name) {
    thread_attrs->allocator.deallocate((char *)dup_name, thread_attrs->allocator.state);
  }
  if (0 < new_affinity.core_count) {
    rcutils_ret_t tmp_ret = rcutils_thread_core_affinity_fini(&new_affinity);
    (void)tmp_ret;
  }

  return RCUTILS_RET_ERROR;
}

RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_thread_core_affinity_t
rcutils_get_zero_initialized_thread_core_affinity(void)
{
  rcutils_thread_core_affinity_t ret = {
    0,
  };
  return ret;
}

#define BITSET_UNIT_BITS ((size_t)(CHAR_BIT - 1))
#define BITSET_UNIT_SHIFT 3  // popcount(BITSET_UNIT_BITS)
static size_t as_bitset_count(size_t n)
{
  return (n + BITSET_UNIT_BITS) >> BITSET_UNIT_SHIFT;
}
static size_t as_bitset_index(size_t n)
{
  return n >> BITSET_UNIT_SHIFT;
}
static size_t round_up_to_bitset_unit(size_t n)
{
  return (n + BITSET_UNIT_BITS) & ~BITSET_UNIT_BITS;
}
static uint8_t bitset_unit_mask(size_t n)
{
  return 1 << (n & BITSET_UNIT_BITS);
}

static rcutils_ret_t extend_thread_core_affinity(
  rcutils_thread_core_affinity_t * aff, size_t new_core_count)
{
  size_t new_bitset_size = as_bitset_count(new_core_count);
  size_t cur_bitset_size = as_bitset_count(aff->core_count);
  rcutils_allocator_t * alloc = &aff->allocator;
  uint8_t * buf = alloc->reallocate(aff->set, new_bitset_size, alloc->state);

  if (NULL == buf) {
    return RCUTILS_RET_BAD_ALLOC;
  }

  memset(buf + cur_bitset_size, 0, new_bitset_size - cur_bitset_size);
  aff->set = buf;
  aff->core_count = round_up_to_bitset_unit(new_core_count);

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_thread_core_affinity_init(
  rcutils_thread_core_affinity_t * aff,
  rcutils_allocator_t alloc)
{
  return rcutils_thread_core_affinity_init_with_capacity(aff, 0, alloc);
}

rcutils_ret_t
rcutils_thread_core_affinity_init_with_capacity(
  rcutils_thread_core_affinity_t * aff,
  size_t num_cores,
  rcutils_allocator_t alloc)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(aff, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret;
  rcutils_thread_core_affinity_t tmp = {
    .allocator = alloc,
  };

  if (0 < num_cores) {
    ret = extend_thread_core_affinity(&tmp, num_cores);
    if (RCUTILS_RET_OK != ret) {
      goto error;
    }
  }

  *aff = tmp;
  return RCUTILS_RET_OK;

error:
  return ret;
}

rcutils_ret_t
rcutils_thread_core_affinity_fini(rcutils_thread_core_affinity_t * aff)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(aff, RCUTILS_RET_INVALID_ARGUMENT);

  if (NULL == aff->set) {
    return RCUTILS_RET_OK;
  }

  rcutils_allocator_t * alloc = &aff->allocator;
  alloc->deallocate(aff->set, alloc->state);
  *aff = rcutils_get_zero_initialized_thread_core_affinity();

  return RCUTILS_RET_OK;
}

RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_thread_core_affinity_copy(
  rcutils_thread_core_affinity_t const * src, rcutils_thread_core_affinity_t * dest)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dest, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(src, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_thread_core_affinity_t tmp = *src;
  rcutils_allocator_t const * alloc = &src->allocator;

  tmp.set = alloc->allocate(as_bitset_count(src->core_count), alloc->state);
  if (NULL == tmp.set) {
    return RCUTILS_RET_BAD_ALLOC;
  }
  memcpy(tmp.set, src->set, as_bitset_count(src->core_count));

  *dest = tmp;

  return RCUTILS_RET_OK;
}

RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_thread_core_affinity_set(rcutils_thread_core_affinity_t * aff, size_t no)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(aff, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret;
  if (as_bitset_index(no) >= as_bitset_count(aff->core_count)) {
    ret = extend_thread_core_affinity(aff, (no + 1) * 2);
    if (RCUTILS_RET_OK != ret) {
      return ret;
    }
  }
  aff->set[as_bitset_index(no)] |= bitset_unit_mask(no);

  return RCUTILS_RET_OK;
}

RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_thread_core_affinity_unset(rcutils_thread_core_affinity_t * aff, size_t no)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(aff, RCUTILS_RET_INVALID_ARGUMENT);

  if (no >= aff->core_count) {
    return RCUTILS_RET_OK;
  }
  aff->set[as_bitset_index(no)] &= ~bitset_unit_mask(no);

  return RCUTILS_RET_OK;
}

static
rcutils_ret_t
fill_bits(rcutils_thread_core_affinity_t * aff, size_t min_no, size_t max_no, bool set)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(aff, RCUTILS_RET_INVALID_ARGUMENT);
  if (min_no > max_no) {
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  if (!set && min_no >= aff->core_count) {
    return RCUTILS_RET_OK;
  }

  rcutils_ret_t ret;

  if (as_bitset_index(max_no) >= as_bitset_count(aff->core_count)) {
    ret = extend_thread_core_affinity(aff, (max_no + 1) * 2);
    if (RCUTILS_RET_OK != ret) {
      return ret;
    }
  }

  max_no += 1;

  size_t min_index = as_bitset_index(min_no);
  size_t max_index = as_bitset_index(max_no);
  uint8_t lower_bound_bit = bitset_unit_mask(min_no);
  uint8_t lower_bound_byte_mask = ~(lower_bound_bit - 1);
  uint8_t upper_bound_bit = bitset_unit_mask(max_no);
  uint8_t upper_bound_byte_mask = upper_bound_bit - 1;
  if (min_index == max_index) {
    if (set) {
      aff->set[min_index] |= lower_bound_byte_mask & upper_bound_byte_mask;
    } else {
      aff->set[min_index] &= ~(lower_bound_byte_mask & upper_bound_byte_mask);
    }
  } else {
    if (set) {
      aff->set[min_index] |= lower_bound_byte_mask;
      memset(aff->set + min_index + 1, 0xff, max_index - (min_index + 1));
      aff->set[max_index] |= upper_bound_byte_mask;
    } else {
      aff->set[min_index] &= ~lower_bound_byte_mask;
      memset(aff->set + min_index + 1, 0x00, max_index - (min_index + 1));
      aff->set[max_index] &= ~upper_bound_byte_mask;
    }
  }
  return RCUTILS_RET_OK;
}

RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_thread_core_affinity_fill(
  rcutils_thread_core_affinity_t * aff, size_t min_no, size_t max_no)
{
  return fill_bits(aff, min_no, max_no, true);
}

RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_thread_core_affinity_clear(
  rcutils_thread_core_affinity_t * aff, size_t min_no, size_t max_no)
{
  return fill_bits(aff, min_no, max_no, false);
}

RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool
rcutils_thread_core_affinity_is_set(rcutils_thread_core_affinity_t const * aff, size_t no)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(aff, RCUTILS_RET_INVALID_ARGUMENT);

  if (no >= aff->core_count) {
    return false;
  }
  return (aff->set[as_bitset_index(no)] & bitset_unit_mask(no)) != 0;
}
