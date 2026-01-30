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

#ifndef RCUTILS__BASE64_H_
#define RCUTILS__BASE64_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcutils/allocator.h"
#include "rcutils/visibility_control.h"
#include "rcutils/types.h"

#define RCUTILS_BASE64_ENCODED_MAX_LENGTH (64 * 1024 * 1024)  // 64 MiB

/**
 * \brief Decode a base64-encoded string into a byte array.
 *
 * This function converts a base64-encoded string into its original binary representation.
 * Base64 encoding uses 4 ASCII characters to represent 3 bytes of data.
 *
 * \param base64_str The null-terminated base64 string to decode.
 * \param byte_array Pointer to rcutils_uint8_array_t to store the decoded byte array (output).
 *                   The array must be zero-initialized by rcutils_get_zero_initialized_uint8_array
 *                   before calling this function.
 * \param allocator The allocator to use for memory allocation.
 * \return RCUTILS_RET_OK on success.
 * \return RCUTILS_RET_INVALID_ARGUMENT if base64_str is NULL, or
 *                                      if byte_array is NULL, or
 *                                      if allocator is NULL or invalid, or
 *                                      if byte_array is not zero-initialized.
 * \return RCUTILS_RET_ERROR if base64 string length exceeds RCUTILS_BASE64_ENCODED_MAX_LENGTH, or
 *                           if base64 string length is not a multiple of 4, or
 *                           if base64 string contains invalid characters, or
 *                           if base64 string has invalid padding.
 * \return RCUTILS_RET_BAD_ALLOC if memory allocation fails.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_decode_base64(
  const char * base64_str,
  rcutils_uint8_array_t * byte_array,
  const rcutils_allocator_t * allocator);

/**
 * \brief Encode a byte array into a base64-encoded string.
 *
 * This function converts a binary byte array into its base64-encoded representation.
 * Base64 encoding uses 4 ASCII characters to represent 3 bytes of data.
 *
 * The output string should be freed by the caller using the provided allocator.
 *
 * \param byte_array The rcutils_uint8_array_t structure containing the byte array to encode.
 * \param base64_str Pointer to store the allocated base64 string (output).
 * \param allocator The allocator to use for memory allocation.
 * \return RCUTILS_RET_OK on success.
 * \return RCUTILS_RET_INVALID_ARGUMENT if byte_array is NULL, or
 *                                      if byte_array->buffer is NULL, or
 *                                      if byte_array->buffer_length is zero, or
 *                                      if base64_str is NULL, or
 *                                      if allocator is NULL or invalid.
 * \return RCUTILS_RET_ERROR if byte_array length exceeds limit for base64 encoding.
 * \return RCUTILS_RET_BAD_ALLOC if memory allocation fails.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_encode_base64(
  const rcutils_uint8_array_t * byte_array,
  char ** base64_str,
  const rcutils_allocator_t * allocator);

#ifdef __cplusplus
}
#endif
#endif  // RCUTILS__BASE64_H_
