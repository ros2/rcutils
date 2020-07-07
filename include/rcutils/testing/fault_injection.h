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
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define RCUTILS_FAULT_INJECTION_NEVER_FAIL -1

#define RCUTILS_FAULT_INJECTION_FAIL_NOW 0

void _rcutils_set_fault_injection_count(int count);

int_least64_t _rcutils_get_fault_injection_count();

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
  if (RCUTILS_FAULT_INJECTION_FAIL_NOW == _rcutils_maybe_fail()) { \
    return return_value_on_error; \
  }

/**
 * \def RCUTILS_SET_FAULT_INJECTION_COUNT
 * \brief Atomically set the fault injection counter.
 *
 * There will be at most one fault injected failure per call to RCUTILS_SET_FAULT_INJECTION_COUNT.
 * To test all reachable fault injection locations, call this macro inside a loop and set the count
 * to an incrementing count variable.
 *
 *  for (int i = 0; i < SUFFICIENTLY_LARGE_ITERATION_COUNT; ++i) {
 *    RCUTILS_SET_FAULT_INJECTION_COUNT(i);
 *    ... // Call function under test
 *  }
 * ASSERT_LT(RCUTILS_FAULT_INJECTION_NEVER_FAIL, RCUTILS_GET_FAULT_INJECTION_COUNT());
 *
 * Where SUFFICIENTLY_LARGE_ITERATION_COUNT is a value larger than the maximum expected calls to
 * `RCUTILS_MAYBE_RETURN_ERROR`. This last assertion just insures that your choice for
 * SUFFICIENTLY_LARGE_ITERATION_COUNT was large enough. To avoid having to choose this count
 * yourself, you can use a do-while loop.
 *
 * int i = 0;
 * do {
 *   RCUTILS_SET_FAULT_INJECTION_COUNT(i++);
 *    ... // Call function under test
 * } while (RCUTILS_GET_FAULT_INJECTION_COUNT() <= RCUTILS_FAULT_INJECTION_NEVER_FAIL);
 *
 * \param count The count to set the fault injection counter to. If count is negative, then fault
 * injection errors will be disabled. The counter is globally initialized to
 * RCUTILS_FAULT_INJECTION_NEVER_FAIL.
 */
#define RCUTILS_SET_FAULT_INJECTION_COUNT(count) \
  _rcutils_set_fault_injection_count(count);

/**
 * \def RCUTILS_GET_FAULT_INJECTION_COUNT
 * \brief Atomically get the fault injection counter value
 *
 * Use this macro after running the code under test to check whether the counter reached a negative
 * value. This is helpful so you can verify that you ran the fault injection test in a loop a
 * sufficient number of times. Likewise, if the code under test returned with an error, but the
 * count value was greater or equal to 0, then the failure was not caused by the fault injection
 * counter.
 */
#define RCUTILS_GET_FAULT_INJECTION_COUNT() \
  _rcutils_get_fault_injection_count();

#else  // RCUTILS_ENABLE_FAULT_INJECTION

#define RCUTILS_SET_FAULT_INJECTION_COUNT(count)

// This needs to be set to an int for compatibility
#define RCUTILS_GET_FAULT_INJECTION_COUNT() RCUTILS_FAULT_INJECTION_NEVER_FAIL

#define RCUTILS_MAYBE_RETURN_ERROR(msg, error_statement)

#endif  // defined RCUTILS_ENABLE_FAULT_INJECTION

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__TESTING_MACROS_H_
