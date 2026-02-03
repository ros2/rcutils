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

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <stdint.h>
#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/base64.h"
#include "rcutils/error_handling.h"
#include "rcutils/strnlen.h"
#include "rcutils/types.h"
#include "rcutils/types/uint8_array.h"

#define BASE64_INVALID 255u

// Initialize the base64 lookup table
static uint8_t base64_map[256];
#ifdef _WIN32
static INIT_ONCE base64_map_initialization_once = INIT_ONCE_STATIC_INIT;

static void initialize_base64_map(void);

BOOL CALLBACK initialize_base64_map_callback(
  PINIT_ONCE InitOnce,
  PVOID Parameter,
  PVOID *lpContext)
{
  initialize_base64_map();
  return TRUE;
}
#else
static pthread_once_t base64_map_initialization_once = PTHREAD_ONCE_INIT;
#endif

static void initialize_base64_map(void)
{
  // Initialize all values to invalid
  for (int i = 0; i < 256; i++) {
    base64_map[i] = BASE64_INVALID;
  }

  // Set valid base64 characters
  base64_map['A'] = 0;  base64_map['B'] = 1;  base64_map['C'] = 2;  base64_map['D'] = 3;
  base64_map['E'] = 4;  base64_map['F'] = 5;  base64_map['G'] = 6;  base64_map['H'] = 7;
  base64_map['I'] = 8;  base64_map['J'] = 9;  base64_map['K'] = 10; base64_map['L'] = 11;
  base64_map['M'] = 12; base64_map['N'] = 13; base64_map['O'] = 14; base64_map['P'] = 15;
  base64_map['Q'] = 16; base64_map['R'] = 17; base64_map['S'] = 18; base64_map['T'] = 19;
  base64_map['U'] = 20; base64_map['V'] = 21; base64_map['W'] = 22; base64_map['X'] = 23;
  base64_map['Y'] = 24; base64_map['Z'] = 25;
  base64_map['a'] = 26; base64_map['b'] = 27; base64_map['c'] = 28; base64_map['d'] = 29;
  base64_map['e'] = 30; base64_map['f'] = 31; base64_map['g'] = 32; base64_map['h'] = 33;
  base64_map['i'] = 34; base64_map['j'] = 35; base64_map['k'] = 36; base64_map['l'] = 37;
  base64_map['m'] = 38; base64_map['n'] = 39; base64_map['o'] = 40; base64_map['p'] = 41;
  base64_map['q'] = 42; base64_map['r'] = 43; base64_map['s'] = 44; base64_map['t'] = 45;
  base64_map['u'] = 46; base64_map['v'] = 47; base64_map['w'] = 48; base64_map['x'] = 49;
  base64_map['y'] = 50; base64_map['z'] = 51;
  base64_map['0'] = 52; base64_map['1'] = 53; base64_map['2'] = 54; base64_map['3'] = 55;
  base64_map['4'] = 56; base64_map['5'] = 57; base64_map['6'] = 58; base64_map['7'] = 59;
  base64_map['8'] = 60; base64_map['9'] = 61; base64_map['+'] = 62; base64_map['/'] = 63;
}

