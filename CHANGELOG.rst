^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rcutils
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.6.0 (2018-11-16)
------------------
* Added rcutils_to_native_path function (`#119 <https://github.com/ros2/rcutils/issues/119>`_)
* Moved stdatomic helper to rcutils (`#126 <https://github.com/ros2/rcutils/issues/126>`_)
* Fixed warning in release build due to assert (`#124 <https://github.com/ros2/rcutils/issues/124>`_)
* Updated to avoid dynamic memory allocation during error handling (`#121 <https://github.com/ros2/rcutils/issues/121>`_)
* Added macro semicolons (`#120 <https://github.com/ros2/rcutils/issues/120>`_)
* Added LL suffix to avoid c4307 (`#118 <https://github.com/ros2/rcutils/issues/118>`_)
* Updated to use the same allocator to free allocated message (`#115 <https://github.com/ros2/rcutils/issues/115>`_)
* Renamed rcutils_serialized_message -> rcutils_char_array (`#111 <https://github.com/ros2/rcutils/issues/111>`_)
* Moved serialized_message from rmw (`#110 <https://github.com/ros2/rcutils/issues/110>`_)
* Updated to verify that the requested allocation size does not overflow. (`#109 <https://github.com/ros2/rcutils/issues/109>`_)
* Contributors: Chris Lalancette, Jacob Perron, Karsten Knese, Mikael Arguedas, Ruffin, Shane Loretz, Todd Malsbary, William Woodall

0.5.1 (2018-06-28)
------------------

* Removed redundant stat() call (`#108 <https://github.com/ros2/rcutils/pull/108>`_)

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
