cmake_minimum_required(VERSION 3.5)

project(rcutils)

find_package(ament_cmake REQUIRED)

include_directories(include)

if(NOT WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  # The warning flags are added per target, because otherwise the local
  # gmock library that is built will have compiler warnings.
  set(rcutils_extra_warning_flags "-Wall -Wextra -Wpedantic")
endif()

if(WIN32)
  set(time_impl_c src/time_win32.c)
else()
  set(time_impl_c src/time_unix.c)
endif()

set(rcutils_sources
  src/allocator.c
  src/cmdline_parser.c
  src/concat.c
  src/error_handling.c
  src/filesystem.c
  src/find.c
  src/format_string.c
  src/get_env.c
  src/repl_str.c
  src/split.c
  src/strdup.c
  src/string_array.c
  src/string_map.c
  ${time_impl_c}
)
set_source_files_properties(
  ${rcutils_sources}
  PROPERTIES language "C")

add_library(
  ${PROJECT_NAME}
  SHARED
  ${rcutils_sources})
if(rcutils_extra_warning_flags)
  set_target_properties(${PROJECT_NAME}
    PROPERTIES COMPILE_FLAGS ${rcutils_extra_warning_flags})
endif()

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

if(BUILD_TESTING)
  if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()

  find_package(ament_cmake_gmock REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  set(extra_test_libraries)
  set(extra_test_env)
  set(extra_lib_dirs)

  # This subdirectory extends both extra_test_libraries, extra_test_env, and extra_lib_dirs.
  add_subdirectory(test/memory_tools)

  set(SKIP_TEST_IF_WIN32 "")
  if(WIN32)  # (memory tools doesn't do anything on Windows)
    set(SKIP_TEST_IF_WIN32 "SKIP_TEST")
  endif()

  macro(rcutils_custom_add_gtest target)
    ament_add_gtest(${target} ${ARGN})
    if(TARGET ${target} AND rcutils_extra_warning_flags)
      set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${rcutils_extra_warning_flags})
    endif()
  endmacro()

  macro(rcutils_custom_add_gmock target)
    ament_add_gmock(${target} ${ARGN})
    if(TARGET ${target} AND rcutils_extra_warning_flags)
      set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${rcutils_extra_warning_flags})
    endif()
  endmacro()

  # Gtests
  rcutils_custom_add_gtest(test_allocator test/test_allocator.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    ${SKIP_TEST_IF_WIN32}
  )
  if(TARGET test_allocator)
    target_link_libraries(test_allocator ${PROJECT_NAME} ${extra_test_libraries})
  endif()

  rcutils_custom_add_gmock(test_error_handling test/test_error_handling.cpp
    # Append the directory of librcutils so it is found at test time.
    APPEND_LIBRARY_DIRS "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  )
  if(TARGET test_error_handling)
    target_link_libraries(test_error_handling ${PROJECT_NAME})
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

  rcutils_custom_add_gtest(test_concat
    test/test_concat.cpp
  )
  if(TARGET test_concat)
    target_link_libraries(test_concat ${PROJECT_NAME})
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
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs})
  if(TARGET test_time)
    target_link_libraries(test_time ${PROJECT_NAME} ${extra_test_libraries})
  endif()
endif()

ament_export_dependencies(ament_cmake)
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_package()

install(
  DIRECTORY include/
  DESTINATION include)