rcutils_ret_t rcutils_decode_base64(
  const char * base64_str,
  rcutils_uint8_array_t * byte_array,
  const rcutils_allocator_t * allocator)
{
#ifdef _WIN32
  InitOnceExecuteOnce(
    &base64_map_initialization_once,
    initialize_base64_map_callback,
    NULL,
    NULL);
#else
  pthread_once(&base64_map_initialization_once, initialize_base64_map);
#endif

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(base64_str, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(byte_array, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    allocator, "base64 decode function has no valid allocator",
    return RCUTILS_RET_INVALID_ARGUMENT);

  // Check that byte_array is zero-initialized
  if (byte_array->buffer != NULL ||
    byte_array->buffer_length != 0 ||
    byte_array->buffer_capacity != 0)
  {
    RCUTILS_SET_ERROR_MSG("byte_array must be zero-initialized before decoding");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  size_t input_str_len = rcutils_strnlen(base64_str, RCUTILS_BASE64_ENCODED_MAX_LENGTH);
  if (RCUTILS_BASE64_ENCODED_MAX_LENGTH == input_str_len) {
    RCUTILS_SET_ERROR_MSG("base64 string length exceeds limit");
    return RCUTILS_RET_ERROR;
  }
  if (0 == input_str_len) {
    return RCUTILS_RET_OK;
  }
  if (0 != (input_str_len % 4)) {
    RCUTILS_SET_ERROR_MSG("base64 string length is not a multiple of 4");
    return RCUTILS_RET_ERROR;
  }

  // Count padding characters ('=' at the end)
  // Valid base64 can have 0, 1, or 2 padding characters
  size_t padding = 0;
  if ('=' == base64_str[input_str_len - 1]) {
    padding++;
    if (input_str_len >= 2 && '=' == base64_str[input_str_len - 2]) {
      padding++;
    }
  }

  if (padding > 2) {
    RCUTILS_SET_ERROR_MSG("invalid base64 padding");
    return RCUTILS_RET_ERROR;
  }

  // Calculate the decoded output size
  // Each 4-character block decodes to 3 bytes, minus any padding
  size_t decoded_len_blocks = input_str_len / 4;
  size_t max_decoded_len = decoded_len_blocks * 3;

  if (padding >= max_decoded_len) {
    RCUTILS_SET_ERROR_MSG("invalid base64: padding exceeds expected length");
    return RCUTILS_RET_ERROR;
  }

  size_t decoded_len = max_decoded_len - padding;

  // Allocate memory for the decoded output
  rcutils_ret_t ret = rcutils_uint8_array_init(byte_array, decoded_len, allocator);
  if (ret != RCUTILS_RET_OK) {
    return ret;
  }
  byte_array->buffer_length = decoded_len;

  // Decode the base64 string block by block
  // Each iteration processes 4 input characters (sextets) to produce 3 output bytes (octets)
  size_t write_index = 0;
  for (size_t read_index = 0; read_index < input_str_len; read_index += 4) {
    // Extract 4 sextets (6-bit values) from the input
    uint8_t sextets[4] = {0, 0, 0, 0};

    for (size_t i = 0; i < 4; ++i) {
      char c = base64_str[read_index + i];

      if ('=' == c) {
        // Padding character validation:
        // - Must be in the last block (last 2 positions of the string)
        // - If at position 2, position 3 must also be padding
        // - Cannot appear at positions 0 or 1 of a block
        if ((read_index + i) < (input_str_len - 2) || i < 2) {
          RCUTILS_SET_ERROR_MSG("invalid base64 padding position");
          goto err;
        }

        if (i == 2 && base64_str[read_index + 3] != '=') {
          RCUTILS_SET_ERROR_MSG("invalid base64 padding");
          goto err;
        }

        sextets[i] = 0;
      } else {
        // Map base64 character to its 6-bit value
        uint8_t value = base64_map[(uint8_t)c];
        if (BASE64_INVALID == value) {
          RCUTILS_SET_ERROR_MSG("invalid base64 character");
          goto err;
        }
        sextets[i] = value;
      }
    }

    // Combine the 4 sextets (6 bits each) into a 24-bit block
    // Sextet 0: bits 23-18 | Sextet 1: bits 17-12 | Sextet 2: bits 11-6 | Sextet 3: bits 5-0
    uint32_t block =
      ((uint32_t)sextets[0] << 18) |
      ((uint32_t)sextets[1] << 12) |
      ((uint32_t)sextets[2] << 6) |
      ((uint32_t)sextets[3] << 0);

    // Extract 3 bytes from the 24-bit block
    // Byte 0: bits 23-16 | Byte 1: bits 15-8 | Byte 2: bits 7-0
    if (write_index < decoded_len) {
      byte_array->buffer[write_index++] = (uint8_t)((block >> 16) & 0xFFu);
    }
    if (write_index < decoded_len) {
      byte_array->buffer[write_index++] = (uint8_t)((block >> 8) & 0xFFu);
    }
    if (write_index < decoded_len) {
      byte_array->buffer[write_index++] = (uint8_t)(block & 0xFFu);
    }
  }

  return RCUTILS_RET_OK;

err:
  // Clean up allocated memory on error
  if (RCUTILS_RET_OK != rcutils_uint8_array_fini(byte_array)) {
    RCUTILS_SET_ERROR_MSG("failed to finalize byte_array during cleanup");
  }
  return RCUTILS_RET_ERROR;
}

rcutils_ret_t rcutils_encode_base64(
  const rcutils_uint8_array_t * byte_array,
  char ** base64_str,
  const rcutils_allocator_t * allocator)
{
  // Base64 encoding table
  static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(byte_array, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(byte_array->buffer, RCUTILS_RET_INVALID_ARGUMENT);
  if (byte_array->buffer_length == 0) {
    RCUTILS_SET_ERROR_MSG("byte_array length is zero");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(base64_str, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    allocator, "base64 encode function has no valid allocator",
    return RCUTILS_RET_INVALID_ARGUMENT);

  *base64_str = NULL;

  // Check for input size limit to prevent integer overflow
  // The maximum input size should ensure the encoded output fits within reasonable limits
  // Base64 encoding: 3 bytes -> 4 chars, so max input = (LIMIT / 4) * 3
  size_t max_input_len = (RCUTILS_BASE64_ENCODED_MAX_LENGTH / 4) * 3;
  if (byte_array->buffer_length > max_input_len) {
    RCUTILS_SET_ERROR_MSG("byte_array length exceeds limit for base64 encoding");
    return RCUTILS_RET_ERROR;
  }

  // Calculate the encoded output size
  // Each 3-byte block encodes to 4 characters, plus padding if necessary
  size_t full_blocks = byte_array->buffer_length / 3;
  size_t remainder = byte_array->buffer_length % 3;
  size_t encoded_len = full_blocks * 4;

  // Add padding block if there's a remainder
  if (remainder > 0) {
    encoded_len += 4;
  }

  // Allocate memory for the encoded output (plus null terminator)
  *base64_str = (char *)allocator->allocate(encoded_len + 1, allocator->state);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    *base64_str,
    "failed to allocate memory for base64 encode",
    return RCUTILS_RET_BAD_ALLOC);

  // Encode the byte array block by block
  // Each iteration processes 3 input bytes (octets) to produce 4 output characters (sextets)
  size_t read_index = 0;
  size_t write_index = 0;

  // Process full 3-byte blocks
  while (read_index + 3 <= byte_array->buffer_length) {
    // Combine 3 bytes (octets) into a 24-bit block
    // Byte 0: bits 23-16 | Byte 1: bits 15-8 | Byte 2: bits 7-0
    uint32_t block =
      ((uint32_t)byte_array->buffer[read_index] << 16) |
      ((uint32_t)byte_array->buffer[read_index + 1] << 8) |
      ((uint32_t)byte_array->buffer[read_index + 2]);

    // Extract 4 sextets (6-bit values) from the 24-bit block
    // Sextet 0: bits 23-18 | Sextet 1: bits 17-12 | Sextet 2: bits 11-6 | Sextet 3: bits 5-0
    (*base64_str)[write_index++] = base64_chars[(block >> 18) & 0x3F];
    (*base64_str)[write_index++] = base64_chars[(block >> 12) & 0x3F];
    (*base64_str)[write_index++] = base64_chars[(block >> 6) & 0x3F];
    (*base64_str)[write_index++] = base64_chars[block & 0x3F];

    read_index += 3;
  }

  // Process the remaining bytes (if any) with padding
  if (remainder == 1) {
    // 1 remaining byte: encode to 2 characters + 2 padding characters
    uint32_t block = (uint32_t)byte_array->buffer[read_index] << 16;

    (*base64_str)[write_index++] = base64_chars[(block >> 18) & 0x3F];
    (*base64_str)[write_index++] = base64_chars[(block >> 12) & 0x3F];
    (*base64_str)[write_index++] = '=';
    (*base64_str)[write_index++] = '=';
  } else if (remainder == 2) {
    // 2 remaining bytes: encode to 3 characters + 1 padding character
    uint32_t block =
      ((uint32_t)byte_array->buffer[read_index] << 16) |
      ((uint32_t)byte_array->buffer[read_index + 1] << 8);

    (*base64_str)[write_index++] = base64_chars[(block >> 18) & 0x3F];
    (*base64_str)[write_index++] = base64_chars[(block >> 12) & 0x3F];
    (*base64_str)[write_index++] = base64_chars[(block >> 6) & 0x3F];
    (*base64_str)[write_index++] = '=';
  }

  // Null-terminate the string
  (*base64_str)[write_index] = '\0';

  return RCUTILS_RET_OK;
}
#ifdef __cplusplus
}
#endif
