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

#ifndef RCUTILS__STDATOMIC_HELPER_H_
#define RCUTILS__STDATOMIC_HELPER_H_

#if !defined(WIN32)

#if defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ <= 9
// If GCC and below GCC-4.9, use the compatability header.
#include "stdatomic_helper/gcc/stdatomic.h"
#elif defined(__clang__) && defined(__has_feature)
#if !__has_feature(c_atomic)
// If Clang and no c_atomics (true for some older versions), use the compatability header.
#include "stdatomic_helper/gcc/stdatomic.h"
#endif
#else
#include <stdatomic.h>
#endif

#define rcutils_atomic_load(object, out) (out) = atomic_load(object)

#define rcutils_atomic_compare_exchange_strong(object, out, expected, desired) \
  (out) = atomic_compare_exchange_strong(object, expected, desired)

#define rcutils_atomic_exchange(object, out, desired) (out) = atomic_exchange(object, desired)

#define rcutils_atomic_store(object, desired) atomic_store(object, desired)

#else  // !defined(WIN32)

#include "./stdatomic_helper/win32/stdatomic.h"

#define rcutils_atomic_load(object, out) rcutils_win32_atomic_load(object, out)

#define rcutils_atomic_compare_exchange_strong(object, out, expected, desired) \
  rcutils_win32_atomic_compare_exchange_strong(object, out, expected, desired)

#define rcutils_atomic_exchange(object, out, desired) \
  rcutils_win32_atomic_exchange(object, out, desired)

#define rcutils_atomic_store(object, desired) rcutils_win32_atomic_store(object, desired)

#endif  // !defined(WIN32)

static inline bool
rcutils_atomic_load_bool(atomic_bool * a_bool)
{
  bool result = false;
  rcutils_atomic_load(a_bool, result);
  return result;
}

static inline int64_t
rcutils_atomic_load_int64_t(atomic_int_least64_t * a_int64_t)
{
  int64_t result = 0;
  rcutils_atomic_load(a_int64_t, result);
  return result;
}

static inline uint64_t
rcutils_atomic_load_uint64_t(atomic_uint_least64_t * a_uint64_t)
{
  uint64_t result = 0;
  rcutils_atomic_load(a_uint64_t, result);
  return result;
}

static inline uintptr_t
rcutils_atomic_load_uintptr_t(atomic_uintptr_t * a_uintptr_t)
{
  uintptr_t result = 0;
  rcutils_atomic_load(a_uintptr_t, result);
  return result;
}

static inline bool
rcutils_atomic_compare_exchange_strong_uint_least64_t(
  atomic_uint_least64_t * a_uint_least64_t, uint64_t * expected, uint64_t desired)
{
  bool result;
  rcutils_atomic_compare_exchange_strong(a_uint_least64_t, result, expected, desired);
  return result;
}

static inline bool
rcutils_atomic_exchange_bool(atomic_bool * a_bool, bool desired)
{
  bool result;
  rcutils_atomic_exchange(a_bool, result, desired);
  return result;
}

static inline int64_t
rcutils_atomic_exchange_int64_t(atomic_int_least64_t * a_int64_t, int64_t desired)
{
  int64_t result;
  rcutils_atomic_exchange(a_int64_t, result, desired);
  return result;
}

static inline uint64_t
rcutils_atomic_exchange_uint64_t(atomic_uint_least64_t * a_uint64_t, uint64_t desired)
{
  uint64_t result;
  rcutils_atomic_exchange(a_uint64_t, result, desired);
  return result;
}

static inline uintptr_t
rcutils_atomic_exchange_uintptr_t(atomic_uintptr_t * a_uintptr_t, uintptr_t desired)
{
  uintptr_t result;
  rcutils_atomic_exchange(a_uintptr_t, result, desired);
  return result;
}

#endif  // RCUTILS__STDATOMIC_HELPER_H_
