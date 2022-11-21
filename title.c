#include "io_reg.h"
#include "vram.h"
#include "register_values.h"
#include "copy.h"
#include "nib.h"
#include "type.h"

#include "res/title/layers.data.indexed.h"

static u16 palette[] = {
  //[0] = PRAM_RGB15( 0, 0, 0), ; transparency
  [1] = PRAM_RGB15(31,31,31), // text outline
  [2] = PRAM_RGB15(16, 3,12), //
  [3] = PRAM_RGB15(31,17,25), // text inner
  [4] = PRAM_RGB15(24, 3,12), //
  [5] = PRAM_RGB15(24, 9,25), //
  [15] = PRAM_RGB15(0, 0, 0), // black background
};

static inline void title_palettes(void)
{
  copy_16((void*)&pram.bg[0][0], &palette[0], (sizeof palette));
}

struct layer {
  u8 * chunks;
  u32 chunks_size;
  u16 * indexes;
  u8 c_width;
  u8 c_height;
};

static struct layer layers[] = {
  {
    .chunks = (u8*)&_binary_res_title_text_data_chunks_start,
    .chunks_size = (u32)&_binary_res_title_text_data_chunks_size,
    .indexes = (u16*)&_binary_res_title_text_data_indexes_start,
    .c_width = 23,
    .c_height = 12,
  },
  {
    .chunks = (u8*)&_binary_res_title_front_data_chunks_start,
    .chunks_size = (u32)&_binary_res_title_front_data_chunks_size,
    .indexes = (u16*)&_binary_res_title_front_data_indexes_start,
    .c_width = 32,
    .c_height = 32,
  },
  {
    .chunks = (u8*)&_binary_res_title_middle_data_chunks_start,
    .chunks_size = (u32)&_binary_res_title_middle_data_chunks_size,
    .indexes = (u16*)&_binary_res_title_middle_data_indexes_start,
    .c_width = 32,
    .c_height = 32,
  },
  {
    .chunks = (u8*)&_binary_res_title_back_data_chunks_start,
    .chunks_size = (u32)&_binary_res_title_back_data_chunks_size,
    .indexes = (u16*)&_binary_res_title_back_data_indexes_start,
    .c_width = 32,
    .c_height = 32,
  },
};

u32 title_layer(u32 ix, u32 offset, u32 screen_index, u32 bg)
{
  u16 vbuf;
  u32 block_offset = offset * TILE_16_LENGTH / 2;
  for (u32 c = 0; c < layers[ix].chunks_size; c++) {
    u8 pixel = layers[ix].chunks[c];
    if (pixel == 0) pixel = bg;
    switch (c % 4) {
    case 0: vbuf  = pixel;      break;
    case 1: vbuf |= pixel << 4; break;
    case 2: vbuf |= pixel << 8; break;
    case 3: vbuf |= pixel << 12;
      vram.character_block[0][block_offset + (c >> 2)] = vbuf;
      break;
    }
  }

  for (int cy = 0; cy < layers[ix].c_height; cy++) {
    for (int cx = 0; cx < layers[ix].c_width; cx++) {
      int i = cy * layers[ix].c_width + cx;
      int j = cy * 32 + cx;
      vram.screen_block[screen_index][j] = offset + layers[ix].indexes[i];
    }
  }

  return layers[ix].chunks_size / TILE_16_LENGTH;
}

static inline void bg_init(void)
{
  io_reg.BG0CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(31)
    | BG_CNT__PRIORITY(0)
    );

  io_reg.BG1CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(30)
    | BG_CNT__PRIORITY(0)
    );

  io_reg.BG2CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(29)
    | BG_CNT__PRIORITY(0)
    );

  io_reg.BG3CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(28)
    | BG_CNT__PRIORITY(0)
    );
}

void title_init(void)
{
  bg_init();
  title_palettes();

  fill_32((void*)&vram.character_block[0][0], nib_32(0), TILE_16_LENGTH);

  u32 offset = 1;
  offset += title_layer(0, offset, 31, 0);
  offset += title_layer(1, offset, 30, 0);
  offset += title_layer(2, offset, 29, 0);
  offset += title_layer(3, offset, 28, 15);

  io_reg.BG0HOFS = - ((240 - (layers[0].c_width * 8)) >> 1);
  io_reg.BG0VOFS = - ((160 - (layers[0].c_height * 8)) >> 1);
}

static u32 tick = 0;
void title_tick(void)
{
  tick += 1;
  io_reg.BG1HOFS = tick >> 2;

  io_reg.BG2HOFS = tick >> 3;
  io_reg.BG2VOFS = tick >> 3;

  io_reg.BG3HOFS = -(tick >> 4);
  io_reg.BG3VOFS = (tick >> 4);
}
