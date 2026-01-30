// Copyright 2026 Sony Group Corporation.
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

#include <cstring>
#include <string>
#include <vector>

#include "rcutils/allocator.h"
#include "rcutils/base64.h"
#include "rcutils/error_handling.h"
#include "rcutils/types/uint8_array.h"

// Test decode_base64 with valid input
TEST(TestBase64, DecodeBase64Valid) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();

  // Test case 1: "Hello"
  const char * base64_str = "SGVsbG8=";
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  ASSERT_EQ(5u, byte_array.buffer_length);
  EXPECT_EQ('H', byte_array.buffer[0]);
  EXPECT_EQ('e', byte_array.buffer[1]);
  EXPECT_EQ('l', byte_array.buffer[2]);
  EXPECT_EQ('l', byte_array.buffer[3]);
  EXPECT_EQ('o', byte_array.buffer[4]);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 2: "Hello World"
  byte_array = rcutils_get_zero_initialized_uint8_array();
  base64_str = "SGVsbG8gV29ybGQ=";
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  ASSERT_EQ(11u, byte_array.buffer_length);
  EXPECT_EQ(0, memcmp(byte_array.buffer, "Hello World", 11));
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 3: "ABC"
  byte_array = rcutils_get_zero_initialized_uint8_array();
  base64_str = "QUJD";
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  ASSERT_EQ(3u, byte_array.buffer_length);
  EXPECT_EQ('A', byte_array.buffer[0]);
  EXPECT_EQ('B', byte_array.buffer[1]);
  EXPECT_EQ('C', byte_array.buffer[2]);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 4: Single byte "A"
  byte_array = rcutils_get_zero_initialized_uint8_array();
  base64_str = "QQ==";
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  ASSERT_EQ(1u, byte_array.buffer_length);
  EXPECT_EQ('A', byte_array.buffer[0]);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 5: Two bytes "AB"
  byte_array = rcutils_get_zero_initialized_uint8_array();
  base64_str = "QUI=";
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  ASSERT_EQ(2u, byte_array.buffer_length);
  EXPECT_EQ('A', byte_array.buffer[0]);
  EXPECT_EQ('B', byte_array.buffer[1]);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 6: Binary data {0x00, 0xFF, 0x80, 0x7F}
  byte_array = rcutils_get_zero_initialized_uint8_array();
  base64_str = "AP+Afw==";
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  ASSERT_EQ(4u, byte_array.buffer_length);
  EXPECT_EQ(0x00, byte_array.buffer[0]);
  EXPECT_EQ(0xFF, byte_array.buffer[1]);
  EXPECT_EQ(0x80, byte_array.buffer[2]);
  EXPECT_EQ(0x7F, byte_array.buffer[3]);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));
}

// Test decode_base64 with empty string
TEST(TestBase64, DecodeBase64EmptyString) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();

  const char * base64_str = "";
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  EXPECT_EQ(0u, byte_array.buffer_length);
  EXPECT_EQ(nullptr, byte_array.buffer);
}

// Test decode_base64 with NULL inputs
TEST(TestBase64, DecodeBase64NullInputs) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();
  const char * base64_str = "SGVsbG8=";

  // NULL base64_str
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_decode_base64(nullptr, &byte_array, &allocator));
  rcutils_reset_error();

  // NULL byte_array
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_decode_base64(base64_str, nullptr, &allocator));
  rcutils_reset_error();

  // NULL allocator
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_decode_base64(base64_str, &byte_array, nullptr));
  rcutils_reset_error();
}

// Test decode_base64 with non-zero-initialized byte_array
TEST(TestBase64, DecodeBase64NonZeroInitializedArray) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();
  const char * base64_str = "SGVsbG8=";

  // Initialize the byte_array first
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, 10, &allocator));

  // This should fail because byte_array is not zero-initialized
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT,
    rcutils_decode_base64(base64_str, &byte_array, &allocator));
  rcutils_reset_error();

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));
}

// Test decode_base64 with invalid base64 string length
TEST(TestBase64, DecodeBase64InvalidLength) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();

  // Length not multiple of 4
  const char * base64_str = "SGVsbG";  // Length 6
  EXPECT_EQ(RCUTILS_RET_ERROR, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  EXPECT_EQ(byte_array.buffer, nullptr);
  EXPECT_EQ(byte_array.buffer_length, 0u);
  rcutils_reset_error();
}

