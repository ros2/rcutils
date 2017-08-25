# Copyright 2017 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from collections import OrderedDict

severities = ('DEBUG', 'INFO', 'WARN', 'ERROR', 'FATAL')
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
    'Log calls are being ignored if the last logged message is not longer ago than the specified '
    'duration.']


class Feature:
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
