#include "type.h"
#include "vram.h"
#include "register_values.h"
#include "base10.h"
#include "copy.h"
#include "nib.h"

#include "tetris.h"

#define TITLE_OFFSET (2 * 256)  // length=512
#define LABEL_OFFSET (1 * 256)  // length=256
#define NUMBER_OFFSET (0 * 256) // length=256
#define LABEL_PALETTE (14)
#define NUMBER_PALETTE (14)
#define PAUSED_PALETTE (15)

#define COMBO_ROW 7
#define BEST_ROW 11
#define LEVEL_ROW 13
#define LINES_ROW 15
#define SCORE_ROW 17

struct label {
  u8 row;
  u8 col;
  u8 buf[14];
};

static void _label(struct label * labels, int ix, int len)
{
  for (int i = 0; i < len; i++) {
    vram.screen_block[31][(32 * labels[ix].row) + (labels[ix].col + i)] =
      ( SCREEN_TEXT__COLOR_PALETTE(LABEL_PALETTE)
      | SCREEN_TEXT__CHARACTER(LABEL_OFFSET + labels[ix].buf[i])
      );
  }
}

static void _clear_label(struct label * labels, int ix, int len)
{
  for (int i = 0; i < len; i++) {
    vram.screen_block[31][(32 * labels[ix].row) + (labels[ix].col + i)] = TITLE_OFFSET;
  }
}

#define SCORE_LABEL_LEN (6)
#define SCORE_LABEL_COUNT (5)

static struct label score_labels[SCORE_LABEL_COUNT] = {
  { COMBO_ROW, 1, "COMBO:", },
  { LEVEL_ROW, 1, "LEVEL:", },
  { LINES_ROW, 1, "LINES:", },
  { SCORE_ROW, 1, "SCORE:", },
  {  BEST_ROW, 1, " BEST:", },
};

void osd_score_labels(void)
{
  for (int j = 1; j < SCORE_LABEL_COUNT; j++) {
    _label(&score_labels[0], j, SCORE_LABEL_LEN);
  }
}

struct osd {
  u32 score;
  u32 lines;
  u32 level;
  u32 best;
  s32 combo;
};

static struct osd last = { -1, -1, -1, -1, -2 };

#define CLEAR_LINE(mem, len) (fill_32(mem, (0), len))

static void
_osd_uint(u32 row, u32 col, u32 value, u32 * last_value)
{
  if (value != *last_value) {
    void * mem = (void *)&(vram.screen_block[31][(32 * row) + col]);
    CLEAR_LINE(mem, 10 * 2);
    uint_to_base10(
      mem,
      SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE) | NUMBER_OFFSET,
      value,
      8
      );
    *last_value = value;
  }
}

static void
_osd_sint(u32 row, u32 col, s32 value, s32 * last_value)
{
  if (value != *last_value) {
    void * mem = (void *)&(vram.screen_block[31][(32 * row) + col]);
    CLEAR_LINE(mem, 10 * 2);
    if (value > 0)
      uint_to_base10(
        mem,
        SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE) | NUMBER_OFFSET,
        value,
        8
        );
    *last_value = value;
  }
}

void osd_render(u32 score, u32 lines, u32 level, u32 best, s32 combo)
{
  _osd_uint(SCORE_ROW + 1, 0, score, &last.score);
  _osd_uint(LEVEL_ROW + 1, 0, level, &last.level);
  _osd_uint(LINES_ROW + 1, 0, lines, &last.lines);
  _osd_uint(BEST_ROW + 1, 0, best, &last.best);

  // combo

#define L_COMBO (0)

  if (combo > 0) _label(&score_labels[0], L_COMBO, SCORE_LABEL_LEN);
  else if (combo != last.combo) _clear_label(&score_labels[0], 0, SCORE_LABEL_LEN);

  _osd_sint(COMBO_ROW + 1, 0, combo, &last.combo);
}

static inline void _text16(u8 * buf, u32 size, u32 row) {
  int t_x = ((15 - (size >> 1)));

  for (int i = 0; i < size; i++) {
    vram.screen_block[31][32 * row + t_x + i] =
      ( SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE)
      | SCREEN_TEXT__CHARACTER(TITLE_OFFSET + (buf[i] * 2))
      );
    vram.screen_block[31][32 * (row + 1) + t_x + i] =
      ( SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE)
      | SCREEN_TEXT__CHARACTER(TITLE_OFFSET + (buf[i] * 2 + 1))
      );
  }
}

static u8 title[] = "Ana's Tetris";
void osd_title(void)
{
  _text16(&title[0], (sizeof title), 8);
}

