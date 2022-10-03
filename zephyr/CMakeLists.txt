set(CMAKE_C_STANDARD 11)

option(RCUTILS_NO_THREAD_SUPPORT "Disable thread support." OFF)
option(RCUTILS_NO_FILESYSTEM "Disable filesystem usage." ON)
option(RCUTILS_AVOID_DYNAMIC_ALLOCATION "Disable dynamic allocations." OFF)
option(RCUTILS_MICROROS "Flag for building micro-ROS." ON)

configure_file(
  ${ZEPHYR_CURRENT_MODULE_DIR}/include/rcutils/configuration_flags.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/rcutils/configuration_flags.h
)

set(GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/rcutils)
set(LOGGING_MACROS_H_EM ${ZEPHYR_CURRENT_MODULE_DIR}/resource/logging_macros.h.em)
set(LOGGING_MACROS_H ${GENERATED_INCLUDE_DIR}/logging_macros.h)
set(RCUTILS_MODULE_PATH rcutils_module_path="${ZEPHYR_CURRENT_MODULE_DIR}")
set(EMPY ${ZEPHYR_CURRENT_MODULE_DIR}/scripts/empy)
add_custom_command(OUTPUT ${LOGGING_MACROS_H}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_INCLUDE_DIR}
  COMMAND ${PYTHON_EXECUTABLE} ${EMPY}
  -o ${LOGGING_MACROS_H}
  -D ${RCUTILS_MODULE_PATH}
  ${LOGGING_MACROS_H_EM}
  DEPENDS
    ${LOGGING_MACROS_H_EM}
  COMMENT "Expanding logging_macros.h.em"
  VERBATIM
)

zephyr_library()
zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
zephyr_include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

if(CONFIG_POSIX_API)
  zephyr_library_include_directories(${ZEPHYR_BASE}/include/zephyr/posix)
endif()

zephyr_library_sources(
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/allocator.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/array_list.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/char_array.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/cmdline_parser.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/env_zephyr.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/error_handling.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/filesystem.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/find.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/format_string.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/hash_map.c
  #${ZEPHYR_CURRENT_MODULE_DIR}/src/logging.c
  #${ZEPHYR_CURRENT_MODULE_DIR}/src/process.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/qsort.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/repl_str.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/shared_library.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/snprintf.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/split.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/strcasecmp.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/strdup.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/strerror.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/string_array.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/string_map.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/testing/fault_injection.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/time.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/time_zephyr.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/uint8_array.c
  ${ZEPHYR_CURRENT_MODULE_DIR}/src/atomic_64bits.c
  ${LOGGING_MACROS_H}
)

zephyr_library_compile_definitions(_DEFAULT_SOURCE)

add_custom_target(ros2_rutils
  DEPENDS ${LOGGING_MACROS_H}
)
