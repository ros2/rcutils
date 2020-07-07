// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rcutils/testing/fault_injection.h"

#include "rcutils/stdatomic_helper.h"

static atomic_int_least64_t g_rcutils_fault_injection_count = -1;

int _rcutils_maybe_fail()
{
  bool set_atomic_success = false;
  int_least64_t current_count = RCUTILS_FAULT_INJECTION_NEVER_FAIL;
  rcutils_atomic_load(&g_rcutils_fault_injection_count, current_count);
  do {
    // A fault_injection_count less than 0 means that maybe_fail doesn't fail, so just return.
    if (current_count <= RCUTILS_FAULT_INJECTION_NEVER_FAIL) {
      return current_count;
    }

    // Otherwise decrement by one, but do so in a thread-safe manner so that exactly one calling
    // thread gets the 0 case.
    int_least64_t desired_count = current_count - 1;
    rcutils_atomic_compare_exchange_strong(
      &g_rcutils_fault_injection_count, set_atomic_success, &current_count, desired_count);
  } while (!set_atomic_success);
  return current_count;
}

void _rcutils_set_fault_injection_count(int count)
{
  rcutils_atomic_store(&g_rcutils_fault_injection_count, count);
}

int_least64_t _rcutils_get_fault_injection_count()
{
  int_least64_t count = 0;
  rcutils_atomic_load(&g_rcutils_fault_injection_count, count);
  return count;
}
