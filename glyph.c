#include "vram.h"
#include "type.h"

#include "res/font.glyph.h"

#include "glyph.h"

struct start_size {
  u32 size;
  u8 * start;
};

static struct start_size fonts[] = {
  {
    (u32)&_binary_res_Bm437_CompaqThin_8x8_glyph_size,
    (u8 *)&_binary_res_Bm437_CompaqThin_8x8_glyph_start
  },
  {
    (u32)&_binary_res_BmPlus_ToshibaSat_8x8_glyph_size,
    (u8 *)&_binary_res_BmPlus_ToshibaSat_8x8_glyph_start
  },
  {
    (u32)&_binary_res_Bm437_CL_EagleIII_8x16_glyph_size,
    (u8 *)&_binary_res_Bm437_CL_EagleIII_8x16_glyph_start
  }
};

static inline u32 glyph_init_8x8(u32 fg_color, u32 bg_color, u32 font, u32 offset)
{
  u32 * block = (u32 *)&vram.character_block[1][offset];

  u32 index = 0;
  for (u32 i = 0; i < fonts[font].size; i++) {
    u32 row_in = fonts[font].start[i];
    u32 row_out = 0;
    for (u32 bit = 0; bit < 8; bit++)
      row_out |= (((row_in >> bit) & 1) ? fg_color : bg_color) << (bit * 4);

    block[index++] = row_out;
  }
  return fonts[font].size * 4;
}

u32 glyph_init(u32 fg_color, u32 bg_color, u32 offset)
{
  for (u32 i = 0; i < ((sizeof fonts) / (sizeof fonts[0])); i++) {
    offset += glyph_init_8x8(fg_color, bg_color, i, offset);
  }
  return offset;
}
