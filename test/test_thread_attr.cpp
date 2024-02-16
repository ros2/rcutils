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
#include <climits>
#include <cstring>

#include "rcutils/thread_attr.h"
#include "rcutils/error_handling.h"

struct TestThreadAffinity : ::testing::Test
{
  void SetUp() override
  {
    rcutils_reset_error();
    affinity = rcutils_get_zero_initialized_thread_core_affinity();
    alloc = rcutils_get_default_allocator();
  }
  void TearDown() override
  {
    rcutils_ret_t ret = rcutils_thread_core_affinity_fini(&affinity);
    EXPECT_EQ(RCUTILS_RET_OK, ret);
  }
  rcutils_thread_core_affinity_t affinity;
  rcutils_allocator_t alloc;
};

struct TestThreadAttrs : TestThreadAffinity
{
  void SetUp() override
  {
    TestThreadAffinity::SetUp();
    attrs = rcutils_get_zero_initialized_thread_attrs();
  }
  void TearDown() override
  {
    rcutils_ret_t ret = rcutils_thread_attrs_fini(&attrs);
    EXPECT_EQ(RCUTILS_RET_OK, ret);
    TestThreadAffinity::TearDown();
  }
  rcutils_thread_attrs_t attrs;
};

TEST_F(TestThreadAffinity, zero_initialized_object) {
  char zeros_aff[sizeof(rcutils_thread_core_affinity_t)] = {0};

  EXPECT_EQ(0, memcmp(&affinity, zeros_aff, sizeof(rcutils_thread_core_affinity_t)));
}

TEST_F(TestThreadAffinity, initialization_without_cap) {
  rcutils_ret_t ret = rcutils_thread_core_affinity_init(&affinity, alloc);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(0, affinity.core_count);
  EXPECT_EQ(nullptr, affinity.set);
  EXPECT_EQ(alloc.allocate, affinity.allocator.allocate);
  EXPECT_EQ(alloc.reallocate, affinity.allocator.reallocate);
  EXPECT_EQ(alloc.zero_allocate, affinity.allocator.zero_allocate);
  EXPECT_EQ(alloc.deallocate, affinity.allocator.deallocate);
  EXPECT_EQ(alloc.state, affinity.allocator.state);
}

TEST_F(TestThreadAffinity, initialization_with_cap) {
  rcutils_ret_t ret = rcutils_thread_core_affinity_init_with_capacity(&affinity, 60, alloc);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(64, affinity.core_count);
  ASSERT_NE(nullptr, affinity.set);
  EXPECT_EQ(alloc.allocate, affinity.allocator.allocate);
  EXPECT_EQ(alloc.reallocate, affinity.allocator.reallocate);
  EXPECT_EQ(alloc.zero_allocate, affinity.allocator.zero_allocate);
  EXPECT_EQ(alloc.deallocate, affinity.allocator.deallocate);
  EXPECT_EQ(alloc.state, affinity.allocator.state);

  for (size_t i = 0; i < 64 / CHAR_BIT; ++i) {
    EXPECT_EQ(0, affinity.set[i]);
  }
}

TEST_F(TestThreadAffinity, set_bits) {
  rcutils_ret_t ret = rcutils_thread_core_affinity_init(&affinity, alloc);

  ASSERT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_thread_core_affinity_set(&affinity, 0);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_LT(0, affinity.core_count);
  ASSERT_NE(nullptr, affinity.set);
  for (unsigned i = 0; i < 8; ++i) {
    EXPECT_EQ(i == 0, rcutils_thread_core_affinity_is_set(&affinity, i));
  }

  ret = rcutils_thread_core_affinity_set(&affinity, 8);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_LT(0, affinity.core_count);
  ASSERT_NE(nullptr, affinity.set);
  for (unsigned i = 0; i < 16; ++i) {
    EXPECT_EQ(i == 0 || i == 8, rcutils_thread_core_affinity_is_set(&affinity, i));
  }

  ret = rcutils_thread_core_affinity_set(&affinity, 60);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_LT(60, affinity.core_count);
  ASSERT_NE(nullptr, affinity.set);
  for (unsigned i = 0; i < 64; ++i) {
    EXPECT_EQ(i == 0 || i == 8 || i == 60, rcutils_thread_core_affinity_is_set(&affinity, i));
  }

  ret = rcutils_thread_core_affinity_set(&affinity, 30);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_LT(60, affinity.core_count);
  ASSERT_NE(nullptr, affinity.set);
  for (unsigned i = 0; i < 64; ++i) {
    EXPECT_EQ(
      i == 0 || i == 8 || i == 30 || i == 60,
      rcutils_thread_core_affinity_is_set(&affinity, i));
  }

  ret = rcutils_thread_core_affinity_set(&affinity, 90);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_LT(90, affinity.core_count);
  ASSERT_NE(nullptr, affinity.set);
  for (unsigned i = 0; i < 96; ++i) {
    EXPECT_EQ(
      i == 0 || i == 8 || i == 30 || i == 60 || i == 90,
      rcutils_thread_core_affinity_is_set(&affinity, i));
  }
}

