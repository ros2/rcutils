// Copyright 2023 Open Source Robotics Foundation, Inc.
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

#include <pthread.h>
#include <stdlib.h>

#include "./rwlock.h"  // NOLINT

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/types/rcutils_ret.h"

typedef struct rcutils_rwlock_impl_s
{
  pthread_rwlock_t lock;
  rcutils_allocator_t allocator;
} rcutils_rwlock_impl_t;

rcutils_rwlock_t
rcutils_get_zero_initialized_rwlock(void)
{
  static rcutils_rwlock_t zero_initialized_rwlock;
  zero_initialized_rwlock.impl = NULL;
  return zero_initialized_rwlock;
}

rcutils_ret_t
rcutils_rwlock_init(rcutils_rwlock_t * lock, rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  if (lock->impl != NULL) {
    RCUTILS_SET_ERROR_MSG("rwlock already initialized");
    return RCUTILS_RET_ERROR;
  }
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);

  lock->impl = allocator.allocate(sizeof(rcutils_rwlock_impl_t), allocator.state);
  if (NULL == lock->impl) {
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for string map impl struct");
    return RCUTILS_RET_BAD_ALLOC;
  }

  lock->impl->allocator = allocator;

  return pthread_rwlock_init(&lock->impl->lock, NULL) == 0 ? RCUTILS_RET_OK : RCUTILS_RET_ERROR;
}

rcutils_ret_t
rcutils_rwlock_read_lock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  return pthread_rwlock_rdlock(&lock->impl->lock) == 0 ? RCUTILS_RET_OK : RCUTILS_RET_ERROR;
}

rcutils_ret_t
rcutils_rwlock_read_unlock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  return pthread_rwlock_unlock(&lock->impl->lock) == 0 ? RCUTILS_RET_OK : RCUTILS_RET_ERROR;
}

rcutils_ret_t
rcutils_rwlock_write_trylock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  return pthread_rwlock_trywrlock(&lock->impl->lock) == 0 ? RCUTILS_RET_OK : RCUTILS_RET_ERROR;
}

rcutils_ret_t
rcutils_rwlock_write_lock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  return pthread_rwlock_wrlock(&lock->impl->lock) == 0 ? RCUTILS_RET_OK : RCUTILS_RET_ERROR;
}

rcutils_ret_t
rcutils_rwlock_write_unlock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  return pthread_rwlock_unlock(&lock->impl->lock) == 0 ? RCUTILS_RET_OK : RCUTILS_RET_ERROR;
}

rcutils_ret_t
rcutils_rwlock_fini(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  if (NULL == lock->impl) {
    return RCUTILS_RET_OK;
  }

  int retval = pthread_rwlock_destroy(&lock->impl->lock);

  rcutils_allocator_t allocator = lock->impl->allocator;

  allocator.deallocate(lock->impl, allocator.state);
  lock->impl = NULL;

  return retval == 0 ? RCUTILS_RET_OK : RCUTILS_RET_ERROR;
}
