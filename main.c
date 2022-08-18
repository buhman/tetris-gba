#include "io_reg.h"
#include "vram.h"
#include "register_values.h"
#include "copy.h"
#include "nib.h"
#include "type.h"
#include "oam.h"

#include "tetris.h"
#include "render.h"
#include "input.h"
#include "block.h"
#include "obj_tet.h"
#include "glyph.h"
#include "osd.h"

struct frame frame = { 0 };

void _user_isr(void)
{
  io_reg.IME = 0;
  u32 ireq = io_reg.IF;

  input();

  if (frame.state == RUNNING) {
    tetris_tick();
    render_field();
    render_piece();
    render_queue();
    render_swap();
    osd_render(frame.points, frame.lines.total, frame.level);
  }

  io_reg.IF = ireq;
  io_reg.IME = IME__INT_MASTER_ENABLE;
}

void _start(void)
{
  init_palettes();
  frame.state = PRE_START;

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

  osd_labels();

  io_reg.DISPCNT =
    ( DISPCNT__BG0
    | DISPCNT__BG1
    | DISPCNT__BG2
    | DISPCNT__BG3
    | DISPCNT__OBJ
    | DISPCNT__OBJ_1_DIMENSION
    | DISPCNT__BG_MODE_0
    );

  io_reg.BG0CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(1)
    | BG_CNT__SCREEN_BASE_BLOCK(31)
    | BG_CNT__PRIORITY(0)
    );

  io_reg.BG1CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(30)
    | BG_CNT__PRIORITY(1)
    );

  io_reg.BG2CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(29)
    | BG_CNT__PRIORITY(2)
    );

  io_reg.BG3CNT =
    ( BG_CNT__COLOR_16_16
    | BG_CNT__SCREEN_SIZE(0)
    | BG_CNT__CHARACTER_BASE_BLOCK(0)
    | BG_CNT__SCREEN_BASE_BLOCK(28)
    | BG_CNT__PRIORITY(3)
    );

  *(volatile u32 *)(IWRAM_USER_ISR) = (u32)(&_user_isr);

  io_reg.DISPSTAT = DISPSTAT__V_BLANK_INT_ENABLE;
  io_reg.IE = IE__V_BLANK;
  io_reg.IF = (u16)-1;
  io_reg.IME = IME__INT_MASTER_ENABLE;

  while (1) {
    io_reg.HALTCNT = 0;
  };
}
