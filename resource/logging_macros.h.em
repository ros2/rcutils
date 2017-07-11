// generated from rcutils/resource/logging_macros.h.em

// Copyright 2017 Open Source Robotics Foundation, Inc.
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

/*! \file */

#ifndef RCUTILS__LOGGING_MACROS_H_
#define RCUTILS__LOGGING_MACROS_H_

#include "rcutils/logging.h"

#include <stdio.h>
#include <stdlib.h>

#if __cplusplus
extern "C"
{
#endif

/**
 * \def RCUTILS_LOG_MIN_SEVERITY
 * Define RCUTILS_LOG_MIN_SEVERITY=RCUTILS_LOG_SEVERITY_[DEBUG|INFO|WARN|ERROR|FATAL]
 * in your build options to compile out anything below that severity.
 */
#ifndef RCUTILS_LOG_MIN_SEVERITY
#define RCUTILS_LOG_MIN_SEVERITY RCUTILS_LOG_SEVERITY_DEBUG
#endif

// Provide the compiler with branch prediction information
#ifndef WIN32
/**
 * \def RCUTILS_LIKELY
 * Instruct the compiler to optimize for the case where the argument equals 1.
 */
# define RCUTILS_LIKELY(x) __builtin_expect((x), 1)
/**
 * \def RCUTILS_UNLIKELY
 * Instruct the compiler to optimize for the case where the argument equals 0.
 */
# define RCUTILS_UNLIKELY(x) __builtin_expect((x), 0)
#else
/**
 * \def RCUTILS_LIKELY
 * No op since Windows doesn't support providing branch prediction information.
 */
# define RCUTILS_LIKELY(x) (x)
/**
 * \def RCUTILS_UNLIKELY
 * No op since Windows doesn't support providing branch prediction information.
 */
# define RCUTILS_UNLIKELY(x) (x)
#endif

/**
 * \def RCUTILS_LOGGING_AUTOINIT
 * \brief Initialize the rcl logging library.
 * Usually it is unnecessary to call the macro directly.
 * All logging macros ensure that this has been called once.
 */
#define RCUTILS_LOGGING_AUTOINIT \
  if (RCUTILS_UNLIKELY(!g_rcutils_logging_initialized)) { \
    rcutils_logging_initialize(); \
  }

/**
 * \def RCUTILS_LOG_COND_NAMED
 * The logging macro all other logging macros call directly or indirectly.
 * \param severity The severity level
 * \param condition_before The condition macro(s) inserted before the log call
 * \param condition_after The condition macro(s) inserted after the log call
 * \param name The name of the logger
 * \param ... The format string, followed by the variable arguments for the format string
 */
#define RCUTILS_LOG_COND_NAMED(severity, condition_before, condition_after, name, ...) \
  { \
    RCUTILS_LOGGING_AUTOINIT \
    static rcutils_log_location_t __rcutils_logging_location = {__func__, __FILE__, __LINE__}; \
    condition_before \
    if (severity >= g_rcutils_logging_severity_threshold) { \
      rcutils_log(&__rcutils_logging_location, severity, name, __VA_ARGS__); \
    } \
    condition_after \
  }

///@@{
/**
 * \def RCUTILS_LOG_CONDITION_EMPTY
 * An empty macro which can be used as a placeholder for `condition_before`
 * and `condition_after` which doesn't affect the logging call.
 */
#define RCUTILS_LOG_CONDITION_EMPTY
///@@}

/** @@name Macros for the `once` condition which ignores all subsequent log
 * calls except the first one.
 */
///@@{
/**
 * \def RCUTILS_LOG_CONDITION_ONCE_BEFORE
 * A macro initializing and checking the `once` condition.
 */
#define RCUTILS_LOG_CONDITION_ONCE_BEFORE \
  { \
    static int __rcutils_logging_once = 0; \
    if (RCUTILS_UNLIKELY(0 == __rcutils_logging_once)) { \
      __rcutils_logging_once = 1;
/**
 * \def RCUTILS_LOG_CONDITION_ONCE_AFTER
 * A macro finalizing the `once` condition.
 */
#define RCUTILS_LOG_CONDITION_ONCE_AFTER } \
  }
///@@}

/** @@name Macros for the `expression` condition which ignores the log calls
 * when the expression evaluates to false.
 */
///@@{
/**
 * \def RCUTILS_LOG_CONDITION_EXPRESSION_BEFORE
 * A macro checking the `expression` condition.
 */
#define RCUTILS_LOG_CONDITION_EXPRESSION_BEFORE(expression) \
  if (expression) {
/**
 * \def RCUTILS_LOG_CONDITION_EXPRESSION_AFTER
 * A macro finalizing the `expression` condition.
 */
#define RCUTILS_LOG_CONDITION_EXPRESSION_AFTER }
///@@}

/** @@name Macros for the `function` condition which ignores the log calls
 * when the function returns false.
 */
///@@{
/// The filter function signature.
/**
 * \return true to log the message, false to ignore the message
 */
typedef bool (* RclLogFilter)();
/**
 * \def RCUTILS_LOG_CONDITION_FUNCTION_BEFORE
 * A macro checking the `function` condition.
 */
#define RCUTILS_LOG_CONDITION_FUNCTION_BEFORE(function) \
  if ((*function)()) {
/**
 * \def RCUTILS_LOG_CONDITION_FUNCTION_AFTER
 * A macro finalizing the `function` condition.
 */
#define RCUTILS_LOG_CONDITION_FUNCTION_AFTER }
///@@}

/** @@name Macros for the `skipfirst` condition which ignores the first log
 * call but processes all subsequent calls.
 */
///@@{
/**
 * \def RCUTILS_LOG_CONDITION_SKIPFIRST_BEFORE
 * A macro initializing and checking the `skipfirst` condition.
 */
#define RCUTILS_LOG_CONDITION_SKIPFIRST_BEFORE \
  { \
    static bool __rcutils_logging_first = true; \
    if (RCUTILS_UNLIKELY(true == __rcutils_logging_first)) { \
      __rcutils_logging_first = false; \
    } else {
/**
 * \def RCUTILS_LOG_CONDITION_SKIPFIRST_AFTER
 * A macro finalizing the `skipfirst` condition.
 */
#define RCUTILS_LOG_CONDITION_SKIPFIRST_AFTER } \
  }
///@@}

/** @@name Macros for the `throttle` condition which ignores log calls if the
 * last logged message is not longer ago than the specified duration.
 */
///@@{
/**
 * \def RCUTILS_LOG_CONDITION_THROTTLE_BEFORE
 * A macro initializing and checking the `throttle` condition.
 */
#define RCUTILS_LOG_CONDITION_THROTTLE_BEFORE(time_source_type, duration) { \
    static rcutils_duration_value_t __rcutils_logging_duration = RCUTILS_MS_TO_NS((rcutils_duration_value_t)duration); \
    static rcutils_time_point_value_t __rcutils_logging_last_logged = 0; \
    rcutils_time_point_value_t __rcutils_logging_now = 0; \
    bool __rcutils_logging_condition = true; \
    if (rcutils_steady_time_now(&__rcutils_logging_now) != RCUTILS_RET_OK) { \
      rcutils_log( \
        &__rcutils_logging_location, RCUTILS_LOG_SEVERITY_ERROR, "", \
        "%s() at %s:%d getting current steady time failed\n", \
        __func__, __FILE__, __LINE__); \
    } else { \
      __rcutils_logging_condition = __rcutils_logging_now >= __rcutils_logging_last_logged + __rcutils_logging_duration; \
    } \
 \
    if (RCUTILS_LIKELY(__rcutils_logging_condition)) { \
      __rcutils_logging_last_logged = __rcutils_logging_now;

/**
 * \def RCUTILS_LOG_CONDITION_THROTTLE_AFTER
 * A macro finalizing the `throttle` condition.
 */
#define RCUTILS_LOG_CONDITION_THROTTLE_AFTER } \
  }
///@@}

@{
severities = ('DEBUG', 'INFO', 'WARN', 'ERROR', 'FATAL')

from collections import OrderedDict
default_args = OrderedDict((
  ('condition_before', 'RCUTILS_LOG_CONDITION_EMPTY'),
  ('condition_after', 'RCUTILS_LOG_CONDITION_EMPTY'),
  ('name', '""'),
))

name_params = OrderedDict((
  ('name', 'The name of the logger'),
))
name_args = {'name': 'name'}

once_args = {
  'condition_before': 'RCUTILS_LOG_CONDITION_ONCE_BEFORE',
  'condition_after': 'RCUTILS_LOG_CONDITION_ONCE_AFTER'}
name_doc_lines = [
  'All subsequent log calls except the first one are being ignored.']

expression_params = OrderedDict((
  ('expression', 'The expression determining if the message should be logged'),
))
expression_args = {
  'condition_before': 'RCUTILS_LOG_CONDITION_EXPRESSION_BEFORE(expression)',
  'condition_after': 'RCUTILS_LOG_CONDITION_EXPRESSION_AFTER'}
expression_doc_lines = [
  'Log calls are being ignored when the expression evaluates to false.']

function_params = OrderedDict((
  ('function', 'The functions return value determines if the message should be logged'),
))
function_args = {
  'condition_before': 'RCUTILS_LOG_CONDITION_FUNCTION_BEFORE(function)',
  'condition_after': 'RCUTILS_LOG_CONDITION_FUNCTION_AFTER'}
function_doc_lines = [
  'Log calls are being ignored when the function returns false.']

skipfirst_args = {
  'condition_before': 'RCUTILS_LOG_CONDITION_SKIPFIRST_BEFORE',
  'condition_after': 'RCUTILS_LOG_CONDITION_SKIPFIRST_AFTER'}
skipfirst_doc_lines = [
  'The first log call is being ignored but all subsequent calls are being processed.']

throttle_params = OrderedDict((
  ('time_source_type', 'The time source type of the time to be used'),
  ('duration', 'The duration of the throttle interval'),
))
throttle_args = {
  'condition_before': 'RCUTILS_LOG_CONDITION_THROTTLE_BEFORE(time_source_type, duration)',
  'condition_after': 'RCUTILS_LOG_CONDITION_THROTTLE_AFTER'}
throttle_doc_lines = [
  'Log calls are being ignored if the last logged message is not longer ago than the specified duration.']

class Feature(object):
  __slots__ = ('params', 'args', 'doc_lines')
  def __init__(self, *, params=None, args=None, doc_lines=None):
    if params is None:
      params = {}
    self.params = params
    if args is None:
      args = {}
    self.args = args
    if doc_lines is None:
      doc_lines = []
    self.doc_lines = doc_lines

feature_combinations = OrderedDict((
  ('', Feature()),
  ('_NAMED', Feature(
    params=name_params,
    args=name_args)),
  ('_ONCE', Feature(
    params=None,
    args=once_args,
    doc_lines=name_doc_lines)),
  ('_ONCE_NAMED', Feature(
    params=name_params,
    args={**once_args, **name_args},
    doc_lines=name_doc_lines)),
  ('_EXPRESSION', Feature(
    params=expression_params,
    args=expression_args,
    doc_lines=expression_doc_lines)),
  ('_EXPRESSION_NAMED', Feature(
    params=OrderedDict((*expression_params.items(), *name_params.items())),
    args={**expression_args, **name_args},
    doc_lines=expression_doc_lines + name_doc_lines)),
  ('_FUNCTION', Feature(
    params=function_params,
    args=function_args,
    doc_lines=function_doc_lines)),
  ('_FUNCTION_NAMED', Feature(
    params=OrderedDict((*function_params.items(), *name_params.items())),
    args={**function_args, **name_args},
    doc_lines=function_doc_lines + name_doc_lines)),
  ('_SKIPFIRST', Feature(
    params=None,
    args=skipfirst_args,
    doc_lines=skipfirst_doc_lines)),
  ('_SKIPFIRST_NAMED', Feature(
    params=name_params,
    args={**skipfirst_args, **name_args},
    doc_lines=skipfirst_doc_lines)),
  ('_THROTTLE', Feature(
    params=throttle_params,
    args=throttle_args,
    doc_lines=throttle_doc_lines)),
  ('_SKIPFIRST_THROTTLE', Feature(
    params=throttle_params,
    args={
      'condition_before': ' '.join([
        throttle_args['condition_before'],
        skipfirst_args['condition_before']]),
      'condition_after': ' '.join([
        throttle_args['condition_after'], skipfirst_args['condition_after']]),
    },
    doc_lines=skipfirst_doc_lines + throttle_doc_lines)),
  ('_THROTTLE_NAMED', Feature(
    params=OrderedDict((*throttle_params.items(), *name_params.items())),
    args={**throttle_args, **name_args},
    doc_lines=throttle_doc_lines)),
  ('_SKIPFIRST_THROTTLE_NAMED', Feature(
    params=OrderedDict((*throttle_params.items(), *name_params.items())),
    args={
      **{
        'condition_before': ' '.join([
          throttle_args['condition_before'],
          skipfirst_args['condition_before']]),
        'condition_after': ' '.join([
          throttle_args['condition_after'],
          skipfirst_args['condition_after']]),
      }, **name_args
    },
    doc_lines=skipfirst_doc_lines + throttle_doc_lines)),
))

def get_macro_parameters(suffix):
  return ''.join([p + ', ' for p in feature_combinations[suffix].params.keys()])

def get_macro_arguments(suffix):
  args = []
  for k, default_value in default_args.items():
    value = feature_combinations[suffix].args.get(k, default_value)
    args.append(value)
  return ''.join([a + ', ' for a in args])
}@
@[for severity in severities]@
/** @@name Logging macros for severity @(severity).
 */
///@@{
#if (RCUTILS_LOG_MIN_SEVERITY > RCUTILS_LOG_SEVERITY_@(severity))
// empty logging macros for severity @(severity) when being disabled at compile time
@[ for suffix in feature_combinations]@
/// Empty logging macro due to the preprocessor definition of RCUTILS_LOG_MIN_SEVERITY.
# define RCUTILS_LOG_@(severity)@(suffix)(@(get_macro_parameters(suffix))format, ...)
@[ end for]@

#else
@[ for suffix in feature_combinations]@
/**
 * \def RCUTILS_LOG_@(severity)@(suffix)
 * Log a message with severity @(severity)@
@[ if feature_combinations[suffix].doc_lines]@
 with the following conditions:
@[ else]@
.
@[ end if]@
@[ for doc_line in feature_combinations[suffix].doc_lines]@
 * @(doc_line)
@[ end for]@
@[ for param_name, doc_line in feature_combinations[suffix].params.items()]@
 * \param @(param_name) @(doc_line)
@[ end for]@
 * \param ... The format string, followed by the variable arguments for the format string
 */
# define RCUTILS_LOG_@(severity)@(suffix)(@(get_macro_parameters(suffix))...) \
  RCUTILS_LOG_COND_NAMED( \
    RCUTILS_LOG_SEVERITY_@(severity), \
    @(get_macro_arguments(suffix))\
    __VA_ARGS__)
@[ end for]@
#endif
///@@}

@[end for]@
#if __cplusplus
}
#endif

#endif  // RCUTILS__LOGGING_MACROS_H_
