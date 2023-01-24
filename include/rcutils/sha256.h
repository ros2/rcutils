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

/// \file Provides a simple SHA256 algorithm for hashing.
/// This implementation makes no security guarantees, its use case
/// is for non-sensitive comparison of message digests
/// Implementation originally copied from Brad Conte
/// https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c

#ifndef RCUTILS__SHA256_H_
#define RCUTILS__SHA256_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

#define RCUTILS_SHA256_BLOCK_SIZE 32

typedef struct
{
  uint8_t data[64];
  uint32_t datalen;
  uint64_t bitlen;
  uint32_t state[8];
} rcutils_sha256_ctx_t;

/// Simple SHA256 implementation
/*********************************************************************
* Filename:   sha256.c
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Implementation of the SHA-256 hashing algorithm.
              SHA-256 is one of the three algorithms in the SHA2
              specification. The others, SHA-384 and SHA-512, are not
              offered in this implementation.
              Algorithm specification can be found here:
               * http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
              This implementation uses little endian byte order.
*********************************************************************/


/**
 *
 *
 * \param[inout] ctx
 * \return rcutils_ret_t
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_sha256_init(rcutils_sha256_ctx_t * ctx);

/**
 *
 *
 * \param ctx
 * \param data
 * \param len
 * \return rcutils_ret_t
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_sha256_update(rcutils_sha256_ctx_t * ctx, const uint8_t data[], size_t len);

/**
 *
 *
 * \param ctx
 * \param hash
 * \return rcutils_ret_t
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t sha256_final(rcutils_sha256_ctx_t * ctx, uint8_t hash[]);

#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__SHA256_H_
