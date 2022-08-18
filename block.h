#pragma once

#include "type.h"

#include "res/block.data.h"

static inline void
_tile(void * src, void * dst, u8 value)
{
  /* tetris block */
  u16 buf;
  for (int i = 0; i < 8*8; i++) {
    u8 pixel = ((u8*)src)[i] * value;
    switch (i % 4) {
    case 0: buf  = pixel;      break;
    case 1: buf |= pixel << 4; break;
    case 2: buf |= pixel << 8; break;
    case 3: buf |= pixel << 12;
      ((u16*)dst)[i >> 2] = buf;
      break;
    }
  }
}

static inline void
block_tile(void * dst, u8 value)
{
  _tile((void *)&_binary_res_block_data_start, dst, value);
}

static inline void
block_end_tile(void * dst, u8 value)
{
  _tile((void *)&_binary_res_block_end_data_start, dst, value);
}
