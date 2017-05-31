// Copyright 2015 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include <inttypes.h>

#include <chrono>
#include <thread>

#include "rcutils/error_handling.h"
#include "rcutils/time.h"

#include "./memory_tools/memory_tools.hpp"

class TestTimeFixture : public ::testing::Test
{
public:
  void SetUp()
  {
    set_on_unexpected_malloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED MALLOC";});
    set_on_unexpected_realloc_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED REALLOC";});
    set_on_unexpected_free_callback([]() {ASSERT_FALSE(true) << "UNEXPECTED FREE";});
    start_memory_checking();
  }

  void TearDown()
  {
    assert_no_malloc_end();
    assert_no_realloc_end();
    assert_no_free_end();
    stop_memory_checking();
    set_on_unexpected_malloc_callback(nullptr);
    set_on_unexpected_realloc_callback(nullptr);
    set_on_unexpected_free_callback(nullptr);
  }
};

// Tests the rcutils_system_time_now() function.
TEST_F(TestTimeFixture, test_rcutils_system_time_now) {
  assert_no_realloc_begin();
  rcutils_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcutils_system_time_now(nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_INVALID_ARGUMENT) << rcutils_get_error_string_safe();
  rcutils_reset_error();
  assert_no_malloc_begin();
  assert_no_free_begin();
  // Check for normal operation (not allowed to alloc).
  rcutils_time_point_value_t now = 0;
  ret = rcutils_system_time_now(&now);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_NE(now, 0u);
  // Compare to std::chrono::system_clock time (within a second).
  now = 0;
  ret = rcutils_system_time_now(&now);
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = now - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCUTILS_MS_TO_NS(k_tolerance_ms)) << "system_clock differs";
  }
}

// Tests the rcutils_steady_time_now() function.
TEST_F(TestTimeFixture, test_rcutils_steady_time_now) {
  assert_no_realloc_begin();
  rcutils_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcutils_steady_time_now(nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_INVALID_ARGUMENT) << rcutils_get_error_string_safe();
  rcutils_reset_error();
  assert_no_malloc_begin();
  assert_no_free_begin();
  // Check for normal operation (not allowed to alloc).
  rcutils_time_point_value_t now = 0;
  ret = rcutils_steady_time_now(&now);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_NE(now, 0u);
  // Compare to std::chrono::steady_clock difference of two times (within a second).
  now = 0;
  ret = rcutils_steady_time_now(&now);
  std::chrono::steady_clock::time_point now_sc = std::chrono::steady_clock::now();
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  // Wait for a little while.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // Then take a new timestamp with each and compare.
  rcutils_time_point_value_t later;
  ret = rcutils_steady_time_now(&later);
  std::chrono::steady_clock::time_point later_sc = std::chrono::steady_clock::now();
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  int64_t steady_diff = later - now;
  int64_t sc_diff =
    std::chrono::duration_cast<std::chrono::nanoseconds>(later_sc - now_sc).count();
  const int k_tolerance_ms = 1;
  EXPECT_LE(
    llabs(steady_diff - sc_diff), RCUTILS_MS_TO_NS(k_tolerance_ms)) << "steady_clock differs";
}

