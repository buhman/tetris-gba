#include "copy.h"
#include "nib.h"
#include "bag.h"

#include "tetris.h"

const coord offsets[7][4][4] = {
  [TET_Z] = {
    {{ 0, 0}, { 1, 0}, { 0, 1}, {-1, 1}},
    {{ 0, 0}, { 0,-1}, { 1, 0}, { 1, 1}},
    {{ 0, 0}, { 0,-1}, { 1,-1}, {-1, 0}},
    {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 1}},
  },
  [TET_L] = {
    {{ 0, 0}, {-1, 0}, { 1, 0}, { 1, 1}},
    {{ 0, 0}, { 0,-1}, { 1,-1}, { 0, 1}},
    {{ 0, 0}, {-1, 0}, { 1, 0}, {-1,-1}},
    {{ 0, 0}, { 0,-1}, { 0, 1}, {-1, 1}},
  },
  [TET_O] = {
    {{ 0, 0}, { 0, 1}, { 1, 0}, { 1, 1}},
    {{ 0, 0}, { 0,-1}, { 1, 0}, { 1,-1}},
    {{ 0, 0}, { 0,-1}, {-1, 0}, {-1,-1}},
    {{ 0, 0}, { 0, 1}, {-1, 0}, {-1, 1}},
  },
  [TET_S] = {
    {{ 0, 0}, {-1, 0}, { 0, 1}, { 1, 1}},
    {{ 0, 0}, { 0, 1}, { 1, 0}, { 1,-1}},
    {{ 0, 0}, { 1, 0}, { 0,-1}, {-1,-1}},
    {{ 0, 0}, { 0,-1}, {-1, 0}, {-1, 1}},
  },
  [TET_I] = {
    {{ 0, 0}, {-1, 0}, { 1, 0}, { 2, 0}},
    {{ 0, 0}, { 0, 1}, { 0,-1}, { 0,-2}},
    {{ 0, 0}, { 1, 0}, {-1, 0}, {-2, 0}},
    {{ 0, 0}, { 0,-1}, { 0, 1}, { 0, 2}},
  },
  [TET_J] = {
    {{ 0, 0}, { 1, 0}, {-1, 0}, {-1, 1}},
    {{ 0, 0}, { 0,-1}, { 0, 1}, { 1, 1}},
    {{ 0, 0}, {-1, 0}, { 1, 0}, { 1,-1}},
    {{ 0, 0}, { 0, 1}, { 0,-1}, {-1,-1}},
  },
  [TET_T] = {
    {{ 0, 0}, {-1, 0}, { 1, 0}, { 0, 1}},
    {{ 0, 0}, { 0, 1}, { 1, 0}, { 0,-1}},
    {{ 0, 0}, { 1, 0}, { 0,-1}, {-1, 0}},
    {{ 0, 0}, {-1, 0}, { 0,-1}, { 0, 1}},
  },
};

static u8 _to_next[] = {
  [0] = 10,
  [1] = 20,
  [2] = 30,
  [3] = 40,
  [4] = 50,
  [5] = 60,
  [6] = 70,
  [7] = 80,
  [8] = 90,
  [9 ... 15] = 100,
  [16] = 110,
  [17] = 120,
  [18] = 130,
  [19] = 140,
  [20] = 150,
  [21] = 160,
  [22] = 170,
  [23] = 180,
  [24] = 190,
  [25 ... 28] = 200,
};

static bool collision(piece * p)
{
  const coord * offset = &offsets[p->tet][p->dir][0];

  for (int i = 0; i < 4; i++) {
    int pu_o = p->pos.u + offset[i].u;
    int pv_o = p->pos.v + offset[i].v;

    struct cell * cell = &frame.field[pv_o][pu_o];

    if (cell->color != TET_EMPTY || pu_o == -1 || pu_o == COLUMNS || pv_o == -1 || pv_o == ROWS)
      return true;
  }
  return false;
}

static void update_drop_row(piece * piece)
{
  struct piece p;
  p.tet = piece->tet;
  p.pos.u = piece->pos.u;
  p.pos.v = piece->pos.v;
  p.dir = piece->dir;

  while (!collision(&p))
    p.pos.v += 1;
  piece->drop_row = p.pos.v - 1;
}

static inline void clear_field(void)
{
  fill_32((void*)&(frame.field[0][0]),
          byte_32(TET_EMPTY),
          (sizeof (frame.field)));
}

