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

#if __cplusplus
extern "C"
{
#endif

#include "rcutils/time.h"

#include <stdbool.h>
#include <stdlib.h>

#include "./common.h"
#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/stdatomic_helper.h"

// Process default ROS time sources
static rcutils_time_source_t * rcutils_default_ros_time_source;
static rcutils_time_source_t * rcutils_default_steady_time_source;
static rcutils_time_source_t * rcutils_default_system_time_source;

// Internal storage for RCUTILS_ROS_TIME implementation
typedef struct rcutils_ros_time_source_storage_t
{
  atomic_uint_least64_t current_time;
  bool active;
  // TODO(tfoote): store subscription here
} rcutils_ros_time_source_storage_t;

// Implementation only
rcutils_ret_t
rcutils_get_steady_time(void * data, rcutils_time_point_value_t * current_time)
{
  (void)data;  // unused
  return rcutils_steady_time_now(current_time);
}

// Implementation only
rcutils_ret_t
rcutils_get_system_time(void * data, rcutils_time_point_value_t * current_time)
{
  (void)data;  // unused
  return rcutils_system_time_now(current_time);
}

// Internal method for zeroing values on init, assumes time_source is valid
void
rcutils_init_generic_time_source(rcutils_time_source_t * time_source)
{
  time_source->type = RCUTILS_TIME_SOURCE_UNINITIALIZED;
  time_source->pre_update = NULL;
  time_source->post_update = NULL;
  time_source->get_now = NULL;
  time_source->data = NULL;
}

// The function used to get the current ros time.
// This is in the implementation only
rcutils_ret_t
rcutils_get_ros_time(void * data, rcutils_time_point_value_t * current_time)
{
  rcutils_ros_time_source_storage_t * t = (rcutils_ros_time_source_storage_t *)data;
  if (!t->active) {
    return rcutils_get_system_time(data, current_time);
  }
  *current_time = rcutils_atomic_load_uint64_t(&(t->current_time));
  return RCUTILS_RET_OK;
}

bool
rcutils_time_source_valid(rcutils_time_source_t * time_source)
{
  if (time_source == NULL ||
    time_source->type == RCUTILS_TIME_SOURCE_UNINITIALIZED ||
    time_source->get_now == NULL)
  {
    return false;
  }
  return true;
}

rcutils_ret_t
rcutils_time_source_init(
  enum rcutils_time_source_type_t time_source_type, rcutils_time_source_t * time_source
)
{
  switch (time_source_type) {
    case RCUTILS_TIME_SOURCE_UNINITIALIZED:
      RCUTILS_CHECK_ARGUMENT_FOR_NULL(
        time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
      rcutils_init_generic_time_source(time_source);
      return RCUTILS_RET_OK;
    case RCUTILS_ROS_TIME:
      return rcutils_ros_time_source_init(time_source);
    case RCUTILS_SYSTEM_TIME:
      return rcutils_system_time_source_init(time_source);
    case RCUTILS_STEADY_TIME:
      return rcutils_steady_time_source_init(time_source);
    default:
      return RCUTILS_RET_INVALID_ARGUMENT;
  }
}

rcutils_ret_t
rcutils_time_source_fini(rcutils_time_source_t * time_source)
{
  switch (time_source->type) {
    case RCUTILS_ROS_TIME:
      return rcutils_ros_time_source_fini(time_source);
    case RCUTILS_SYSTEM_TIME:
      return rcutils_system_time_source_fini(time_source);
    case RCUTILS_STEADY_TIME:
      return rcutils_steady_time_source_fini(time_source);
    case RCUTILS_TIME_SOURCE_UNINITIALIZED:
    // fall through
    default:
      return RCUTILS_RET_INVALID_ARGUMENT;
  }
}

rcutils_ret_t
rcutils_ros_time_source_init(rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  rcutils_init_generic_time_source(time_source);
  time_source->data = calloc(1, sizeof(rcutils_ros_time_source_storage_t));
  time_source->get_now = rcutils_get_ros_time;
  time_source->type = RCUTILS_ROS_TIME;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_ros_time_source_fini(rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (time_source->type != RCUTILS_ROS_TIME) {
    RCUTILS_SET_ERROR_MSG(
      "time_source not of type RCUTILS_ROS_TIME", rcutils_get_default_allocator());
    return RCUTILS_RET_ERROR;
  }
  free((rcutils_ros_time_source_storage_t *)time_source->data);
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_steady_time_source_init(rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  rcutils_init_generic_time_source(time_source);
  time_source->get_now = rcutils_get_steady_time;
  time_source->type = RCUTILS_STEADY_TIME;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_steady_time_source_fini(rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (time_source->type != RCUTILS_STEADY_TIME) {
    RCUTILS_SET_ERROR_MSG(
      "time_source not of type RCUTILS_STEADY_TIME", rcutils_get_default_allocator());
    return RCUTILS_RET_ERROR;
  }
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_system_time_source_init(rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  rcutils_init_generic_time_source(time_source);
  time_source->get_now = rcutils_get_system_time;
  time_source->type = RCUTILS_SYSTEM_TIME;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_system_time_source_fini(rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (time_source->type != RCUTILS_SYSTEM_TIME) {
    RCUTILS_SET_ERROR_MSG(
      "time_source not of type RCUTILS_SYSTEM_TIME", rcutils_get_default_allocator());
    return RCUTILS_RET_ERROR;
  }
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_time_point_init(rcutils_time_point_t * time_point, rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_point, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (!time_source) {
    time_point->time_source = rcutils_get_default_ros_time_source();
    return RCUTILS_RET_OK;
  }
  time_point->time_source = time_source;

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_time_point_fini(rcutils_time_point_t * time_point)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_point, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  (void)time_point;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_duration_init(rcutils_duration_t * duration, rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    duration, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (!time_source) {
    duration->time_source = rcutils_get_default_ros_time_source();
    return RCUTILS_RET_OK;
  }
  duration->time_source = time_source;

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_duration_fini(rcutils_duration_t * duration)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    duration, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  (void)duration;
  return RCUTILS_RET_OK;
}

rcutils_time_source_t *
rcutils_get_default_ros_time_source(void)
{
  if (!rcutils_default_ros_time_source) {
    rcutils_default_ros_time_source =
      (rcutils_time_source_t *)calloc(1, sizeof(rcutils_time_source_t));
    rcutils_ret_t retval = rcutils_ros_time_source_init(rcutils_default_ros_time_source);
    if (retval != RCUTILS_RET_OK) {
      return NULL;
    }
  }
  return rcutils_default_ros_time_source;
}

rcutils_time_source_t *
rcutils_get_default_steady_time_source(void)
{
  if (!rcutils_default_steady_time_source) {
    rcutils_default_steady_time_source =
      (rcutils_time_source_t *)calloc(1, sizeof(rcutils_time_source_t));
    rcutils_ret_t retval = rcutils_steady_time_source_init(rcutils_default_steady_time_source);
    if (retval != RCUTILS_RET_OK) {
      return NULL;
    }
  }
  return rcutils_default_steady_time_source;
}

rcutils_time_source_t *
rcutils_get_default_system_time_source(void)
{
  if (!rcutils_default_system_time_source) {
    rcutils_default_system_time_source =
      (rcutils_time_source_t *)calloc(1, sizeof(rcutils_time_source_t));
    rcutils_ret_t retval = rcutils_system_time_source_init(rcutils_default_system_time_source);
    if (retval != RCUTILS_RET_OK) {
      return NULL;
    }
  }
  return rcutils_default_system_time_source;
}

rcutils_ret_t
rcutils_set_default_ros_time_source(rcutils_time_source_t * process_time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    process_time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (rcutils_default_ros_time_source) {
    free(rcutils_default_ros_time_source);
  }
  rcutils_default_ros_time_source = process_time_source;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_difference_times(rcutils_time_point_t * start, rcutils_time_point_t * finish,
  rcutils_duration_t * delta)
{
  if (start->time_source->type != finish->time_source->type) {
    RCUTILS_SET_ERROR_MSG(
      "Cannot difference between time points with time_sources types.",
      rcutils_get_default_allocator());
    return RCUTILS_RET_ERROR;
  }
  if (finish->nanoseconds < start->nanoseconds) {
    rcutils_time_point_value_t intermediate = start->nanoseconds - finish->nanoseconds;
    delta->nanoseconds = -1 * (int) intermediate;
  }
  delta->nanoseconds = (int)(finish->nanoseconds - start->nanoseconds);
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_time_point_get_now(rcutils_time_point_t * time_point)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_point, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (time_point->time_source && time_point->time_source->get_now) {
    return time_point->time_source->get_now(time_point->time_source->data,
             &(time_point->nanoseconds));
  }
  RCUTILS_SET_ERROR_MSG(
    "time_source is not initialized or does not have get_now registered.",
    rcutils_get_default_allocator());
  return RCUTILS_RET_ERROR;
}

rcutils_ret_t
rcutils_enable_ros_time_override(rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (time_source->type != RCUTILS_ROS_TIME) {
    RCUTILS_SET_ERROR_MSG(
      "Time source is not RCUTILS_ROS_TIME cannot enable override.",
      rcutils_get_default_allocator())
    return RCUTILS_RET_ERROR;
  }
  rcutils_ros_time_source_storage_t * storage = \
    (rcutils_ros_time_source_storage_t *)time_source->data;
  if (!storage) {
    RCUTILS_SET_ERROR_MSG(
      "Storage not initialized, cannot enable.", rcutils_get_default_allocator())
    return RCUTILS_RET_ERROR;
  }
  storage->active = true;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_disable_ros_time_override(rcutils_time_source_t * time_source)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (time_source->type != RCUTILS_ROS_TIME) {
    return RCUTILS_RET_ERROR;
  }
  rcutils_ros_time_source_storage_t * storage = \
    (rcutils_ros_time_source_storage_t *)time_source->data;
  if (!storage) {
    RCUTILS_SET_ERROR_MSG(
      "Storage not initialized, cannot disable.", rcutils_get_default_allocator())
    return RCUTILS_RET_ERROR;
  }
  storage->active = false;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_is_enabled_ros_time_override(
  rcutils_time_source_t * time_source,
  bool * is_enabled)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    is_enabled, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (time_source->type != RCUTILS_ROS_TIME) {
    return RCUTILS_RET_ERROR;
  }
  rcutils_ros_time_source_storage_t * storage = \
    (rcutils_ros_time_source_storage_t *)time_source->data;
  if (!storage) {
    RCUTILS_SET_ERROR_MSG(
      "Storage not initialized, cannot query.", rcutils_get_default_allocator())
    return RCUTILS_RET_ERROR;
  }
  *is_enabled = storage->active;
  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_set_ros_time_override(
  rcutils_time_source_t * time_source,
  rcutils_time_point_value_t time_value)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(
    time_source, RCUTILS_RET_INVALID_ARGUMENT, rcutils_get_default_allocator());
  if (time_source->type != RCUTILS_ROS_TIME) {
    return RCUTILS_RET_ERROR;
  }
  rcutils_ros_time_source_storage_t * storage = \
    (rcutils_ros_time_source_storage_t *)time_source->data;
  if (storage->active && time_source->pre_update) {
    time_source->pre_update();
  }
  rcutils_atomic_store(&(storage->current_time), time_value);
  if (storage->active && time_source->post_update) {
    time_source->post_update();
  }
  return RCUTILS_RET_OK;
}

#if __cplusplus
}
#endif
