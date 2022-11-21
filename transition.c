#include "io_reg.h"
#include "register_values.h"

#include "tetris.h"
#include "transition.h"
#include "osd.h"
#include "title.h"
#include "render.h"
#include "game.h"

typedef void (*transition_t)(void);

static void transition_title(void)
{
  //osd_clear();
  //osd_title();
  title_init();

  io_reg.DISPCNT =
    ( DISPCNT__BG0
    | DISPCNT__BG1
    | DISPCNT__BG2
    | DISPCNT__BG3
    | DISPCNT__BG_MODE_0
    );  
}

static void transition_reset(void)
{
  game_init();
  
  tetris_reset_frame();
  transition(STATE_RUNNING);
}

static void transition_running(void)
{
  osd_clear();
  osd_score_labels();

  io_reg.DISPCNT =
    ( DISPCNT__BG0
    | DISPCNT__BG3
    | DISPCNT__OBJ
    | DISPCNT__OBJ_1_DIMENSION
    | DISPCNT__BG_MODE_0
    );
}

static void transition_paused(void)
{
  osd_clear();
  osd_paused();
  osd_menu_labels();

  io_reg.DISPCNT =
    ( DISPCNT__BG0
      //| DISPCNT__BG1
    | DISPCNT__BG_MODE_0
    );
}

static void transition_topped_out(void)
{
  osd_topped_out();

  io_reg.DISPCNT =
    ( DISPCNT__BG0
    | DISPCNT__BG1
    | DISPCNT__BG3
    | DISPCNT__OBJ
    | DISPCNT__OBJ_1_DIMENSION
    | DISPCNT__BG_MODE_0
    );
}

static transition_t _transitions[STATE_LAST] = {
  [STATE_TITLE] = &transition_title,
  [STATE_RESET] = &transition_reset,
  [STATE_RUNNING] = &transition_running,
  [STATE_PAUSED] = &transition_paused,
  [STATE_TOPPED_OUT] = &transition_topped_out,
};

void transition(enum state state)
{
  if (frame.state == STATE_PAUSED)
    tetris_save_options();

  frame.state = state;
  (*_transitions[state])();
}
