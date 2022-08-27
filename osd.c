#include "type.h"
#include "vram.h"
#include "register_values.h"
#include "base10.h"
#include "copy.h"
#include "nib.h"

#define TITLE_OFFSET (2 * 256)  // length=512
#define LABEL_OFFSET (1 * 256)  // length=256
#define NUMBER_OFFSET (0 * 256) // length=256
#define LABEL_PALETTE (14)
#define NUMBER_PALETTE (15)

#define LABEL_LEN (6)
#define LABELS (5)

#define COMBO_ROW 7
#define BEST_ROW 11
#define LEVEL_ROW 13
#define LINES_ROW 15
#define SCORE_ROW 17

static struct {
  u8 buf[6];
  u8 row;
} labels[LABELS] = {
  { "COMBO:", COMBO_ROW },
  { "LEVEL:", LEVEL_ROW },
  { "LINES:", LINES_ROW },
  { "SCORE:", SCORE_ROW },
  { " BEST:", BEST_ROW },
};


static void _label(int j)
{
  for (int i = 0; i < LABEL_LEN; i++) {
    vram.screen_block[31][(32 * labels[j].row) + (1+i)] =
      ( SCREEN_TEXT__COLOR_PALETTE(LABEL_PALETTE)
      | SCREEN_TEXT__CHARACTER(LABEL_OFFSET + labels[j].buf[i])
      );
  }
}

static void _clear_label(int j)
{
  for (int i = 0; i < LABEL_LEN; i++) {
    vram.screen_block[31][(32 * labels[j].row) + (1+i)] = 0;
  }
}

void osd_labels(void)
{
  for (int j = 1; j < LABELS; j++) {
    _label(j);
  }

  static u8 ana[] = "ana";
  for (int j = 0; j < 3; j++) {
    vram.screen_block[31][(32 * 19) + (27+j)] =
      ( SCREEN_TEXT__COLOR_PALETTE(LABEL_PALETTE)
      | SCREEN_TEXT__CHARACTER(LABEL_OFFSET + ana[j])
      );
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

static void
_osd_uint(u32 row, u32 value, u32 * last_value)
{
  if (value != *last_value) {
    void * mem = (void *)&(vram.screen_block[31][(32 * (row + 1))]);
    fill_32(mem, 0, 10 * 2);
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
_osd_sint(u32 row, s32 value, s32 * last_value)
{
  if (value != *last_value) {
    void * mem = (void *)&(vram.screen_block[31][(32 * (row + 1))]);
    fill_32(mem, 0, 10 * 2);
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
  _osd_uint(SCORE_ROW, score, &last.score);
  _osd_uint(LEVEL_ROW, level, &last.level);
  _osd_uint(LINES_ROW, lines, &last.lines);
  _osd_uint(BEST_ROW, best, &last.best);

  // combo

  if (combo > 0) _label(0);
  else if (combo != last.combo) _clear_label(0);

  _osd_sint(COMBO_ROW, combo, &last.combo);
}

static inline void _text16(u8 * buf, u32 size) {
  int t_x = ((15 - (size >> 1)));

  for (int i = 0; i < size; i++) {
    vram.screen_block[30][32 * 8 + t_x + i] =
      ( SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE)
      | SCREEN_TEXT__CHARACTER(TITLE_OFFSET + (buf[i] * 2))
      );
    vram.screen_block[30][32 * 9 + t_x + i] =
      ( SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE)
      | SCREEN_TEXT__CHARACTER(TITLE_OFFSET + (buf[i] * 2 + 1))
      );
  }
}

static u8 title[] = "Ana's Tetris";
void osd_title(void)
{
  _text16(&title[0], (sizeof title));
}

static u8 paused[] = "paused";
static u8 instructions[2][17] = {
  "`select' to reset",
  "`start' to resume",
};
void osd_paused(void)
{
  _text16(&paused[0], (sizeof paused));

  int t_x = ((15 - (17 >> 1)));

  for (int j = 0; j < 2; j++) {
    for (int i = 0; i < 17; i++) {
      vram.screen_block[30][32 * (11 + j * 2) + t_x + i] =
        ( SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE)
        | SCREEN_TEXT__CHARACTER(LABEL_OFFSET + instructions[j][i])
        );
    }
  }
}

static u8 topped_out[] = "topped out";
void osd_topped_out(void)
{
  _text16(&topped_out[0], (sizeof topped_out));

  int t_x = ((15 - (17 >> 1)));

  for (int i = 0; i < 17; i++) {
    vram.screen_block[30][32 * (11 + 0 * 2) + t_x + i] =
      ( SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE)
      | SCREEN_TEXT__CHARACTER(LABEL_OFFSET + instructions[0][i])
      );
  }
}

void osd_clear(void)
{
  fill_32((void*)&vram.screen_block[30][0],
          half_32(SCREEN_TEXT__CHARACTER(0)),
          SCREEN_BASE_BLOCK_LENGTH);
}
