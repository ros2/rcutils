^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcutils
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.5.0 (2018-06-20)
------------------
* Audited use of malloc/realloc/calloc/free to make sure it always goes through an ``rcutils_allocator_t`` (`#102 <https://github.com/ros2/rcutils/issues/102>`_)
* Added ability to include a timestamp when a console logging message happens (`#85 <https://github.com/ros2/rcutils/issues/85>`_)
* Updated to use new memory_tools from osrf_testing_tools_cpp (`#101 <https://github.com/ros2/rcutils/issues/101>`_)
* Fixed a possible bug by preventing the default logger's level from being unset (`#106 <https://github.com/ros2/rcutils/issues/106>`_)
* Updated to use launch.legacy instead of launch (now used for new launch system) (`#105 <https://github.com/ros2/rcutils/issues/105>`_)
* Fixed a memory check issue in ``split.c`` (`#104 <https://github.com/ros2/rcutils/issues/104>`_)
  * Signed-off-by: testkit <cathy.shen@intel.com>
* Added ``RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED`` to control flusing of output from the default output handler of the logging macros. (`#98 <https://github.com/ros2/rcutils/issues/98>`_)
* Can now control shared/static linking via BUILD_SHARED_LIBS (`#94 <https://github.com/ros2/rcutils/issues/94>`_)
* Addressed some MISRA C compliance issues (`#91 <https://github.com/ros2/rcutils/issues/91>`_)
* Fixed a steady time overflow issue (`#87 <https://github.com/ros2/rcutils/issues/87>`_)
* Changed rcutils_time_point_value_t type from uint64_t to int64_t (`#84 <https://github.com/ros2/rcutils/issues/84>`_)
* Fixed out-of-bounds read issue (`#83 <https://github.com/ros2/rcutils/issues/83>`_)
  * Signed-off-by: Ethan Gao <ethan.gao@linux.intel.com>
* Contributors: Dirk Thomas, Ethan Gao, Michael Carroll, Mikael Arguedas, Sagnik Basu, Shane Loretz, William Woodall, cshen, dhood, serge-nikulin
