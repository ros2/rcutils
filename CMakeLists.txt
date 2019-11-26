cmake_minimum_required(VERSION 3.5)

project(rcutils)

# Default to C11
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
endif()
# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(ament_cmake_python REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(tinydir_vendor REQUIRED)

ament_python_install_package(${PROJECT_NAME})

include_directories(include)

if(NOT WIN32)
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(WIN32)
  set(time_impl_c src/time_win32.c)
else()
  set(time_impl_c src/time_unix.c)
endif()

set(rcutils_sources
  src/allocator.c
  src/array_list.c
  src/char_array.c
  src/cmdline_parser.c
  src/error_handling.c
  src/filesystem.c
  src/find.c
  src/format_string.c
  src/get_env.c
  src/hash_map.c
  src/logging.c
  src/process.c
  src/repl_str.c
  src/snprintf.c
  src/split.c
  src/strdup.c
  src/string_array.c
  src/string_map.c
  src/time.c
  ${time_impl_c}
  src/uint8_array.c
  src/security_directory.c
)

set_source_files_properties(
  ${rcutils_sources}
  PROPERTIES language "C")

# "watch" template/inputs for changes
configure_file(
  "resource/logging_macros.h.em"
  "logging_macros.h.em.watch"
  COPYONLY)
configure_file(
  "rcutils/logging.py"
  "logging.py.watch"
  COPYONLY)
# generate header with logging macros
set(rcutils_module_path ${CMAKE_CURRENT_SOURCE_DIR})
set(python_code
  "import em"  # implicitly added ; between python statements due to CMake list
  "\
em.invoke( \
  [ \
    '-o', 'include/rcutils/logging_macros.h', \
    '-D', 'rcutils_module_path=\"${rcutils_module_path}\"', \
    '${CMAKE_CURRENT_SOURCE_DIR}/resource/logging_macros.h.em' \
  ] \
)")
string(REPLACE ";" "$<SEMICOLON>" python_code "${python_code}")
add_custom_command(OUTPUT include/rcutils/logging_macros.h
  COMMAND ${CMAKE_COMMAND} -E make_directory "include/rcutils"
  COMMAND ${PYTHON_EXECUTABLE} ARGS -c "${python_code}"
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/logging_macros.h.em.watch"
    "${CMAKE_CURRENT_BINARY_DIR}/logging.py.watch"
  COMMENT "Expanding logging_macros.h.em"
  VERBATIM
)
list(APPEND rcutils_sources
  include/rcutils/logging_macros.h)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

add_library(
  ${PROJECT_NAME}
  ${rcutils_sources})

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME} PRIVATE "RCUTILS_BUILDING_DLL")

# Needed if pthread is used for thread local storage.
if(IOS AND IOS_SDK_VERSION LESS 10.0)
  ament_export_libraries(pthread)
endif()

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

ament_target_dependencies(${PROJECT_NAME}
  "tinydir_vendor"
)

