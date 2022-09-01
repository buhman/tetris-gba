#include "type.h"
#include "tetris.h"
#include "xorshift.h"
#include "bag.h"
#include "transition.h"
#include "osd.h"

typedef struct count_flop {
  s8 count;
  u8 flop;
  u8 das;
  u8 repeat;
} count_flop_t;

typedef struct input {
  count_flop_t left;
  count_flop_t right;
  count_flop_t up;
  count_flop_t down;
  count_flop_t a;
  count_flop_t b;
  count_flop_t l;
  count_flop_t r;
  count_flop_t start;
  count_flop_t select;
} input_t;

#define KEY_FLIP_COUNT (2)

static inline void
key_input_count(count_flop_t * key, unsigned int input, unsigned int mask)
{
  if (input & mask) {
    if (key->count < KEY_FLIP_COUNT)
      key->count += 1;
    else
      key->das += 1;
  } else {
    if (key->count == 0) {
      key->flop = 0;
      key->das = 0;
      key->repeat = 0;
    }
    else if (key->count > 0)
      key->count -= 1;
  }
}

static inline u8
key_flopped(count_flop_t * key)
{
  if (key->count == KEY_FLIP_COUNT && key->flop == 0) {
    key->flop = 1;
    return 1;
  } else if (key->flop == 1 && key->das == options.das && key->repeat == 0) {
    key->repeat = 1;
    key->das = 0;
    return 2;
  } else if (key->repeat == 1 && (key->das == options.arr)) {
    key->das = 0;
    return 2;
  } else {
    return 0;
  }
}

static input_t _input = { 0 };
static u16 last_key_input = 0;
static u32 tick;

static inline void
input(void)
{
  tick += 1;

  u16 key_input = ~(io_reg.KEY_INPUT);
  if (key_input != last_key_input) bag_add_entropy(tick);
  last_key_input = key_input;

  key_input_count(&_input.left, key_input, KEYCNT__INPUT_LEFT);
  key_input_count(&_input.right, key_input, KEYCNT__INPUT_RIGHT);
  key_input_count(&_input.up, key_input, KEYCNT__INPUT_UP);
  key_input_count(&_input.down, key_input, KEYCNT__INPUT_DOWN);
  key_input_count(&_input.a, key_input, KEYCNT__INPUT_A);
  key_input_count(&_input.b, key_input, KEYCNT__INPUT_B);
  key_input_count(&_input.l, key_input, KEYCNT__INPUT_L);
  key_input_count(&_input.r, key_input, KEYCNT__INPUT_R);
  key_input_count(&_input.start, key_input, KEYCNT__INPUT_ST);
  key_input_count(&_input.select, key_input, KEYCNT__INPUT_SL);

  // != 0 is repeating
  // == 1 is non-repeating
#define EVENT_START (key_flopped(&_input.start) == 1)
#define EVENT_CONFIRM (key_flopped(&_input.select) == 1)

  if (EVENT_START) {
    switch (frame.state) {
    case STATE_RUNNING: transition(STATE_PAUSED); break;
    case STATE_PAUSED: transition(STATE_RUNNING); break;
    case STATE_TOPPED_OUT: break;
    default: transition(STATE_RESET); break;
    }
  }
  if (EVENT_CONFIRM) {
    switch (frame.state) {
    case STATE_RUNNING: transition(STATE_PAUSED); break;
    case STATE_PAUSED:
    case STATE_TOPPED_OUT: transition(STATE_RESET); break;
    default: break;
    }
  }

  switch (frame.state) {

  // STATE_RUNNING

#define EVENT_TET_LEFT (key_flopped(&_input.left) != 0)
#define EVENT_TET_RIGHT (key_flopped(&_input.right) != 0)
#define EVENT_TET_DOWN (key_flopped(&_input.down) != 0)
#define EVENT_SWAP (key_flopped(&_input.up) == 1)
#define EVENT_DROP (key_flopped(&_input.l) == 1)
#define EVENT_ROTATE_CW (key_flopped(&_input.a) == 1)
#define EVENT_ROTATE_CCW (key_flopped(&_input.b) == 1)

  case STATE_RUNNING:
    if (EVENT_TET_LEFT  )   tetris_move((coord){-1, 0}, 0);
    if (EVENT_TET_RIGHT )   tetris_move((coord){1, 0}, 0);
    if (EVENT_TET_DOWN  ) { tetris_move((coord){0, 1}, 0); frame.piece.soft_drop += 1; }
    if (EVENT_ROTATE_CW )   tetris_move((coord){0, 0}, 1);
    if (EVENT_ROTATE_CCW)   tetris_move((coord){0, 0}, -1);
    if (EVENT_SWAP      )   tetris_swap();
    if (EVENT_DROP      )   tetris_drop();
    break;



  // STATE_PAUSED

#define EVENT_LEFT (key_flopped(&_input.left) == 1)
#define EVENT_RIGHT (key_flopped(&_input.right) == 1)
#define EVENT_DOWN (key_flopped(&_input.down) == 1)
#define EVENT_UP (key_flopped(&_input.up) == 1)

  case STATE_PAUSED:
    if (EVENT_UP        )   osd_menu_up();
    if (EVENT_DOWN      )   osd_menu_down();
    if (EVENT_LEFT      )   osd_menu_left();
    if (EVENT_RIGHT     )   osd_menu_right();
    break;

  default: break;
  }
}
