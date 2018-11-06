// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS__TYPES__UNSIGNED_CHAR_ARRAY_H_
#define RCUTILS__TYPES__UNSIGNED_CHAR_ARRAY_H_

#if __cplusplus
extern "C"
{
#endif

#include "rcutils/allocator.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

typedef struct RCUTILS_PUBLIC_TYPE rcutils_unsigned_char_array_t
{
  unsigned char * buffer;
  size_t buffer_length;
  size_t buffer_capacity;
  rcutils_allocator_t allocator;
} rcutils_unsigned_char_array_t;

/// Return a zero initialized unsigned char array struct.
/**
 * \return rcutils_unsigned_char_array_t a zero initialized unsigned char array struct
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_unsigned_char_array_t
rcutils_get_zero_initialized_unsigned_char_array(void);

/// Initialize a zero initialized unsigned char array struct.
/**
 * This function may leak if the unsigned char array struct is already
 * pre-initialized.
 * If the capacity is set to 0, no memory is allocated and the internal buffer
 * is still NULL.
 *
 * \param char_array a pointer to the to be initialized unsigned char array struct
 * \param buffer_capacity the size of the memory to allocate for the byte stream
 * \param allocator the allocator to use for the memory allocation
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_ERROR` if an unexpected error occurs
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_unsigned_char_array_init(
  rcutils_unsigned_char_array_t * char_array,
  size_t buffer_capacity,
  const rcutils_allocator_t * allocator);

/// Finalize a unsigned char array struct.
/**
 * Cleans up and deallocates any resources used in a rcutils_unsigned_char_array_t.
 * Passing a rcutils_unsigned_char_array_t which has not been zero initialized using
 * rcutils_get_zero_initialized_unsigned_char_array() to this function is undefined
 * behavior.
 *
 * \param char_array pointer to the rcutils_unsigned_char_array_t to be cleaned up
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_BAD_ALLOC` if memory allocation failed, or
 * \return `RCUTILS_RET_ERROR` if an unexpected error occurs
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_unsigned_char_array_fini(rcutils_unsigned_char_array_t * char_array);

/// Resize the internal buffer of the unsigned char array.
/**
 * The internal buffer of the unsigned char array can be resized dynamically if needed.
 * If the new size is smaller than the current capacity, then the memory is
 * truncated.
 * Be aware, that this will deallocate the memory and therefore invalidates any
 * pointers to this storage.
 * If the new size is larger, new memory is getting allocated and the existing
 * content is copied over.
 *
 * \param char_array pointer to the instance of rcutils_unsigned_char_array_t which is being resized
 * \param new_size the new size of the internal buffer
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_BAD_ALLOC` if memory allocation failed, or
 * \return `RCUTILS_RET_ERROR` if an unexpected error occurs
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_unsigned_char_array_resize(rcutils_unsigned_char_array_t * char_array, size_t new_size);

#if __cplusplus
}
#endif

#endif  // RCUTILS__TYPES__UNSIGNED_CHAR_ARRAY_H_