if(BUILD_TESTING)
  if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()

  find_package(ament_cmake_gmock REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_cmake_pytest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(launch_testing_ament_cmake REQUIRED)

  if(ament_cmake_cppcheck_FOUND)
    ament_cppcheck(
      TESTNAME "cppcheck_logging_macros"
      "${CMAKE_CURRENT_BINARY_DIR}/include/rcutils/logging_macros.h")
  endif()
  if(ament_cmake_cpplint_FOUND)
    ament_cpplint(
      TESTNAME "cpplint_logging_macros"
      # the generated code might contain longer lines for templated types
      MAX_LINE_LENGTH 999
      ROOT "${CMAKE_CURRENT_BINARY_DIR}/include"
      "${CMAKE_CURRENT_BINARY_DIR}/include/rcutils/logging_macros.h")
  endif()
  if(ament_cmake_uncrustify_FOUND)
    ament_uncrustify(
      TESTNAME "uncrustify_logging_macros"
      # the generated code might contain longer lines for templated types
      MAX_LINE_LENGTH 0
      "${CMAKE_CURRENT_BINARY_DIR}/include/rcutils/logging_macros.h")
  endif()

  find_package(osrf_testing_tools_cpp REQUIRED)
  get_target_property(memory_tools_test_env_vars
    osrf_testing_tools_cpp::memory_tools LIBRARY_PRELOAD_ENVIRONMENT_VARIABLE)
  get_target_property(memory_tools_is_available
    osrf_testing_tools_cpp::memory_tools LIBRARY_PRELOAD_ENVIRONMENT_IS_AVAILABLE)

  ament_add_gtest(test_logging test/test_logging.cpp)
  target_link_libraries(test_logging ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools)

  add_executable(test_logging_long_messages test/test_logging_long_messages.cpp)
  target_link_libraries(test_logging_long_messages ${PROJECT_NAME})
  add_launch_test(
    "test/test_logging_long_messages.py"
    TARGET test_logging_long_messages
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:test_logging_long_messages>"
    TIMEOUT 10
  )

  add_launch_test(
    "test/test_logging_output_format.py"
    TARGET test_logging_output_format
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:test_logging_long_messages>"
    TIMEOUT 10
  )

  ament_add_gmock(test_logging_macros test/test_logging_macros.cpp)
  target_link_libraries(test_logging_macros ${PROJECT_NAME})

  add_executable(test_logging_macros_c test/test_logging_macros.c)
  target_link_libraries(test_logging_macros_c ${PROJECT_NAME})
  ament_add_test(test_logging_macros_c
    COMMAND "$<TARGET_FILE:test_logging_macros_c>"
    GENERATE_RESULT_FOR_RETURN_CODE_ZERO)

  set(SKIP_MEMORY_TOOLS_TEST "")
  if(NOT memory_tools_is_available)
    set(SKIP_MEMORY_TOOLS_TEST "SKIP_TEST")
  endif()

  macro(rcutils_custom_add_gtest target)
    ament_add_gtest(${target} ${ARGN})
  endmacro()

  macro(rcutils_custom_add_gmock target)
    ament_add_gmock(${target} ${ARGN})
  endmacro()

  # Gtests
  rcutils_custom_add_gtest(test_allocator test/test_allocator.cpp
    ENV ${memory_tools_test_env_vars}
    ${SKIP_MEMORY_TOOLS_TEST}
  )
  if(TARGET test_allocator)
    target_link_libraries(test_allocator ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools)
  endif()

  rcutils_custom_add_gtest(test_char_array
    test/test_char_array.cpp
  )
  if(TARGET test_char_array)
    target_link_libraries(test_char_array ${PROJECT_NAME})
  endif()

  # Can't use C++ with stdatomic_helper.h
  add_executable(test_atomics_executable
    test/test_atomics.c
  )
  set_target_properties(test_atomics_executable
    PROPERTIES
      LANGUAGE C
  )
  target_link_libraries(test_atomics_executable ${PROJECT_NAME})
  add_test(NAME test_atomics COMMAND test_atomics_executable)

  rcutils_custom_add_gmock(test_error_handling test/test_error_handling.cpp
    # Append the directory of librcutils so it is found at test time.
    APPEND_LIBRARY_DIRS "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    ENV ${memory_tools_test_env_vars}
  )
  if(TARGET test_error_handling)
    target_link_libraries(test_error_handling ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools)
  endif()

  rcutils_custom_add_gmock(test_error_handling_helpers test/test_error_handling_helpers.cpp
    # Append the directory of librcutils so it is found at test time.
    APPEND_LIBRARY_DIRS "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    ENV ${memory_tools_test_env_vars}
  )
  if(TARGET test_error_handling_helpers)
    target_link_libraries(test_error_handling_helpers osrf_testing_tools_cpp::memory_tools)
  endif()

  rcutils_custom_add_gtest(test_split
    test/test_split.cpp
  )
  if(TARGET test_split)
    target_link_libraries(test_split ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_find
    test/test_find.cpp
  )
  if(TARGET test_find)
    target_link_libraries(test_find ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_string_array
    test/test_string_array.cpp
  )
  if(TARGET test_string_array)
    target_link_libraries(test_string_array ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_get_env test/test_get_env.cpp
    ENV
      EMPTY_TEST=
      NORMAL_TEST=foo
    APPEND_LIBRARY_DIRS "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  )
  if(TARGET test_get_env)
    target_link_libraries(test_get_env ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_filesystem
    test/test_filesystem.cpp
  )
  if(TARGET test_filesystem)
    ament_target_dependencies(test_filesystem "osrf_testing_tools_cpp")
    target_link_libraries(test_filesystem ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_strdup
    test/test_strdup.cpp
  )
  if(TARGET test_strdup)
    target_link_libraries(test_strdup ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_format_string
    test/test_format_string.cpp
  )
  if(TARGET test_format_string)
    target_link_libraries(test_format_string ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_string_map
    test/test_string_map.cpp
  )
  if(TARGET test_string_map)
    ament_target_dependencies(test_string_map "osrf_testing_tools_cpp")
    target_link_libraries(test_string_map ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_isalnum_no_locale
    test/test_isalnum_no_locale.cpp
  )
  if(TARGET test_isalnum_no_locale)
    target_link_libraries(test_isalnum_no_locale ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_repl_str
    test/test_repl_str.cpp
  )
  if(TARGET test_repl_str)
    target_link_libraries(test_repl_str ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_time
    test/test_time.cpp
    ENV ${memory_tools_test_env_vars})
  if(TARGET test_time)
    target_link_libraries(test_time ${PROJECT_NAME} osrf_testing_tools_cpp::memory_tools)
  endif()

  rcutils_custom_add_gtest(test_snprintf
    test/test_snprintf.cpp
  )
  if(TARGET test_snprintf)
    target_link_libraries(test_snprintf ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_uint8_array
    test/test_uint8_array.cpp
  )
  if(TARGET test_uint8_array)
    target_link_libraries(test_uint8_array ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_array_list
    test/test_array_list.cpp
  )
  if(TARGET test_array_list)
    target_link_libraries(test_array_list ${PROJECT_NAME})
  endif()

  rcutils_custom_add_gtest(test_hash_map
    test/test_hash_map.cpp
  )
  if(TARGET test_hash_map)
    target_link_libraries(test_hash_map ${PROJECT_NAME})
  endif()
endif()

ament_export_dependencies(ament_cmake)
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_package()

install(
  DIRECTORY include/ ${CMAKE_CURRENT_BINARY_DIR}/include/
  DESTINATION include)