static void _next_piece(tet t)
{
  frame.piece.soft_drop = 0;
  frame.piece.tet = t;
  frame.piece.pos.u = 4;
  frame.piece.pos.v = 18;
  frame.piece.dir = DIR_UP;
  frame.piece.lock_delay.locking = false;
  frame.piece.lock_delay.ticks = 0;

  if (collision(&frame.piece)) {
    transition(STATE_TOPPED_OUT);
  }

  update_drop_row(&frame.piece);
}

static tet next_tet(void)
{
  return bag_next_piece();
}

void tetris_reset_frame(void)
{
  if ( _save.magic[0] != 'M' || _save.magic[1] != 'A'
    || _save.magic[2] != 'G' || _save.magic[3] != '0') {
    _save.magic[0] = 'M'; _save.magic[1] = 'A';
    _save.magic[2] = 'G'; _save.magic[3] = '0';

    for (int i = 0; i < 4; i++)
      ((u8*)&_save.best)[i] = 0;
    frame.best = 0;
  } else {
    for (int i = 0; i < 4; i++)
      ((u8*)&frame.best)[i] = ((u8*)&_save.best)[i];
  }

  clear_field();
  frame.combo = -1;
  frame.ticks = 0;
  frame.level = 0;
  frame.points = 0;
  frame.lines.total = 0;
  frame.lines.to_next = _to_next[0];
  frame.hold.piece = TET_EMPTY;
  frame.hold.swapped = false;

  bag_reset();
  _next_piece(next_tet());
}

typedef coord kick_table_t[4][KICKS];

static const kick_table_t zlsjt_kick = {
  {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}}, // up
  {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}}, // right
  {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}}, // down
  {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}}, // left
};

static const kick_table_t i_kick = {
  {{ 0, 0}, {-1, 0}, { 2, 0}, {-1, 0}, { 2, 0}},
  {{-1, 0}, { 0, 0}, { 0, 0}, { 0, 1}, { 0,-2}},
  {{-1, 1}, { 1, 1}, {-2, 1}, { 1, 0}, {-2, 0}},
  {{ 0, 1}, { 0, 1}, { 0, 1}, { 0,-1}, { 0, 2}},
};

static const kick_table_t o_kick = {
  {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
  {{ 0,-1}, { 0,-1}, { 0,-1}, { 0,-1}, { 0,-1}},
  {{-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}},
  {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}},
};

static inline const kick_table_t * kick_offsets(tet t)
{
  switch (t) {
  case TET_I:
    return &i_kick;
  case TET_O:
    return &o_kick;
  default:
    return &zlsjt_kick;
  }
}

bool tetris_move(coord offset, int rotation)
{
  piece p;

  p.tet = frame.piece.tet;
  p.dir = (frame.piece.dir + rotation) & 3;

  for (int kick = 0; kick < KICKS; kick++) {
    p.pos.u = frame.piece.pos.u + offset.u;
    p.pos.v = frame.piece.pos.v + offset.v;

    if (rotation != 0) {
      const coord * k_offset_a = &(*kick_offsets(frame.piece.tet))[frame.piece.dir][kick];
      const coord * k_offset_b = &(*kick_offsets(frame.piece.tet))[p.dir][kick];
      int kick_u = k_offset_a->u - k_offset_b->u;
      int kick_v = k_offset_a->v - k_offset_b->v;
      p.pos.u += kick_u;
      p.pos.v += kick_v;
    }

    if (collision(&p)) {
      if (rotation == 0)
        return false;
      else
        continue;
    } else {
      frame.piece.pos.u = p.pos.u;
      frame.piece.pos.v = p.pos.v;
      frame.piece.dir = p.dir;
      if (offset.u || rotation)
        update_drop_row(&frame.piece);

      frame.piece.lock_delay.ticks = 0;

      return true;
    }
  }
  return false;
}

static u8 _gravity[30] = {
  [0] = 48,
  [1] = 43,
  [2] = 38,
  [3] = 33,
  [4] = 28,
  [5] = 23,
  [6] = 18,
  [7] = 13,
  [8] = 8,
  [9] = 6,
  [10 ... 12] = 5,
  [13 ... 15] = 4,
  [16 ... 18] = 3,
  [19 ... 28] = 2,
  [29] = 1
};

static inline bool gravity(void)
{
  if (frame.ticks >= _gravity[frame.level]) {
    frame.ticks = 0;
    return true;
  } else {
    return false;
  }
}

