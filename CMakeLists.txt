cmake_minimum_required(VERSION 3.5)

project(c_utilities)

find_package(ament_cmake REQUIRED)

include_directories(include)

set(utilities_sources
  src/cmdline_parser.c
  src/concat.c
  src/find.c
  src/split.c
  src/string_array.c)
set_source_files_properties(
  utilities_sources
  PROPERTIES language "C")

add_library(
  c_utilities
  SHARED
  ${utilities_sources})

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(c_utilities PRIVATE "C_UTILITIES_BUILDING_DLL")

install(TARGETS c_utilities
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

if(BUILD_TESTING)
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    add_compile_options(-Wall -Wextra -Wpedantic)
  endif()

  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  # Gtests
  ament_add_gtest(test_split
    test/test_split.cpp
  )
  if(TARGET test_split)
    target_include_directories(test_split PUBLIC
      ${c_utilities_INCLUDE_DIRS}
    )
    target_link_libraries(test_split c_utilities)
  endif()
endif()

ament_export_dependencies(ament_cmake)
ament_export_include_directories(include)
ament_export_libraries(c_utilities)
ament_package()

install(
  DIRECTORY include/
  DESTINATION include)
