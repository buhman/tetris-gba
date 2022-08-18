#include "type.h"
#include "tetris.h"
#include "xorshift.h"
#include "bag.h"

typedef struct count_flop {
  s8 count;
  u8 flop;
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
} input_t;

static inline void
key_input_count(count_flop_t * key, unsigned int input, unsigned int mask)
{
  if (input & mask) {
    if (key->count < 2)
      key->count += 1;
  } else {
    if (key->count > 0)
      key->count -= 1;
    if (key->count == 0)
      key->flop = 0;
  }
}

static inline bool
key_flopped(count_flop_t * key)
{
  if (key->count == 2 && key->flop == 0) {
    key->flop = 1;
    return true;
  } else {
    return false;
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

#define EVENT_LEFT (key_flopped(&_input.left))
#define EVENT_RIGHT (key_flopped(&_input.right))
#define EVENT_DOWN (key_flopped(&_input.down))
#define EVENT_SWAP (key_flopped(&_input.up))
#define EVENT_DROP (key_flopped(&_input.l))
#define EVENT_ROTATE_CW (key_flopped(&_input.a))
#define EVENT_ROTATE_CCW (key_flopped(&_input.b))

#define EVENT_START (key_flopped(&_input.start))

  if (EVENT_START) {
    tetris_reset_frame();
  }

  if (frame.state != RUNNING)
    return;

  if (EVENT_LEFT) {
    tetris_move((coord){-1, 0}, 0);
  }
  if (EVENT_RIGHT) {
    tetris_move((coord){1, 0}, 0);
  }
  if (EVENT_DOWN) {
    tetris_move((coord){0, 1}, 0);
  }
  if (EVENT_DROP) {
    tetris_drop();
  }
  if (EVENT_ROTATE_CW) {
    tetris_move((coord){0, 0}, 1);
  }
  if (EVENT_ROTATE_CCW) {
    tetris_move((coord){0, 0}, -1);
  }
  if (EVENT_SWAP) {
    tetris_swap();
  }
}