static inline int _clear_lines(void)
{
  int cleared = 0;
  u8 seen[ROWS];
  u8 rows[ROWS];
  fill_32((void*)&seen[0], 0, ROWS);
  fill_32((void*)&rows[0], 0, ROWS);

  for (int i = 0; i < 4; i++) {
    const coord * offset = offsets[frame.piece.tet][frame.piece.dir];
    int pv_o = frame.piece.pos.v + offset[i].v;
    if (seen[pv_o])
      continue;

    seen[pv_o] = 1;

    for (int u = 0; u < COLUMNS; u++) {
      if (frame.field[pv_o][u].color == TET_EMPTY)
        goto next_line;
    }

    cleared += 1;
    rows[pv_o] = 1;

  next_line:
    (void)0;
  }

  int off = 0;
  for (int row = (ROWS - 1); row >= 0; row--) {
    while (row - off >= 0 && rows[row - off] != 0) {
      off++;
    }

    for (int col = 0; col < COLUMNS; col++) {
      if ((row - off) < 0)
        frame.field[row][col].color = TET_EMPTY;
      else
        frame.field[row][col].color = frame.field[row - off][col].color;
    }
  }

  return cleared;
}

static inline void _place(void)
{
  for (int i = 0; i < 4; i++) {
    const coord * offset = offsets[frame.piece.tet][frame.piece.dir];
    int po_u = frame.piece.pos.u + offset[i].u;
    int po_v = frame.piece.pos.v + offset[i].v;

    frame.field[po_v][po_u].color = frame.piece.tet;
  }
}

static u8 _base_points[5] = {1, 3, 5, 8};

static inline int points(int soft_drop, int hard_drop, int cleared, int combo)
{
  int p = 0;
  if (cleared > 0)
    p += _base_points[--cleared] * (frame.level + 1) * 100;

  p += hard_drop * 2;
  p += soft_drop * 1;

  if (combo > 0)
    p += combo * 50 * frame.level;

  return p;
}

#define max(a,b)               \
  ({ __typeof__ (a) _a = (a);  \
    __typeof__ (b) _b = (b);   \
    _a > _b ? _a : _b; })

static inline void next_level(int cleared)
{
  frame.lines.to_next -= cleared;

  if (frame.lines.to_next <= 0) {
    frame.level += 1;
    frame.lines.to_next = _to_next[frame.level] + frame.lines.to_next;
  }
}

static void _drop(void)
{
  frame.hold.swapped = false;
  int hard_drop = max(frame.piece.drop_row - frame.piece.pos.v, 0);
  frame.piece.pos.v = frame.piece.drop_row;

  _place();

  int cleared = _clear_lines();
  if (cleared == 0) frame.combo = -1;
  else frame.combo += 1;

  frame.points += points(frame.piece.soft_drop, hard_drop, cleared, frame.combo);
  if (frame.points > frame.best) {
    for (int i = 0; i < 4; i++)
      ((u8*)&_save.best)[i] = ((u8*)&frame.points)[i];
  }
  frame.lines.total += cleared;
  next_level(cleared);
}

bool _topped_out(piece * p)
{
  const coord * offset = &offsets[p->tet][p->dir][0];
  for (int i = 0; i < 4; i++) {
    int pv_o = p->drop_row + offset[i].v;
    if (pv_o >= 20)
      return false;
  }
  return true;
}

void tetris_drop(void)
{
  frame.hold.swapped = false;

  if (_topped_out(&frame.piece))
    transition(STATE_TOPPED_OUT);

  _drop();
  _next_piece(next_tet());
}

int lock_delay_should_drop(void)
{
  if (frame.piece.lock_delay.locking) {
    if (frame.piece.lock_delay.ticks++ >= 30)
      return true;
    else {
      return false;
    }
  }
  return false;
}

void tetris_tick(void)
{
  frame.ticks += 1;

  if (gravity()) {
    if (tetris_move((coord){0, 1}, 0)) {
    } else {
      frame.piece.lock_delay.locking = true;
    }
  }

  if (lock_delay_should_drop()) {
    tetris_drop();
  }
}

void tetris_swap(void)
{
  if (frame.hold.swapped) return;
  frame.hold.swapped = true;
  tet swap = frame.hold.piece;
  frame.hold.piece = frame.piece.tet;
  if (swap == TET_EMPTY)
    _next_piece(next_tet());
  else
    _next_piece(swap);
}