// Tests the rcutils_set_ros_time_override() function.
TEST_F(TestTimeFixture, test_rcutils_ros_time_set_override) {
  rcutils_time_source_t * ros_time_source = rcutils_get_default_ros_time_source();
  assert_no_realloc_begin();
  rcutils_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcutils_set_ros_time_override(nullptr, 0);
  EXPECT_EQ(ret, RCUTILS_RET_INVALID_ARGUMENT) << rcutils_get_error_string_safe();
  rcutils_reset_error();
  bool result;
  ret = rcutils_is_enabled_ros_time_override(nullptr, &result);
  EXPECT_EQ(ret, RCUTILS_RET_INVALID_ARGUMENT) << rcutils_get_error_string_safe();
  rcutils_reset_error();
  ret = rcutils_is_enabled_ros_time_override(ros_time_source, nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_INVALID_ARGUMENT) << rcutils_get_error_string_safe();
  rcutils_reset_error();
  ret = rcutils_is_enabled_ros_time_override(nullptr, nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_INVALID_ARGUMENT) << rcutils_get_error_string_safe();
  rcutils_reset_error();
  rcutils_time_point_t query_now;
  bool is_enabled;
  ret = rcutils_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  ret = rcutils_time_point_init(&query_now, ros_time_source);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  assert_no_malloc_begin();
  assert_no_free_begin();
  // Check for normal operation (not allowed to alloc).
  ret = rcutils_time_point_get_now(&query_now);
  assert_no_malloc_end();
  assert_no_realloc_end();
  assert_no_free_end();
  stop_memory_checking();
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_NE(query_now.nanoseconds, 0u);
  // Compare to std::chrono::system_clock time (within a second).
  ret = rcutils_time_point_get_now(&query_now);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = query_now.nanoseconds - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCUTILS_MS_TO_NS(k_tolerance_ms)) << "ros_clock differs";
  }
  // Test ros time specific APIs
  rcutils_time_point_value_t set_point = 1000000000ull;
  // Check initialized state
  ret = rcutils_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  // set the time point
  ret = rcutils_set_ros_time_override(ros_time_source, set_point);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  // check still disabled
  ret = rcutils_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  // get real
  ret = rcutils_time_point_get_now(&query_now);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = query_now.nanoseconds - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCUTILS_MS_TO_NS(k_tolerance_ms)) << "ros_clock differs";
  }
  // enable
  ret = rcutils_enable_ros_time_override(ros_time_source);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  // check enabled
  ret = rcutils_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_EQ(is_enabled, true);
  // get sim
  ret = rcutils_time_point_get_now(&query_now);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_EQ(query_now.nanoseconds, set_point);
  // disable
  ret = rcutils_disable_ros_time_override(ros_time_source);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  // check disabled
  ret = rcutils_is_enabled_ros_time_override(ros_time_source, &is_enabled);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_EQ(is_enabled, false);
  // get real
  ret = rcutils_time_point_get_now(&query_now);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = query_now.nanoseconds - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCUTILS_MS_TO_NS(k_tolerance_ms)) << "ros_clock differs";
  }
}

