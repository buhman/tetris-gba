#pragma once

#include <assert.h>

#include "type.h"
#include "base.h"
#include "transition.h"

typedef enum tet {
  TET_Z,
  TET_L,
  TET_O,
  TET_S,
  TET_I,
  TET_J,
  TET_T,
  TET_EMPTY,
  TET_LAST,
} tet;
static_assert((sizeof (enum tet)) == 1);

typedef enum dir {
  DIR_UP = 0,
  DIR_RIGHT = 1,
  DIR_DOWN = 2,
  DIR_LEFT = 3,
  DIR_LAST = 4,
} dir;
static_assert((sizeof (enum dir)) == 1);

typedef enum event {
  EVENT_LEFT,
  EVENT_RIGHT,
  EVENT_DOWN,
  EVENT_DROP,
  EVENT_SPIN_CW,
  EVENT_SPIN_CCW,
  EVENT_SPIN_180,
  EVENT_SWAP,
} event;
static_assert((sizeof (enum event)) == 1);

typedef struct cell {
  tet color;
} cell;
static_assert((sizeof (struct cell)) == 1);

typedef struct coord {
  s8 u;
  s8 v;
} coord;

typedef struct piece {
  tet tet;   // 1
  dir dir;   // 1
  coord pos; // 2
  s8 drop_row; // 1
  u8 soft_drop; // 1
  struct {
    u8 locking;
    u8 ticks;
  } lock_delay;
} piece;

#define COLUMNS (10)
#define ROWS (40)
#define KICKS (5)

typedef struct cell field[ROWS][COLUMNS];
static_assert((sizeof (field)) == (ROWS * COLUMNS));

#define BAG_QUEUE_LEN (6)

typedef struct bag {
  u8 mask;
  u8 count;
} bag;

typedef struct queue {
  u8 index;
  tet tet[BAG_QUEUE_LEN];
} queue;

struct frame {
  field field;
  u32 points;
  u32 best;
  s32 combo;
  struct {
    s32 to_next;
    u32 total;
  } lines;
  u32 level;
  u32 ticks;
  // begin unaligned
  bag bag;
  queue queue;
  piece piece;
  struct {
    tet piece;
    u8 swapped;
  } hold;
  state state;
};

struct save {
  u8 magic[4];
  u32 best;
};

extern struct frame frame;

extern const coord offsets[7][4][4];

extern struct save _save;

/* functions */

void tetris_reset_frame(void);

bool tetris_move(coord offset, int rotation);

void tetris_tick(void);

void tetris_drop(void);

void tetris_swap(void);
