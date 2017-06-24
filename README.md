# rcutils: ROS 2 C Utilities data structures

`rcutils` is a C API consisting of macros, functions, and data structures used through out the ROS 2 code base.

The API is a combination of parts:

- Allocator concept, used to inject the allocating and deallocating methods into a function or type.
  - rcutils_allocator_t
  - rcutils/allocator.h
- Command line interface utilities:
  - rcutils/cmdline_parser.h
- Utilities for setting error states (error message, file, and line number) like `strerror` for `errno`:
  - rcutils/error_handling.h
- Some basic filesystem utilities like checking for path existence, getting the cwd, etc...:
  - rcutils/filesystem.h
- A C string find method:
  - rcutils_find()
  - rcutils_find_last()
  - rcutils/find.h
- A convenient string formatting function, which takes a custom allocator:
  - rcutils_format_string()
  - rcutils/format_string.h
- A function to get an environment variable's value:
  - rcutils_get_env()
  - rcutils/get_env.h
- Extensible logging macros:
  - Some examples (not exhaustive):
    - RCUTILS_LOG_DEBUG()
    - RCUTILS_LOG_INFO_NAMED()
    - RCUTILS_LOG_WARN_ONCE()
    - RCUTILS_LOG_ERROR_SKIPFIRST_NAMED()
  - rcutils/logging_macros.h
  - rcutils/logging.h
- A string replacement function which takes an allocator, based on http://creativeandcritical.net/str-replace-c:
  - rcutils_repl_str()
  - rcutils/repl_str.h
- String splitting functions which take a custom allocator:
  - rcutils_split()
  - rcutils_split_last()
  - rcutils/split.h
- A version of `strdup` which takes an allocator:
  - rcutils_strdup()
  - rcutils/strdup.h
- Portable implementations of "get system time" and "get steady time":
  - rcutils_system_time_now()
  - rcutils_steady_time_now()
  - rcutils/time.h
- Some useful data structures:
  - A "string array" data structure (analogous to `std::vector<std::string>`):
    - rcutils_string_array_t
    - rcutils/types/string_array.h
  - A "string-string map" data structure (analogous to `std::map<std::string, std::string>`)
    - rcutils_string_map_t
    - rcutils/types/string_map.h
- Macros for controlling symbol visibility and linkage for this library:
  - rcutils/visibility_control.h