// Test decode_base64 with invalid characters
TEST(TestBase64, DecodeBase64InvalidCharacters) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();

  // Invalid character '@'
  const char * base64_str = "SGVs@G8=";
  EXPECT_EQ(RCUTILS_RET_ERROR, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  EXPECT_EQ(byte_array.buffer, nullptr);
  EXPECT_EQ(byte_array.buffer_length, 0u);
  rcutils_reset_error();

  // Invalid character with space
  byte_array = rcutils_get_zero_initialized_uint8_array();
  base64_str = "SGVs bG8=";
  EXPECT_EQ(RCUTILS_RET_ERROR, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  EXPECT_EQ(byte_array.buffer, nullptr);
  EXPECT_EQ(byte_array.buffer_length, 0u);
  rcutils_reset_error();
}

// Test decode_base64 with invalid padding
TEST(TestBase64, DecodeBase64InvalidPadding) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();

  // Padding in wrong position
  const char * base64_str = "SG=sbG8=";
  EXPECT_EQ(RCUTILS_RET_ERROR, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  rcutils_reset_error();

  // Invalid padding sequence
  byte_array = rcutils_get_zero_initialized_uint8_array();
  base64_str = "SGVs=G8=";
  EXPECT_EQ(RCUTILS_RET_ERROR, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  EXPECT_EQ(byte_array.buffer, nullptr);
  EXPECT_EQ(byte_array.buffer_length, 0u);
  rcutils_reset_error();

  // More than 2 padding characters
  byte_array = rcutils_get_zero_initialized_uint8_array();
  base64_str = "SGVs===";  // Not valid length anyway
  EXPECT_EQ(RCUTILS_RET_ERROR, rcutils_decode_base64(base64_str, &byte_array, &allocator));
  EXPECT_EQ(byte_array.buffer, nullptr);
  EXPECT_EQ(byte_array.buffer_length, 0u);
  rcutils_reset_error();
}

// Test encode_base64 with valid input
TEST(TestBase64, EncodeBase64Valid) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  char * base64_str = nullptr;

  // Test case 1: "Hello"
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();
  const uint8_t data1[] = {'H', 'e', 'l', 'l', 'o'};
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, 5, &allocator));
  memcpy(byte_array.buffer, data1, 5);
  byte_array.buffer_length = 5;

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_encode_base64(&byte_array, &base64_str, &allocator));
  EXPECT_STREQ("SGVsbG8=", base64_str);
  allocator.deallocate(base64_str, allocator.state);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 2: "Hello World"
  byte_array = rcutils_get_zero_initialized_uint8_array();
  const uint8_t data2[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, 11, &allocator));
  memcpy(byte_array.buffer, data2, 11);
  byte_array.buffer_length = 11;

  base64_str = nullptr;
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_encode_base64(&byte_array, &base64_str, &allocator));
  EXPECT_STREQ("SGVsbG8gV29ybGQ=", base64_str);
  allocator.deallocate(base64_str, allocator.state);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 3: "ABC"
  byte_array = rcutils_get_zero_initialized_uint8_array();
  const uint8_t data3[] = {'A', 'B', 'C'};
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, 3, &allocator));
  memcpy(byte_array.buffer, data3, 3);
  byte_array.buffer_length = 3;

  base64_str = nullptr;
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_encode_base64(&byte_array, &base64_str, &allocator));
  EXPECT_STREQ("QUJD", base64_str);
  allocator.deallocate(base64_str, allocator.state);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 4: Single byte "A"
  byte_array = rcutils_get_zero_initialized_uint8_array();
  const uint8_t data4[] = {'A'};
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, 1, &allocator));
  memcpy(byte_array.buffer, data4, 1);
  byte_array.buffer_length = 1;

  base64_str = nullptr;
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_encode_base64(&byte_array, &base64_str, &allocator));
  EXPECT_STREQ("QQ==", base64_str);
  allocator.deallocate(base64_str, allocator.state);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 5: Two bytes "AB"
  byte_array = rcutils_get_zero_initialized_uint8_array();
  const uint8_t data5[] = {'A', 'B'};
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, 2, &allocator));
  memcpy(byte_array.buffer, data5, 2);
  byte_array.buffer_length = 2;

  base64_str = nullptr;
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_encode_base64(&byte_array, &base64_str, &allocator));
  EXPECT_STREQ("QUI=", base64_str);
  allocator.deallocate(base64_str, allocator.state);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));

  // Test case 6: Binary data {0x00, 0xFF, 0x80, 0x7F}
  byte_array = rcutils_get_zero_initialized_uint8_array();
  const uint8_t data6[] = {0x00, 0xFF, 0x80, 0x7F};
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, 4, &allocator));
  memcpy(byte_array.buffer, data6, 4);
  byte_array.buffer_length = 4;

  base64_str = nullptr;
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_encode_base64(&byte_array, &base64_str, &allocator));
  EXPECT_STREQ("AP+Afw==", base64_str);
  allocator.deallocate(base64_str, allocator.state);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));
}

// Test encode_base64 with empty array
TEST(TestBase64, EncodeBase64EmptyArray) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();
  char * base64_str = nullptr;

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, 0, &allocator));
  byte_array.buffer_length = 0;

  // Empty array (buffer_length = 0) should return error
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT,
    rcutils_encode_base64(&byte_array, &base64_str, &allocator));
  rcutils_reset_error();
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));
}