TEST_F(TestThreadAffinity, copy) {
  rcutils_ret_t ret = rcutils_thread_core_affinity_init(&affinity, alloc);

  EXPECT_EQ(RCUTILS_RET_OK, ret);

  ret = rcutils_thread_core_affinity_set(&affinity, 0);
  ret = rcutils_thread_core_affinity_set(&affinity, 10);
  ret = rcutils_thread_core_affinity_set(&affinity, 20);
  ret = rcutils_thread_core_affinity_set(&affinity, 30);

  rcutils_thread_core_affinity_t dest = rcutils_get_zero_initialized_thread_core_affinity();
  ret = rcutils_thread_core_affinity_copy(&affinity, &dest);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  ASSERT_NE(nullptr, dest.set);
  EXPECT_LT(30, dest.core_count);
  EXPECT_EQ(affinity.allocator.allocate, dest.allocator.allocate);
  EXPECT_EQ(affinity.allocator.deallocate, dest.allocator.deallocate);
  EXPECT_EQ(affinity.allocator.reallocate, dest.allocator.reallocate);
  EXPECT_EQ(affinity.allocator.zero_allocate, dest.allocator.zero_allocate);
  EXPECT_EQ(affinity.allocator.state, dest.allocator.state);
  for (unsigned i = 0; i < dest.core_count; ++i) {
    EXPECT_EQ(
      rcutils_thread_core_affinity_is_set(&affinity, i),
      rcutils_thread_core_affinity_is_set(&dest, i));
  }
}

TEST_F(TestThreadAffinity, bit_range_ops) {
  rcutils_ret_t ret = rcutils_thread_core_affinity_init_with_capacity(&affinity, 30, alloc);

  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_LE(32, affinity.core_count);

  ret = rcutils_thread_core_affinity_fill(&affinity, 0, affinity.core_count - 1);

  for (unsigned i = 0; i < 32; ++i) {
    EXPECT_EQ(true, rcutils_thread_core_affinity_is_set(&affinity, i));
  }

  ret = rcutils_thread_core_affinity_clear(&affinity, 8, 24);

  for (unsigned i = 0; i < 8; ++i) {
    EXPECT_EQ(true, rcutils_thread_core_affinity_is_set(&affinity, i));
  }
  for (unsigned i = 8; i < 25; ++i) {
    EXPECT_EQ(false, rcutils_thread_core_affinity_is_set(&affinity, i));
  }
  for (unsigned i = 25; i < 32; ++i) {
    EXPECT_EQ(true, rcutils_thread_core_affinity_is_set(&affinity, i));
  }
}

TEST_F(TestThreadAttrs, zero_initialized_object) {
  char zeros_attrs[sizeof(rcutils_thread_attrs_t)] = {0};

  EXPECT_EQ(0, memcmp(&attrs, zeros_attrs, sizeof(rcutils_thread_attrs_t)));
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
  ret = rcutils_thread_core_affinity_init(&affinity, alloc);
  ret = rcutils_thread_core_affinity_set(&affinity, 0xaa);

  char attr_tag[32];
  for (size_t i = 0; i < 100; ++i) {
    snprintf(attr_tag, sizeof(attr_tag), "attr tag %lu", i);
    ret = rcutils_thread_attrs_add_attr(
      &attrs, RCUTILS_THREAD_SCHEDULING_POLICY_FIFO, &affinity, 0xbb, attr_tag);
    EXPECT_EQ(RCUTILS_RET_OK, ret);
    ASSERT_NE(nullptr, attrs.attributes);
    ASSERT_LE(i + 1, attrs.capacity_attributes);
    EXPECT_EQ(i + 1, attrs.num_attributes);
  }

  for (size_t i = 0; i < 100; ++i) {
    rcutils_thread_attr_t attr = attrs.attributes[i];

    snprintf(attr_tag, sizeof(attr_tag), "attr tag %lu", i);
    EXPECT_EQ(RCUTILS_THREAD_SCHEDULING_POLICY_FIFO, attr.scheduling_policy);
    EXPECT_NE(affinity.set, attr.core_affinity.set);
    EXPECT_EQ(affinity.core_count, attr.core_affinity.core_count);
    EXPECT_EQ(0, memcmp(affinity.set, attr.core_affinity.set, affinity.core_count / CHAR_BIT));
    EXPECT_EQ(0xbb, attr.priority);
    EXPECT_NE(attr_tag, attr.tag);
    EXPECT_STREQ(attr_tag, attr.tag);
  }

  ret = rcutils_thread_attrs_fini(&attrs);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(nullptr, attrs.attributes);
}

TEST_F(TestThreadAttrs, copy) {
  rcutils_ret_t ret = rcutils_thread_attrs_init(&attrs, alloc);
  ret = rcutils_thread_core_affinity_init(&affinity, alloc);
  ret = rcutils_thread_core_affinity_set(&affinity, 0xaa);

  char attr_tag[32];
  for (size_t i = 0; i < 100; ++i) {
    snprintf(attr_tag, sizeof(attr_tag), "attr tag %lu", i);
    ret = rcutils_thread_attrs_add_attr(
      &attrs, RCUTILS_THREAD_SCHEDULING_POLICY_FIFO, &affinity, 0xbb, attr_tag);
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
    EXPECT_EQ(0, memcmp(affinity.set, attr.core_affinity.set, affinity.core_count / CHAR_BIT));
    EXPECT_EQ(0xbb, attr.priority);
    EXPECT_NE(attrs.attributes[i].tag, attr.tag);
    EXPECT_STREQ(attrs.attributes[i].tag, attr.tag);
  }

  ret = rcutils_thread_attrs_fini(&attrs);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(nullptr, attrs.attributes);

  ret = rcutils_thread_attrs_fini(&attrs_copy);
  EXPECT_EQ(RCUTILS_RET_OK, ret);
  EXPECT_EQ(nullptr, attrs_copy.attributes);
}
