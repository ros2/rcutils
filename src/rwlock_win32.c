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

// When building with MSVC 19.28.29333.0 on Windows 10 (as of 2020-11-11),
// there appears to be a problem with winbase.h (which is included by
// Windows.h).  In particular, warnings of the form:
//
// warning C5105: macro expansion producing 'defined' has undefined behavior
//
// See https://developercommunity.visualstudio.com/content/problem/695656/wdk-and-sdk-are-not-compatible-with-experimentalpr.html
// for more information.  For now disable that warning when including windows.h
#pragma warning(push)
#pragma warning(disable : 5105)
#include <windows.h>
#pragma warning(pop)

#include "./rwlock.h"  // NOLINT

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/types/rcutils_ret.h"

typedef struct rcutils_rwlock_impl_s
{
  SRWLOCK lock;
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

  InitializeSRWLock(&lock->impl->lock);

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_rwlock_read_lock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  AcquireSRWLockShared(&lock->impl->lock);

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_rwlock_read_unlock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  ReleaseSRWLockShared(&lock->impl->lock);

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_rwlock_write_trylock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  return TryAcquireSRWLockExclusive(&lock->impl->lock) ? RCUTILS_RET_OK : RCUTILS_RET_ERROR;
}

rcutils_ret_t
rcutils_rwlock_write_lock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  AcquireSRWLockExclusive(&lock->impl->lock);

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_rwlock_write_unlock(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lock->impl, "invalid lock", return RCUTILS_RET_ERROR);

  ReleaseSRWLockExclusive(&lock->impl->lock);

  return RCUTILS_RET_OK;
}

rcutils_ret_t
rcutils_rwlock_fini(rcutils_rwlock_t * lock)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lock, RCUTILS_RET_INVALID_ARGUMENT);
  if (NULL == lock->impl) {
    return RCUTILS_RET_OK;
  }

  rcutils_allocator_t allocator = lock->impl->allocator;

  allocator.deallocate(lock->impl, allocator.state);
  lock->impl = NULL;

  return RCUTILS_RET_OK;
}