TEST_F(TestTimeFixture, test_rcutils_init_for_time_source_and_point) {
  assert_no_realloc_begin();
  rcutils_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcutils_ros_time_source_init(nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_INVALID_ARGUMENT) << rcutils_get_error_string_safe();
  rcutils_reset_error();
  // Check for normal operation (not allowed to alloc).
  rcutils_time_source_t source;
  ret = rcutils_ros_time_source_init(&source);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  rcutils_time_point_t a_time;
  ret = rcutils_time_point_init(&a_time, &source);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  rcutils_time_point_t default_time;
  ret = rcutils_time_point_init(&default_time, nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  // assert_no_malloc_begin();
  // assert_no_free_begin();
  // // Do stuff in here
  // assert_no_malloc_end();
  // assert_no_realloc_end();
  // assert_no_free_end();
  // stop_memory_checking();
  // EXPECT_NE(now.nanoseconds, 0u);
  // // Compare to std::chrono::system_clock time (within a second).
  // now = {0};
  // ret = rcutils_system_time_now(&now);
  // {
  //   std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
  //   auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
  // now_sc.time_since_epoch());
  //   int64_t now_ns_int = now_ns.count();
  //   int64_t now_diff = now.nanoseconds - now_ns_int;
  //   const int k_tolerance_ms = 1000;
  //   EXPECT_LE(llabs(now_diff), RCUTILS_MS_TO_NS(k_tolerance_ms)) << "system_clock differs";
  // }
  ret = rcutils_time_point_fini(&a_time);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  ret = rcutils_ros_time_source_fini(&source);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
}

TEST(rcutils_time, time_source_validation) {
  ASSERT_FALSE(rcutils_time_source_valid(NULL));
  rcutils_time_source_t uninitialized;
  // Not reliably detectable due to random values.
  // ASSERT_FALSE(rcutils_time_source_valid(&uninitialized));
  rcutils_ret_t ret;
  ret = rcutils_ros_time_source_init(&uninitialized);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
}

TEST(rcutils_time, default_time_source_instanciation) {
  rcutils_time_source_t * ros_time_source = rcutils_get_default_ros_time_source();
  ASSERT_TRUE(rcutils_time_source_valid(ros_time_source));
  rcutils_time_source_t * steady_time_source = rcutils_get_default_steady_time_source();
  ASSERT_TRUE(rcutils_time_source_valid(steady_time_source));
  rcutils_time_source_t * system_time_source = rcutils_get_default_system_time_source();
  ASSERT_TRUE(rcutils_time_source_valid(system_time_source));
}

TEST(rcutils_time, specific_time_source_instantiation) {
  {
    rcutils_time_source_t uninitialized_time_source;
    rcutils_ret_t ret = rcutils_time_source_init(
      RCUTILS_TIME_SOURCE_UNINITIALIZED, &uninitialized_time_source);
    EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
    EXPECT_EQ(uninitialized_time_source.type, RCUTILS_TIME_SOURCE_UNINITIALIZED) <<
      "Expected time source of type RCUTILS_TIME_SOURCE_UNINITIALIZED";
  }
  {
    rcutils_time_source_t ros_time_source;
    rcutils_ret_t ret = rcutils_time_source_init(RCUTILS_ROS_TIME, &ros_time_source);
    EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
    EXPECT_EQ(ros_time_source.type, RCUTILS_ROS_TIME) <<
      "Expected time source of type RCUTILS_ROS_TIME";
    ret = rcutils_time_source_fini(&ros_time_source);
    EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  }
  {
    rcutils_time_source_t system_time_source;
    rcutils_ret_t ret = rcutils_time_source_init(RCUTILS_SYSTEM_TIME, &system_time_source);
    EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
    EXPECT_EQ(system_time_source.type, RCUTILS_SYSTEM_TIME) <<
      "Expected time source of type RCUTILS_SYSTEM_TIME";
    ret = rcutils_time_source_fini(&system_time_source);
    EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  }
  {
    rcutils_time_source_t steady_time_source;
    rcutils_ret_t ret = rcutils_time_source_init(RCUTILS_STEADY_TIME, &steady_time_source);
    EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
    EXPECT_EQ(steady_time_source.type, RCUTILS_STEADY_TIME) <<
      "Expected time source of type RCUTILS_STEADY_TIME";
    ret = rcutils_time_source_fini(&steady_time_source);
    EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  }
}

TEST(rcutils_time, rcutils_time_difference) {
  rcutils_ret_t ret;
  rcutils_time_point_t a, b;
  ret = rcutils_time_point_init(&a, nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  ret = rcutils_time_point_init(&b, nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  a.nanoseconds = 1000;
  b.nanoseconds = 2000;

  rcutils_duration_t d;
  ret = rcutils_duration_init(&d, nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  ret = rcutils_difference_times(&a, &b, &d);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  EXPECT_EQ(d.nanoseconds, 1000);
  EXPECT_EQ(d.time_source->type, RCUTILS_ROS_TIME);

  ret = rcutils_difference_times(&b, &a, &d);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_EQ(d.nanoseconds, -1000);
  EXPECT_EQ(d.time_source->type, RCUTILS_ROS_TIME);

  rcutils_time_source_t * system_time_source = rcutils_get_default_system_time_source();
  EXPECT_TRUE(system_time_source != nullptr);

  rcutils_time_point_t e;
  ret = rcutils_time_point_init(&e, system_time_source);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
}

static bool pre_callback_called = false;
static bool post_callback_called = false;

void pre_callback(void)
{
  pre_callback_called = true;
  EXPECT_FALSE(post_callback_called);
}
void post_callback(void)
{
  EXPECT_TRUE(pre_callback_called);
  post_callback_called = true;
}


TEST(rcutils_time, rcutils_time_update_callbacks) {
  rcutils_time_source_t * ros_time_source = rcutils_get_default_ros_time_source();
  rcutils_time_point_t query_now;
  rcutils_ret_t ret;
  rcutils_time_point_value_t set_point = 1000000000ull;

  ret = rcutils_time_point_init(&query_now, ros_time_source);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  // set callbacks
  ros_time_source->pre_update = pre_callback;
  ros_time_source->post_update = post_callback;


  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // Query it to do something different. no changes expected
  ret = rcutils_time_point_get_now(&query_now);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // Set the time before it's enabled. Should be no callbacks
  ret = rcutils_set_ros_time_override(ros_time_source, set_point);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // enable
  ret = rcutils_enable_ros_time_override(ros_time_source);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  EXPECT_FALSE(pre_callback_called);
  EXPECT_FALSE(post_callback_called);

  // Set the time now that it's enabled, now get callbacks
  ret = rcutils_set_ros_time_override(ros_time_source, set_point);
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();

  EXPECT_TRUE(pre_callback_called);
  EXPECT_TRUE(post_callback_called);
}