// Test encode_base64 with NULL inputs
TEST(TestBase64, EncodeBase64NullInputs) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();
  char * base64_str = nullptr;

  const uint8_t data[] = {'A', 'B', 'C'};
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, 3, &allocator));
  memcpy(byte_array.buffer, data, 3);
  byte_array.buffer_length = 3;

  // NULL byte_array
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_encode_base64(nullptr, &base64_str, &allocator));
  rcutils_reset_error();

  // NULL base64_str
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_encode_base64(&byte_array, nullptr, &allocator));
  rcutils_reset_error();

  // NULL allocator
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_encode_base64(&byte_array, &base64_str, nullptr));
  rcutils_reset_error();

  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));
}

// Test encode_base64 with NULL buffer in byte_array
TEST(TestBase64, EncodeBase64NullBuffer) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();
  char * base64_str = nullptr;

  // byte_array with NULL buffer
  byte_array.buffer = nullptr;
  byte_array.buffer_length = 5;

  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT,
    rcutils_encode_base64(&byte_array, &base64_str, &allocator));
  rcutils_reset_error();
}

// Test round-trip encoding and decoding
TEST(TestBase64, RoundTripEncodeDecode) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  // Test various data sizes (excluding empty string as encode_base64 requires length > 0)
  std::vector<std::string> test_strings = {
    "A",
    "AB",
    "ABC",
    "ABCD",
    "Hello World!",
    "The quick brown fox jumps over the lazy dog",
    "Base64 encoding test with special chars: !@#$%^&*()_+-=[]{}|;:,.<>?",
  };

  for (const auto & test_str : test_strings) {
    // Encode
    rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();
    ASSERT_EQ(RCUTILS_RET_OK,
      rcutils_uint8_array_init(&byte_array, test_str.size(), &allocator));
    memcpy(byte_array.buffer, test_str.c_str(), test_str.size());
    byte_array.buffer_length = test_str.size();

    char * base64_str = nullptr;
    ASSERT_EQ(RCUTILS_RET_OK, rcutils_encode_base64(&byte_array, &base64_str, &allocator));

    // Decode
    rcutils_uint8_array_t decoded_array = rcutils_get_zero_initialized_uint8_array();
    ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &decoded_array, &allocator));
    // Verify
    EXPECT_EQ(byte_array.buffer_length, decoded_array.buffer_length);
    EXPECT_EQ(0, memcmp(byte_array.buffer, decoded_array.buffer, byte_array.buffer_length));

    // Cleanup
    allocator.deallocate(base64_str, allocator.state);
    ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));
    ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&decoded_array));
  }
}

// Test round-trip with binary data
TEST(TestBase64, RoundTripBinaryData) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  // Test with various binary patterns
  std::vector<std::vector<uint8_t>> test_data = {
    {0x00},
    {0xFF},
    {0x00, 0xFF},
    {0xFF, 0x00},
    {0x00, 0x00, 0x00},
    {0xFF, 0xFF, 0xFF},
    {0x00, 0x7F, 0x80, 0xFF},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
  };

  for (const auto & data : test_data) {
    // Encode
    rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();
    ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, data.size(), &allocator));
    memcpy(byte_array.buffer, data.data(), data.size());
    byte_array.buffer_length = data.size();

    char * base64_str = nullptr;
    ASSERT_EQ(RCUTILS_RET_OK, rcutils_encode_base64(&byte_array, &base64_str, &allocator));

    // Decode
    rcutils_uint8_array_t decoded_array = rcutils_get_zero_initialized_uint8_array();
    ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &decoded_array, &allocator));
    // Verify
    EXPECT_EQ(data.size(), decoded_array.buffer_length);
    EXPECT_EQ(0, memcmp(data.data(), decoded_array.buffer, data.size()));

    // Cleanup
    allocator.deallocate(base64_str, allocator.state);
    ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));
    ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&decoded_array));
  }
}

// Test encode_base64 with large data
TEST(TestBase64, EncodeDecodeLargeData) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  // Create large data (1KB)
  const size_t data_size = 1024;
  rcutils_uint8_array_t byte_array = rcutils_get_zero_initialized_uint8_array();
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_init(&byte_array, data_size, &allocator));

  // Fill with pattern
  for (size_t i = 0; i < data_size; ++i) {
    byte_array.buffer[i] = static_cast<uint8_t>(i % 256);
  }
  byte_array.buffer_length = data_size;

  // Encode
  char * base64_str = nullptr;
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_encode_base64(&byte_array, &base64_str, &allocator));

  // Verify encoded length
  size_t expected_encoded_len = ((data_size + 2) / 3) * 4;
  EXPECT_EQ(expected_encoded_len, strlen(base64_str));

  // Decode
  rcutils_uint8_array_t decoded_array = rcutils_get_zero_initialized_uint8_array();
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_decode_base64(base64_str, &decoded_array, &allocator));

  // Verify
  EXPECT_EQ(data_size, decoded_array.buffer_length);
  EXPECT_EQ(0, memcmp(byte_array.buffer, decoded_array.buffer, data_size));

  // Cleanup
  allocator.deallocate(base64_str, allocator.state);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&byte_array));
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_uint8_array_fini(&decoded_array));
}
