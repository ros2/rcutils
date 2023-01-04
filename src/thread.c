// Copyright (c) 2020 Robert Bosch GmbH
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

#include "rcutils/thread.h"

#ifdef _WIN32  // i.e., Windows platform.
// #include <windows.h>
#elif __APPLE__  // i.e., macOS platform.
// #include <pthread.h>
// #include <mach/mach_init.h>
// #include <mach/mach_port.h>
// #include <mach/mach_time.h>
// #include <mach/thread_act.h>
// #include <mach/thread_policy.h>
// #include <sys/sysctl.h>
#else  // POSIX platforms
  #include <pthread.h>
  #ifdef __QNXNTO__
    #include <sys/neutrino.h>
    #include <sys/syspage.h>
  #endif  // __QNXNTO__
#endif

#ifdef __cplusplus
extern "C"
{
#endif

rcutils_ret_t calculate_os_thread_priority(
  const int thread_priority,
  int * os_priority)
{
#ifdef _WIN32
  return RCUTILS_RET_ERROR;
#elif __APPLE__
  return RCUTILS_RET_ERROR;
#else
  if (thread_priority == THREAD_PRIORITY_HIGH) {
    *os_priority = sched_get_priority_max(SCHED_FIFO);
  } else if (thread_priority == THREAD_PRIORITY_LOW) {
    *os_priority = sched_get_priority_min(SCHED_FIFO);
  } else if (thread_priority == THREAD_PRIORITY_MEDIUM) {
    // Should be a value of 49 on standard Linux platforms, which is just below
    // the default priority of 50 for threaded interrupt handling.
    *os_priority =
      (sched_get_priority_min(SCHED_FIFO) + sched_get_priority_max(SCHED_FIFO)) / 2 - 1;
  } else {  // unhandled priority
    return RCUTILS_RET_ERROR;
  }
  return RCUTILS_RET_OK;
#endif
}

rcutils_ret_t configure_native_realtime_thread(
  unsigned long int native_handle, const int priority,
  const unsigned int cpu_bitmask)
{
  int success = 1;
#ifdef _WIN32
  return RCUTILS_RET_ERROR;
#elif __APPLE__
  return RCUTILS_RET_ERROR;
#else  // POSIX systems
  struct sched_param params;
  int policy;
  success &= (pthread_getschedparam(native_handle, &policy, &params) == 0);
  success &= (calculate_os_thread_priority(priority, &params.sched_priority) ==
    RCUTILS_RET_OK ? 1 : 0);
  success &= (pthread_setschedparam(native_handle, SCHED_FIFO, &params) == 0);

#ifdef __QNXNTO__
  // run_mask is a bit mask to set which cpu a thread runs on
  // where each bit corresponds to a cpu core
  int64_t run_mask = cpu_bitmask;

  // Function used to change thread affinity of thread associated with native_handle
  if (ThreadCtlExt(
      0, native_handle, _NTO_TCTL_RUNMASK,
      (void *)run_mask) == -1)
  {
    success &= 0;
  } else {
    success &= 1;
  }
#else
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  for (unsigned int i = 0; i < sizeof(cpu_bitmask) * 8; i++) {
    if ( (cpu_bitmask & (1 << i)) != 0) {
      CPU_SET(i, &cpuset);
    }
  }
  success &= (pthread_setaffinity_np(native_handle, sizeof(cpu_set_t), &cpuset) == 0);
#endif  // __QNXNTO__
#endif

  return success ? RCUTILS_RET_OK : RCUTILS_RET_ERROR;
}

#ifdef __cplusplus
}
#endif
