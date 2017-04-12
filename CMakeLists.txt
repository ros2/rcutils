cmake_minimum_required(VERSION 3.5)

project(c_utilities)

find_package(ament_cmake REQUIRED)

include_directories(include)

if(NOT WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(utilities_sources
  src/allocator.c
  src/cmdline_parser.c
  src/concat.c
  src/error_handling.c
  src/filesystem.c
  src/find.c
  src/get_env.c
  src/split.c
  src/string_array.c)
set_source_files_properties(
  ${utilities_sources}
  PROPERTIES language "C")

add_library(
  c_utilities
  SHARED
  ${utilities_sources})

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(c_utilities PRIVATE "C_UTILITIES_BUILDING_DLL")

# Needed if pthread is used for thread local storage.
if(IOS AND IOS_SDK_VERSION LESS 10.0)
  ament_export_libraries(pthread)
endif()

install(TARGETS c_utilities
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

if(BUILD_TESTING)
  if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()

  find_package(ament_cmake_gmock REQUIRED)
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

  # Gtests
  ament_add_gmock(test_allocator test/test_allocator.cpp
    ENV ${extra_test_env}
    APPEND_LIBRARY_DIRS ${extra_lib_dirs}
    ${SKIP_TEST_IF_WIN32}
  )
  if(TARGET test_allocator)
    target_link_libraries(test_allocator c_utilities ${extra_test_libraries})
  endif()

  ament_add_gmock(test_error_handling test/test_error_handling.cpp
    # Append the directory of libc_utilities so it is found at test time.
    APPEND_LIBRARY_DIRS "$<TARGET_FILE_DIR:c_utilities>"
  )
  if(TARGET test_error_handling)
    target_link_libraries(test_error_handling ${PROJECT_NAME})
  endif()

  ament_add_gmock(test_split
    test/test_split.cpp
  )
  if(TARGET test_split)
    target_link_libraries(test_split c_utilities)
  endif()

  ament_add_gmock(test_find
    test/test_find.cpp
  )
  if(TARGET test_find)
    target_link_libraries(test_find c_utilities)
  endif()

  ament_add_gmock(test_concat
    test/test_concat.cpp
  )
  if(TARGET test_concat)
    target_link_libraries(test_concat c_utilities)
  endif()

  ament_add_gmock(test_string_array
    test/test_string_array.cpp
  )
  if(TARGET test_string_array)
    target_link_libraries(test_string_array c_utilities)
  endif()

  ament_add_gmock(test_get_env test/test_get_env.cpp
    ENV
      EMPTY_TEST=
      NORMAL_TEST=foo
    APPEND_LIBRARY_DIRS "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  )
  if(TARGET test_get_env)
    target_link_libraries(test_get_env ${PROJECT_NAME})
    if(UNIX AND NOT APPLE)
      target_link_libraries(test_get_env pthread)
    endif()
    if(NOT WIN32)
      set_target_properties(test_get_env PROPERTIES COMPILE_FLAGS "-std=c++11")
    endif()
  endif()

  ament_add_gmock(test_filesystem
    test/test_filesystem.cpp
  )
  if(TARGET test_filesystem)
    target_link_libraries(test_filesystem c_utilities)
  endif()

endif()

ament_export_dependencies(ament_cmake)
ament_export_include_directories(include)
ament_export_libraries(c_utilities)
ament_package()

install(
  DIRECTORY include/
  DESTINATION include)