static u8 paused[] = "paused";
static u8 instructions[2][17] = {
  "`select' to reset",
  "`start' to resume",
};
void osd_paused(void)
{
#define PAUSED_ROW (13)
  _text16(&paused[0], (sizeof paused), PAUSED_ROW);

  int t_x = ((15 - (17 >> 1)));

  for (int j = 0; j < 2; j++) {
    for (int i = 0; i < 17; i++) {
      vram.screen_block[31][32 * (PAUSED_ROW + 3 + j * 2) + t_x + i] =
        ( SCREEN_TEXT__COLOR_PALETTE(PAUSED_PALETTE)
        | SCREEN_TEXT__CHARACTER(LABEL_OFFSET + instructions[j][i])
        );
    }
  }
}

static u8 topped_out[] = "topped out";
void osd_topped_out(void)
{
  _text16(&topped_out[0], (sizeof topped_out), 8);

  int t_x = ((15 - (17 >> 1)));

  for (int i = 0; i < 17; i++) {
    vram.screen_block[31][32 * (11 + 0 * 2) + t_x + i] =
      ( SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE)
      | SCREEN_TEXT__CHARACTER(LABEL_OFFSET + instructions[0][i])
      );
  }
}

static inline void osd_invalidate(void)
{
  last.score = -1;
  last.lines = -1;
  last.level = -1;
  last.best = -1;
  last.combo = -2;
}

void osd_clear(void)
{
  osd_invalidate();

  fill_32((void*)&vram.screen_block[31][0],
          half_32(SCREEN_TEXT__CHARACTER(0)),
          SCREEN_BASE_BLOCK_LENGTH);
}

struct menu {
  struct {
    s8 curr;
    s8 last;
  } item;
};

static struct menu menu = {
  .item = { 0, -1 },
};

#define C_RIGHT_ARROW (0x10)

#define MENU_LABEL_COUNT (2)
#define MENU_LABEL_LEN (10)

// must match `union options`
enum menu_label {
  L_DAS = 0,
  L_ARR = 1,
  L_LAST
};

static struct label menu_labels[MENU_LABEL_COUNT] = {
  { (L_DAS * 2) + 4, 10, "DAS:    ms" },
  { (L_ARR * 2) + 4, 10, "ARR:    ms" },
};

static u8 menu_value_dirty[L_LAST] = { 1, 1 };

static u8 t_options[] = "options";
void osd_menu_labels(void)
{
  _text16(&t_options[0], (sizeof t_options), 1);

  for (int j = 0; j < MENU_LABEL_COUNT; j++) {
    _label(&menu_labels[0], j, MENU_LABEL_LEN);
  }

  // also reset dirty
  fill_16(&menu_value_dirty[0], (1 << 8 | 1), 2);
  menu.item.curr = 0;
  menu.item.last = -1;
}

void osd_menu_render(void)
{
  if (menu.item.curr != menu.item.last) {
    vram.screen_block[31][(32 * ((menu.item.last * 2) + 4)) + 8] = 0;

    vram.screen_block[31][(32 * ((menu.item.curr * 2) + 4)) + 8] =
      ( SCREEN_TEXT__COLOR_PALETTE(LABEL_PALETTE)
      | SCREEN_TEXT__CHARACTER(LABEL_OFFSET + C_RIGHT_ARROW)
      );

    menu.item.last = menu.item.curr;
  }

  for (int i = 0; i < L_LAST; i++) {
    if (menu_value_dirty[i]) {
      void * mem = (void *)&(vram.screen_block[31][(32 * ((i * 2) + 4)) + 10 + 4]);
      fill_16(mem, (0), 4);
      uint_to_base10(mem,
                     SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE) | NUMBER_OFFSET,
                     options.option[i],
                     5
                     );
      menu_value_dirty[i] = 0;
    }
  }
}

void osd_menu_up(void)
{
  menu.item.curr -= 1;
  if (menu.item.curr < 0) menu.item.curr = MENU_LABEL_COUNT - 1;
}

void osd_menu_down(void)
{
  menu.item.curr += 1;
  if (menu.item.curr >= MENU_LABEL_COUNT) menu.item.curr = 0;
}

void osd_menu_left(void)
{
  if (options.option[menu.item.curr] > 0) {
    options.option[menu.item.curr] -= 1;
    menu_value_dirty[menu.item.curr] = 1;
  }
}

void osd_menu_right(void)
{
  if (options.option[menu.item.curr] < 255) {
    options.option[menu.item.curr] += 1;
    menu_value_dirty[menu.item.curr] = 1;
  }
}
