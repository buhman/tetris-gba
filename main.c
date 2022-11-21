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
#include "osd.h"
#include "music.h"
#include "title.h"

void _user_isr(void)
{
  io_reg.IME = 0;
  u32 ireq = io_reg.IF;

  if ((ireq & IE__TIMER_0) != 0) {
    ireq = IE__TIMER_0;
    if (frame.state != STATE_TITLE) music_tick();
  } else if ((ireq & IE__V_BLANK) != 0) {
    ireq = IE__V_BLANK;
    input();

    switch (frame.state) {
    case STATE_TITLE:
      title_tick();
      break;
    case STATE_RUNNING:
      tetris_tick();
      render_field();
      render_piece();
      render_queue();
      render_swap();
      osd_render(frame.points, frame.lines.total, frame.level, frame.best, frame.combo);
      break;
    case STATE_PAUSED:
      osd_menu_render();
      break;
    default: break;
    }
  }

  io_reg.IF = ireq;
  io_reg.IME = IME__INT_MASTER_ENABLE;
}

void _start(void)
{
  transition(STATE_TITLE);

  *(volatile u32 *)(IWRAM_USER_ISR) = (u32)(&_user_isr);

  io_reg.TM0CNT_L = (u16)-3581;

  io_reg.TM0CNT_H =
    ( TM_CNT_H__ENABLE
    | TM_CNT_H__INT_ENABLE
    | TM_CNT_H__PRESCALAR_64
    );

  io_reg.DISPSTAT = DISPSTAT__V_BLANK_INT_ENABLE;
  io_reg.IE = IE__V_BLANK | IE__TIMER_0;
  io_reg.IF = (u16)-1;
  io_reg.IME = IME__INT_MASTER_ENABLE;

  while (1) {
    io_reg.HALTCNT = 0;
  };
}
