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

#include <gtest/gtest.h>

#include "rcutils/thread.h"
#include "rcutils/error_handling.h"

#ifdef __linux__
#include <pthread.h>
#endif  // __linux__

TEST(test_thread, config_rt_thread) {
#ifdef __linux__
  const unsigned cpu_id = 0;
  const unsigned long cpu_bitmask = 1 << cpu_id;
  const int priority = THREAD_PRIORITY_MEDIUM;
  if (configure_native_realtime_thread(pthread_self(), priority, cpu_bitmask) != RCUTILS_RET_OK) {
    GTEST_SKIP() << "Unable to set realtime thread priority.";
    return;
  }

  struct sched_param params_self;
  int policy_self;
  EXPECT_EQ(0, pthread_getschedparam(pthread_self(), &policy_self, &params_self));
  int os_prio_calculated;
  EXPECT_EQ(RCUTILS_RET_OK, calculate_os_fifo_thread_priority(priority, &os_prio_calculated));
  EXPECT_EQ(os_prio_calculated, params_self.sched_priority);
  EXPECT_EQ(SCHED_FIFO, policy_self);

  cpu_set_t cpuset_self;
  EXPECT_EQ(0, pthread_getaffinity_np(pthread_self(), sizeof(cpuset_self), &cpuset_self));
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_id, &cpuset);
  EXPECT_EQ(0, memcmp(&cpuset, &cpuset_self, sizeof(cpu_set_t)));
#else
  GTEST_SKIP() << "Testcase not implemented for this platform."
#endif  // __linux__
}

TEST(test_thread, calculate_rt_priorities) {
#ifdef __linux__
  int prio = -1;
  const int max_prio = sched_get_priority_max(SCHED_FIFO);
  const int min_prio = sched_get_priority_min(SCHED_FIFO);
  EXPECT_EQ(RCUTILS_RET_OK, calculate_os_fifo_thread_priority(THREAD_PRIORITY_LOWEST, &prio));
  EXPECT_EQ(min_prio, prio);
  EXPECT_EQ(RCUTILS_RET_OK, calculate_os_fifo_thread_priority(THREAD_PRIORITY_MEDIUM, &prio));
  EXPECT_EQ((max_prio + min_prio) / 2 - 1, prio);
  EXPECT_EQ(RCUTILS_RET_OK, calculate_os_fifo_thread_priority(THREAD_PRIORITY_HIGHEST, &prio));
  EXPECT_EQ(max_prio, prio);
#elif
  GTEST_SKIP() << "Testcase not implemented for this platform."
#endif  // __linux__
}
