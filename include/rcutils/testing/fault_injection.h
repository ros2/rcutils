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

#ifndef RCUTILS__TESTING_MACROS_H_
#define RCUTILS__TESTING_MACROS_H_
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

void _rcutils_set_fault_injection_count(int count);

int _rcutils_maybe_fail();

#if defined RCUTILS_ENABLE_FAULT_INJECTION

/**
 * \def RCUTILS_MAYBE_RETURN_ERROR
 * \brief This macro checks and decrements a static global variable atomic counter and returns
 * `return_value_on_error` if 0.
 *
 * This macro is not a function itself, so when this macro returns it will cause the calling
 * function to return with the return value.
 *
 * Set the counter with `RCUTILS_SET_FAULT_INJECTION_COUNT`. If the count is less than 0, then
 * `RCUTILS_MAYBE_RETURN_ERROR` will not cause an early return.
 *
 * This macro is thread-safe, and ensures that at most one invocation results in a failure for each
 * time the fault injection counter is set with `RCUTILS_SET_FAULT_INJECTION_COUNT`
 *
 * \param return_value_on_error the value to return in the case of fault injected failure.
 */
#define RCUTILS_MAYBE_RETURN_ERROR(return_value_on_error) \
  if (0 == _rcutils_maybe_fail()) { \
    return return_value_on_error; \
  }

/**
 * \def RCUTILS_SET_FAULT_INJECTION_COUNT
 * \brief Atomically set the fault injection counter.
 *
 * There will be at most one fault injected failure per call to RCUTILS_SET_FAULT_INJECTION_COUNT.
 * To test all reachable fault injection locations, call this macro inside a for loop with
 * sufficient iterations setting count to the loop iteration variable. For example:
 *
 *  for (int i = 0; i < SUFFICIENTLY_LARGE_ITERATION_COUNT; ++i) {
 *    RCUTILS_SET_FAULT_INJECTION_COUNT(i);
 *    ... // Call function under test
 *  }
 *
 * Where SUFFICIENTLY_LARGE_ITERATION_COUNT is a value larger than the maximum expected calls to
 * `RCUTILS_MAYBE_RETURN_ERROR`. In your fault injection unit test, it is recommended to run one
 * last iteration with the fault injection counter set to this maximum value and validate that the
 * results of the call to the function under test would result in the same thing as if no fault
 * injection was used. This will help ensure that this maximum value is suitable and will call
 * attention to maintainers if it needs to be increased because more instances of
 * RCUTILS_MAYBE_RETURN_ERROR were introduced.
 *
 * \param count The count to set the fault injection counter to. If count is negative, then fault
 * injection errors will be disabled. The counter is globally initialized to -1.
 */
#define RCUTILS_SET_FAULT_INJECTION_COUNT(count) \
  _rcutils_set_fault_injection_count(count);

#else  // RCUTILS_ENABLE_FAULT_INJECTION

#define RCUTILS_SET_FAULT_INJECTION_COUNT(count)

#define RCUTILS_MAYBE_RETURN_ERROR(msg, error_statement)

#endif  // defined RCUTILS_ENABLE_FAULT_INJECTION

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__TESTING_MACROS_H_
