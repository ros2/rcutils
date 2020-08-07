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

#ifndef RCUTILS__TESTING__FAULT_INJECTION_H_
#define RCUTILS__TESTING__FAULT_INJECTION_H_
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define RCUTILS_FAULT_INJECTION_NEVER_FAIL -1

#define RCUTILS_FAULT_INJECTION_FAIL_NOW 0

bool rcutils_fault_injection_is_test_complete();

void _rcutils_fault_injection_set_count(int count);

int_least64_t _rcutils_fault_injection_get_count();

int _rcutils_fault_injection_maybe_fail();

/**
 * \def RCUTILS_FAULT_INJECTION_MAYBE_RETURN_ERROR
 * \brief This macro checks and decrements a static global variable atomic counter and returns
 * `return_value_on_error` if 0.
 *
 * This macro is not a function itself, so when this macro returns it will cause the calling
 * function to return with the return value.
 *
 * Set the counter with `RCUTILS_FAULT_INJECTION_SET_COUNT`. If the count is less than 0, then
 * `RCUTILS_FAULT_INJECTION_MAYBE_RETURN_ERROR` will not cause an early return.
 *
 * This macro is thread-safe, and ensures that at most one invocation results in a failure for each
 * time the fault injection counter is set with `RCUTILS_FAULT_INJECTION_SET_COUNT`
 *
 * \param return_value_on_error the value to return in the case of fault injected failure.
 */
#define RCUTILS_FAULT_INJECTION_MAYBE_RETURN_ERROR(return_value_on_error) \
  if (RCUTILS_FAULT_INJECTION_FAIL_NOW == _rcutils_fault_injection_maybe_fail()) { \
    printf( \
      "%s:%d Injecting fault and returning " #return_value_on_error "\n", __FILE__, __LINE__); \
    return return_value_on_error; \
  }

/**
 * \def RCUTILS_FAULT_INJECTION_SET_COUNT
 * \brief Atomically set the fault injection counter.
 *
 * There will be at most one fault injected failure per call to RCUTILS_FAULT_INJECTION_SET_COUNT.
 * To test all reachable fault injection locations, call this macro inside a loop and set the count
 * to an incrementing count variable.
 *
 *  for (int i = 0; i < SUFFICIENTLY_LARGE_ITERATION_COUNT; ++i) {
 *    RCUTILS_FAULT_INJECTION_SET_COUNT(i);
 *    ... // Call function under test
 *  }
 * ASSERT_LT(RCUTILS_FAULT_INJECTION_NEVER_FAIL, RCUTILS_FAULT_INJECTION_GET_COUNT());
 *
 * Where SUFFICIENTLY_LARGE_ITERATION_COUNT is a value larger than the maximum expected calls to
 * `RCUTILS_FAULT_INJECTION_MAYBE_RETURN_ERROR`. This last assertion just insures that your choice for
 * SUFFICIENTLY_LARGE_ITERATION_COUNT was large enough. To avoid having to choose this count
 * yourself, you can use a do-while loop.
 *
 * int i = 0;
 * do {
 *   RCUTILS_FAULT_INJECTION_SET_COUNT(i++);
 *    ... // Call function under test
 * } while (RCUTILS_FAULT_INJECTION_GET_COUNT() <= RCUTILS_FAULT_INJECTION_NEVER_FAIL);
 *
 * \param count The count to set the fault injection counter to. If count is negative, then fault
 * injection errors will be disabled. The counter is globally initialized to
 * RCUTILS_FAULT_INJECTION_NEVER_FAIL.
 */
#define RCUTILS_FAULT_INJECTION_SET_COUNT(count) \
  _rcutils_fault_injection_set_count(count);

/**
 * \def RCUTILS_FAULT_INJECTION_GET_COUNT
 * \brief Atomically get the fault injection counter value
 *
 * Use this macro after running the code under test to check whether the counter reached a negative
 * value. This is helpful so you can verify that you ran the fault injection test in a loop a
 * sufficient number of times. Likewise, if the code under test returned with an error, but the
 * count value was greater or equal to 0, then the failure was not caused by the fault injection
 * counter.
 */
#define RCUTILS_FAULT_INJECTION_GET_COUNT() \
  _rcutils_fault_injection_get_count();

#define RCUTILS_FAULT_INJECTION_TEST(code) \
  do { \
    int fault_injection_count = 0; \
    do { \
      RCUTILS_FAULT_INJECTION_SET_COUNT(fault_injection_count++); \
      code; \
    } while (!rcutils_fault_injection_is_test_complete()); \
    RCUTILS_FAULT_INJECTION_SET_COUNT(RCUTILS_FAULT_INJECTION_NEVER_FAIL); \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__TESTING__FAULT_INJECTION_H_
