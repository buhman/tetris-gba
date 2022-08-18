#include "type.h"

#include "tetris.h"
#include "bag.h"
#include "xorshift.h"

static xorshift_state_t rand_state =  {
  .a = 0x1337beef, .e = 0,
};

void bag_add_entropy(u32 e)
{
  xorshift_add_entropy(&rand_state, e);
}

static inline int _next_piece(void)
{
  int random_index = xorshift_rand(&rand_state, frame.bag.count);
  int bag_index = 0;

  for (int tet = 0; tet < 7; tet++) {
    if (frame.bag.mask & (1 << tet)) {
      if (bag_index == random_index) {
        frame.bag.mask &= ~(1 << tet);
        frame.bag.count -= 1;
        if (frame.bag.mask == 0) {
          frame.bag.mask = 0x7f;
          frame.bag.count = 7;
        }
        return tet;
      }

      bag_index += 1;
    }
  }

  return -1;
}

static inline int _bag_next(int tet)
{
  int t = frame.queue.tet[frame.queue.index];

  frame.queue.tet[frame.queue.index] = tet;

  frame.queue.index += 1;
  if (frame.queue.index >= BAG_QUEUE_LEN)
    frame.queue.index = 0;

  return t;
}

int bag_next_piece(void)
{
  return _bag_next(_next_piece());
}

void bag_reset(void)
{
  frame.bag.mask = 0x7f;
  frame.bag.count = 7;
  frame.queue.index = 0;

  for (int i = 0; i < BAG_QUEUE_LEN; i++) {
    _bag_next(_next_piece());
  }
}
