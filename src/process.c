// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#ifdef __cplusplus
extern "C"
{
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN32 || defined __CYGWIN__
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
#include <Windows.h>
#pragma warning(pop)
#else
#include <libgen.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/join.h"
#include "rcutils/process.h"
#include "rcutils/strdup.h"

int rcutils_get_pid(void)
{
#if defined _WIN32 || defined __CYGWIN__
  return (int)GetCurrentProcessId();
#else
  return (int)getpid();
#endif
}

char * rcutils_get_executable_name(rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return NULL);

#if defined __APPLE__ || defined __FreeBSD__ || (defined __ANDROID__ && __ANDROID_API__ >= 21)
  const char * appname = getprogname();
#elif (defined __GNUC__ && !defined(__MINGW64__)) && !defined(__QNXNTO__) && !defined(__OHOS__)
  const char * appname = program_invocation_name;
#elif defined _WIN32 || defined __CYGWIN__
  char appname[MAX_PATH];
  int32_t size = GetModuleFileNameA(NULL, appname, MAX_PATH);
  if (size == 0) {
    return NULL;
  }
#elif defined __QNXNTO__ || defined __OHOS__
  extern char * __progname;
  const char * appname = __progname;
#else
#error "Unsupported OS"
#endif

  size_t applen = strlen(appname);

  // Since the above memory may be static, and the caller may want to modify
  // the argument, make and return a copy here.
  char * executable_name = allocator.allocate(applen + 1, allocator.state);
  if (NULL == executable_name) {
    return NULL;
  }

  // Get just the executable name (Unix may return the absolute path)
#if defined __APPLE__ || defined __FreeBSD__ || (defined __GNUC__ && !defined(__MINGW64__))
  // We need an intermediate copy because basename may modify its arguments
  char * intermediate = rcutils_strdup(appname, allocator);
  if (NULL == intermediate) {
    allocator.deallocate(executable_name, allocator.state);
    return NULL;
  }

  char * bname = basename(intermediate);
  size_t baselen = strlen(bname);
  memcpy(executable_name, bname, baselen);
  executable_name[baselen] = '\0';
  allocator.deallocate(intermediate, allocator.state);
#elif defined _WIN32 || defined __CYGWIN__
  errno_t err = _splitpath_s(appname, NULL, 0, NULL, 0, executable_name, applen, NULL, 0);
  if (err != 0) {
    allocator.deallocate(executable_name, allocator.state);
    return NULL;
  }
#else
#error "Unsupported OS"
#endif

  return executable_name;
}

rcutils_process_t *
rcutils_start_process(
  const rcutils_string_array_t * args,
  rcutils_allocator_t * allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(args, NULL);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, NULL);
  if (args->size < 1) {
    RCUTILS_SET_ERROR_MSG("args list is empty");
    return NULL;
  }
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    allocator, "allocator is invalid", return NULL);

  rcutils_process_t * process = allocator->zero_allocate(
    1, sizeof(rcutils_process_t), allocator->state);
  if (NULL == process) {
    return NULL;
  }
  process->allocator = *allocator;

#if defined _WIN32 || defined __CYGWIN__
  LPSTR cmd = rcutils_join(args, " ", *allocator);
  if (NULL == cmd) {
    rcutils_process_close(process);
    return NULL;
  }

  STARTUPINFO start_info = {sizeof(start_info)};
  PROCESS_INFORMATION process_info = {0};
  if (!CreateProcess(
          NULL, cmd, NULL, NULL, TRUE, 0,
          NULL, NULL, &start_info, &process_info))
  {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("Failed to create process: %lu", GetLastError());
    allocator->deallocate(cmd, &allocator->state);
    rcutils_process_close(process);
    return NULL;
  }

  allocator->deallocate(cmd, &allocator->state);

  CloseHandle(process_info.hThread);

  process->handle = process_info.hProcess;
  process->pid = process_info.dwProcessId;

  return process;
#else
  char **argv = allocator->zero_allocate(args->size + 1, sizeof(*argv), &allocator->state);
  if (NULL == argv) {
    return NULL;
  }
  memcpy(argv, args->data, args->size * sizeof(*argv));

  process->pid = fork();
  if (-1 == process->pid) {
    int error = errno;
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Failed to fork process: %d (%s)", error, strerror(error));
    allocator->deallocate(argv, &allocator->state);
    rcutils_process_close(process);
    return NULL;
  } else if (0 != process->pid) {
    allocator->deallocate(argv, &allocator->state);
    return process;
  }

  (void)execvp(argv[0], argv);

  int error = errno;
  RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
    "Failed to execute process: %d (%s)", error, strerror(error));
  allocator->deallocate(argv, &allocator->state);
  exit(127);
#endif
}

void
rcutils_process_close(rcutils_process_t * process)
{
  if (NULL == process) {
    return;
  }

  rcutils_allocator_t allocator = process->allocator;
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "allocator is invalid", return );

#if defined _WIN32 || defined __CYGWIN__
  CloseHandle(process->handle);
#endif

  allocator.deallocate(process, allocator.state);
}

rcutils_ret_t
rcutils_process_wait(const rcutils_process_t * process, int * exit_code)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(process, RCUTILS_RET_INVALID_ARGUMENT);

#if defined _WIN32 || defined __CYGWIN__
  DWORD status;

  if (WAIT_FAILED == WaitForSingleObject(process->handle, INFINITE)) {
    return RCUTILS_RET_ERROR;
  }

  if (NULL != exit_code) {
    if (!GetExitCodeProcess(process->handle, &status)) {
      return RCUTILS_RET_ERROR;
    }
    *exit_code = status;
  }
#else
  int status;

  int ret = waitpid(process->pid, &status, 0);
  if (-1 == ret) {
    int error = errno;
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
      "Failed to wait for process %d: %d (%s)", process->pid, error, strerror(error));
    return RCUTILS_RET_ERROR;
  }

  if (NULL != exit_code) {
    if (WIFSIGNALED(status)) {
      *exit_code = -WTERMSIG(status);
    } else if (WIFEXITED(status)) {
      *exit_code = WEXITSTATUS(status);
    } else if (WIFSTOPPED(status)) {
      *exit_code = -WSTOPSIG(status);
    } else {
      return RCUTILS_RET_ERROR;
    }
  }
#endif

  return RCUTILS_RET_OK;
}

#ifdef __cplusplus
}
#endif
