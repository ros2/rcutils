// Copyright 2023 Open Source Robotics Foundation, Inc.
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

#include "rcutils/sha256.h"

TEST(TestSHA256, test_text1) {
  uint8_t text1[] = {"abc"};
  size_t text1_len = sizeof(text1) - 1;
  uint8_t expected_hash1[RCUTILS_SHA256_BLOCK_SIZE] = {
    0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
    0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
    0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
    0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
  uint8_t buf[RCUTILS_SHA256_BLOCK_SIZE];


  rcutils_sha256_ctx_t ctx;
  rcutils_sha256_init(&ctx);
  rcutils_sha256_update(&ctx, text1, text1_len);
  rcutils_sha256_final(&ctx, buf);

  ASSERT_EQ(0, memcmp(expected_hash1, buf, RCUTILS_SHA256_BLOCK_SIZE));
}

TEST(TestSHA256, test_text2) {
  uint8_t text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
  size_t text2_len = sizeof(text2) - 1;
  uint8_t expected_hash2[RCUTILS_SHA256_BLOCK_SIZE] = {
    0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
    0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
    0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
    0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};
  uint8_t buf[RCUTILS_SHA256_BLOCK_SIZE];

  rcutils_sha256_ctx_t ctx;
  rcutils_sha256_init(&ctx);
  rcutils_sha256_update(&ctx, text2, text2_len);
  rcutils_sha256_final(&ctx, buf);

  ASSERT_EQ(0, memcmp(expected_hash2, buf, RCUTILS_SHA256_BLOCK_SIZE));
}

TEST(TestSHA256, test_multi_update) {
  uint8_t text[] = {"aaaaaaaaaa"};
  size_t text_len = sizeof(text) - 1;

  uint8_t expected_hash[RCUTILS_SHA256_BLOCK_SIZE] = {
    0x28, 0x16, 0x59, 0x78, 0x88, 0xe4, 0xa0, 0xd3,
    0xa3, 0x6b, 0x82, 0xb8, 0x33, 0x16, 0xab, 0x32,
    0x68, 0x0e, 0xb8, 0xf0, 0x0f, 0x8c, 0xd3, 0xb9,
    0x04, 0xd6, 0x81, 0x24, 0x6d, 0x28, 0x5a, 0x0e};
  uint8_t buf[RCUTILS_SHA256_BLOCK_SIZE];

  rcutils_sha256_ctx_t ctx;
  rcutils_sha256_init(&ctx);
  for (int i = 0; i < 10; i++) {
    rcutils_sha256_update(&ctx, text, text_len);
  }
  rcutils_sha256_final(&ctx, buf);

  ASSERT_EQ(0, memcmp(expected_hash, buf, RCUTILS_SHA256_BLOCK_SIZE));
}
