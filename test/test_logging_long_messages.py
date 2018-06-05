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

import os

from launch.legacy import LaunchDescriptor
from launch.legacy.exit_handler import ignore_exit_handler
from launch.legacy.launcher import DefaultLauncher
from launch.legacy.output_handler import ConsoleOutput
from launch_testing import create_handler


def test_logging_long_messages():
    launch_descriptor = LaunchDescriptor()

    output_file = os.path.join(os.path.dirname(__file__), 'test_logging_long_messages')
    handler = create_handler(
        'test_logging_long_messages', launch_descriptor, output_file)
    assert handler, 'Cannot find appropriate handler for %s' % output_file

    # Set the output format to a "verbose" format that is expected by the executable output
    os.environ['RCUTILS_CONSOLE_OUTPUT_FORMAT'] = \
        '[{severity}] [{name}]: {message} ({function_name}() at {file_name}:{line_number})'
    executable = os.path.join(os.getcwd(), 'test_logging_long_messages')
    if os.name == 'nt':
        executable += '.exe'
    launch_descriptor.add_process(
        cmd=[executable],
        name='test_logging_long_messages',
        exit_handler=ignore_exit_handler,  # The process will automatically exit after printing.
        output_handlers=[ConsoleOutput(), handler],
    )

    launcher = DefaultLauncher()
    launcher.add_launch_descriptor(launch_descriptor)
    rc = launcher.launch()

    assert rc == 0, \
        "The launch file failed with exit code '" + str(rc) + "'"

    handler.check()


if __name__ == '__main__':
    test_logging_long_messages()
