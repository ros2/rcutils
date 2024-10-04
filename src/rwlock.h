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

#ifndef RWLOCK_H_
#define RWLOCK_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcutils/allocator.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

struct rcutils_rwlock_impl_s;

typedef struct rwlock_s
{
  struct rcutils_rwlock_impl_s * impl;
} rcutils_rwlock_t;

RCUTILS_PUBLIC
rcutils_rwlock_t
rcutils_get_zero_initialized_rwlock(void);

RCUTILS_PUBLIC
rcutils_ret_t
rcutils_rwlock_init(rcutils_rwlock_t * lock, rcutils_allocator_t allocator);

RCUTILS_PUBLIC
rcutils_ret_t
rcutils_rwlock_read_lock(rcutils_rwlock_t * lock);

RCUTILS_PUBLIC
rcutils_ret_t
rcutils_rwlock_read_unlock(rcutils_rwlock_t * lock);

RCUTILS_PUBLIC
rcutils_ret_t
rcutils_rwlock_write_lock(rcutils_rwlock_t * lock);

RCUTILS_PUBLIC
rcutils_ret_t
rcutils_rwlock_write_trylock(rcutils_rwlock_t * lock);

RCUTILS_PUBLIC
rcutils_ret_t
rcutils_rwlock_write_unlock(rcutils_rwlock_t * lock);

RCUTILS_PUBLIC
rcutils_ret_t
rcutils_rwlock_fini(rcutils_rwlock_t * lock);

#ifdef __cplusplus
}
#endif

#endif  // RWLOCK_H_
