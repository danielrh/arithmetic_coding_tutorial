/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <stdlib.h>
#include <string.h>
//#include "./vpx_config.h"

#include "bitreader.h"
#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
# define htobe64 OSSwapHostToBigInt64
# define be64toh OSSwapBigToHostInt64
# define htobe32 OSSwapHostToBigInt32
# define be32toh OSSwapBigToHostInt32
# define htobe16 OSSwapHostToBigInt16
# define be16toh OSSwapBigToHostInt16

# define htole64 OSSwapHostToLittleInt64
# define le64toh OSSwapLittleToHostInt64
# define htole32 OSSwapHostToLittleInt32
# define le32toh OSSwapLittleToHostInt32
# define htole16 OSSwapHostToLittleInt16
# define le16toh OSSwapLittleToHostInt16
#else
#define _BSD_SOURCE       /* See feature_test_macros(7) */
#include <endian.h>
#endif


//#include "vpx_dsp/prob.h"
//#include "vpx_ports/mem.h"
//#include "vpx_mem/vpx_mem.h"
//#include "vpx_util/endian_inl.h"

int r_bitcount = 0;

int vpx_reader_init(vpx_reader *r,
                    const uint8_t *buffer,
                    size_t size) {
  if (size && !buffer) {
    return 1;
  } else {
    r->buffer_end = buffer + size;
    r->buffer = buffer;
    r->value = 0;
    r->count = -8;
    r->range = 255;
    vpx_reader_fill(r);
    return vpx_read_bit(r) != 0;  // marker bit
  }
}

void vpx_reader_fill(vpx_reader *r) {
  const uint8_t *const buffer_end = r->buffer_end;
  const uint8_t *buffer = r->buffer;
  const uint8_t *buffer_start = buffer;
  BD_VALUE value = r->value;
  int count = r->count;
  const size_t bytes_left = buffer_end - buffer;
  const size_t bits_left = bytes_left * CHAR_BIT;
  int shift = BD_VALUE_SIZE - CHAR_BIT - (count + CHAR_BIT);

  if (bits_left > BD_VALUE_SIZE) {
      const int bits = (shift & 0xfffffff8) + CHAR_BIT;
      BD_VALUE nv;
      BD_VALUE big_endian_values;
      memcpy(&big_endian_values, buffer, sizeof(BD_VALUE));
      if (sizeof(BD_VALUE) == 8) {
          big_endian_values = htobe64(big_endian_values);
      } else {
        big_endian_values = htobe32(big_endian_values);
      }
      nv = big_endian_values >> (BD_VALUE_SIZE - bits);
      count += bits;
      buffer += (bits >> 3);
      value = r->value | (nv << (shift & 0x7));
  } else {
    const int bits_over = (int)(shift + CHAR_BIT - bits_left);
    int loop_end = 0;
    if (bits_over >= 0) {
      count += LOTS_OF_BITS;
      loop_end = bits_over;
    }

    if (bits_over < 0 || bits_left) {
      while (shift >= loop_end) {
        count += CHAR_BIT;
        value |= (BD_VALUE)*buffer++ << shift;
        shift -= CHAR_BIT;
      }
    }
  }

  // NOTE: Variable 'buffer' may not relate to 'r->buffer' after decryption,
  // so we increase 'r->buffer' by the amount that 'buffer' moved, rather than
  // assign 'buffer' to 'r->buffer'.
  r->buffer += buffer - buffer_start;
  r->value = value;
  r->count = count;
}

const uint8_t *vpx_reader_find_end(vpx_reader *r) {
  // Find the end of the coded buffer
  while (r->count > CHAR_BIT && r->count < BD_VALUE_SIZE) {
    r->count -= CHAR_BIT;
    r->buffer--;
  }
  return r->buffer;
}
