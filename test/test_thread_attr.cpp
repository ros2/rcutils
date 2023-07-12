// Copyright 2023 eSOL Co.,Ltd.
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

#include "rcutils/thread_attr.h"
#include "rcutils/error_handling.h"

struct TestThreadAttrs : ::testing::Test
{
  void SetUp() override
  {
    rcutils_reset_error();
    attrs = rcutils_get_zero_initialized_thread_attrs();
    alloc = rcutils_get_default_allocator();
  }
  void TearDown() override
  {
    rcutils_ret_t ret = rcutils_thread_attrs_fini(&attrs);
    EXPECT_EQ(RCUTILS_RET_OK, ret);
  }
  rcutils_thread_attrs_t attrs;
  rcutils_allocator_t alloc;
};

TEST_F(TestThreadAttrs, zero_initialized_object) {
  char zeros[sizeof(rcutils_thread_attrs_t)] = {0};

  EXPECT_EQ(memcmp(&attrs, zeros, sizeof(rcutils_thread_attrs_t)), 0);
}

TEST_F(TestThreadAttrs, initialization_without_cap) {
  rcutils_ret_t ret = rcutils_thread_attrs_init(&attrs, alloc);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(0ul, attrs.capacity_attributes);
  EXPECT_EQ(0ul, attrs.num_attributes);
  EXPECT_EQ(nullptr, attrs.attributes);
  EXPECT_EQ(alloc.allocate, attrs.allocator.allocate);
  EXPECT_EQ(alloc.reallocate, attrs.allocator.reallocate);
  EXPECT_EQ(alloc.zero_allocate, attrs.allocator.zero_allocate);
  EXPECT_EQ(alloc.deallocate, attrs.allocator.deallocate);
  EXPECT_EQ(alloc.state, attrs.allocator.state);
}

TEST_F(TestThreadAttrs, initialization_with_cap) {
  rcutils_ret_t ret = rcutils_thread_attrs_init_with_capacity(&attrs, alloc, 100);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(100ul, attrs.capacity_attributes);
  EXPECT_EQ(0ul, attrs.num_attributes);
  EXPECT_NE(nullptr, attrs.attributes);
  EXPECT_EQ(alloc.allocate, attrs.allocator.allocate);
  EXPECT_EQ(alloc.reallocate, attrs.allocator.reallocate);
  EXPECT_EQ(alloc.zero_allocate, attrs.allocator.zero_allocate);
  EXPECT_EQ(alloc.deallocate, attrs.allocator.deallocate);
  EXPECT_EQ(alloc.state, attrs.allocator.state);
}

TEST_F(TestThreadAttrs, finalization) {
  rcutils_ret_t ret = rcutils_thread_attrs_init_with_capacity(&attrs, alloc, 100);

  ret = rcutils_thread_attrs_fini(&attrs);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(nullptr, attrs.attributes);
}

TEST_F(TestThreadAttrs, add_attribute) {
  rcutils_ret_t ret = rcutils_thread_attrs_init(&attrs, alloc);

  char thread_name[32];
  for (size_t i = 0; i < 100; ++i) {
    snprintf(thread_name, sizeof(thread_name), "thread name %lu", i);
    ret = rcutils_thread_attrs_add_attr(
      &attrs, RCUTILS_THREAD_SCHEDULING_POLICY_FIFO, 0xaa, 0xbb, thread_name);
    EXPECT_EQ(RCUTILS_RET_OK, ret);
    ASSERT_NE(nullptr, attrs.attributes);
    ASSERT_LE(i + 1, attrs.capacity_attributes);
    EXPECT_EQ(i + 1, attrs.num_attributes);
  }

  for (size_t i = 0; i < 100; ++i) {
    rcutils_thread_attr_t attr = attrs.attributes[i];

    snprintf(thread_name, sizeof(thread_name), "thread name %lu", i);
    EXPECT_EQ(RCUTILS_THREAD_SCHEDULING_POLICY_FIFO, attr.scheduling_policy);
    EXPECT_EQ(0xaa, attr.core_affinity);
    EXPECT_EQ(0xbb, attr.priority);
    EXPECT_NE(thread_name, attr.name);
    EXPECT_STREQ(thread_name, attr.name);
  }

  ret = rcutils_thread_attrs_fini(&attrs);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(nullptr, attrs.attributes);
}

TEST_F(TestThreadAttrs, copy) {
  rcutils_ret_t ret = rcutils_thread_attrs_init(&attrs, alloc);

  char thread_name[32];
  for (size_t i = 0; i < 100; ++i) {
    snprintf(thread_name, sizeof(thread_name), "thread name %lu", i);
    ret = rcutils_thread_attrs_add_attr(
      &attrs, RCUTILS_THREAD_SCHEDULING_POLICY_FIFO, 0xaa, 0xbb, thread_name);
    ASSERT_EQ(RCUTILS_RET_OK, ret);
  }

  rcutils_thread_attrs_t attrs_copy = rcutils_get_zero_initialized_thread_attrs();
  ret = rcutils_thread_attrs_copy(&attrs, &attrs_copy);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(100, attrs_copy.num_attributes);
  EXPECT_LE(100, attrs_copy.capacity_attributes);

  for (size_t i = 0; i < 100; ++i) {
    rcutils_thread_attr_t attr = attrs_copy.attributes[i];
    EXPECT_EQ(RCUTILS_THREAD_SCHEDULING_POLICY_FIFO, attr.scheduling_policy);
    EXPECT_EQ(0xaa, attr.core_affinity);
    EXPECT_EQ(0xbb, attr.priority);
    EXPECT_NE(attrs.attributes[i].name, attr.name);
    EXPECT_STREQ(attrs.attributes[i].name, attr.name);
  }

  ret = rcutils_thread_attrs_fini(&attrs);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(nullptr, attrs.attributes);

  ret = rcutils_thread_attrs_fini(&attrs_copy);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(nullptr, attrs_copy.attributes);
}
