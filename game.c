#include "io_reg.h"
#include "vram.h"
#include "register_values.h"
#include "copy.h"
#include "nib.h"
#include "type.h"

#include "render.h"
#include "glyph.h"
#include "obj_tet.h"
#include "block.h"
#include "music.h"

void game_init(void)
{
  // OSD text
  io_reg.BG0CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(1)
    | BG_CNT__SCREEN_BASE_BLOCK(31)
    | BG_CNT__PRIORITY(0)
    );

  // pause/title background
  io_reg.BG1CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(30)
    | BG_CNT__PRIORITY(1)
    );

  // unused
  io_reg.BG2CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(29)
    | BG_CNT__PRIORITY(2)
    );

  // tetris field and sidebar background
  io_reg.BG3CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(28)
    | BG_CNT__PRIORITY(3)
    );
  
  init_palettes();
  
  fill_32((void*)&vram.character_block[0][tile(0)],
          nib_32(0),
          TILE_16_LENGTH);

  fill_32((void*)&vram.character_block[0][tile(1)],
          nib_32(1),
          TILE_16_LENGTH);

  block_tile((void*)&vram.character_block[0][tile(2)], 1);

  block_end_tile((void*)&vram.character_block[0][tile(3)], 1);

  obj_tet_init();

  render_static_backgrounds();
  render_init_queue();

  pram.bg[14][1] = PRAM_RGB15(25, 25, 25);
  pram.bg[15][1] = PRAM_RGB15(31, 31, 31);
  glyph_init(1, 0, 0);

  music_init();

  io_reg.BG0VOFS = 0;
  io_reg.BG0HOFS = 0;

  io_reg.BG1VOFS = 0;
  io_reg.BG1HOFS = 0;

  io_reg.BG2VOFS = 0;
  io_reg.BG2HOFS = 0;

  io_reg.BG3VOFS = 0;
  io_reg.BG3HOFS = 0;
}
