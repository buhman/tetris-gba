#include "type.h"
#include "type.h"
#include "vram.h"
#include "register_values.h"
#include "base10.h"
#include "copy.h"

#define LABEL_OFFSET (1 * 256)
#define NUMBER_OFFSET (0 * 256)
#define LABEL_PALETTE (14)
#define NUMBER_PALETTE (15)

#define LABEL_LEN (6)
#define LABELS (4)

#define BEST_ROW 8
#define LEVEL_ROW 13
#define LINES_ROW 15
#define SCORE_ROW 17

static struct {
  u8 buf[6];
  u8 row;
} labels[LABELS] = {
  { "LEVEL:", LEVEL_ROW },
  { "LINES:", LINES_ROW },
  { "SCORE:", SCORE_ROW },
  { " BEST:", BEST_ROW },
};


void osd_labels(void)
{
  for (int i = 0; i < LABEL_LEN; i++) {
    for (int j = 0; j < LABELS; j++) {
      vram.screen_block[31][(32 * labels[j].row) + (1+i)] =
        ( SCREEN_TEXT__COLOR_PALETTE(LABEL_PALETTE)
        | SCREEN_TEXT__CHARACTER(LABEL_OFFSET + labels[j].buf[i])
        );
    }
  }
}

struct osd {
  u32 score;
  u32 lines;
  u32 level;
  u32 best;
};

static struct osd last = { -1, -1, -1, -1 };

static inline void
_osd_int(u32 row, u32 value, u32 * last_value)
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

void osd_render(u32 score, u32 lines, u32 level, u32 best)
{
  _osd_int(SCORE_ROW, score, &last.score);
  _osd_int(LEVEL_ROW, level, &last.level);
  _osd_int(LINES_ROW, lines, &last.lines);
  _osd_int(BEST_ROW, best, &last.best);
}
