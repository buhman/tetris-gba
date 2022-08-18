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
#define LABELS (3)

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
};

static struct osd last = { -1, -1, -1 };

void osd_render(u32 score, u32 lines, u32 level)
{
  if (score != last.score) {
    void * mem = (void *)&(vram.screen_block[31][(32 * (SCORE_ROW + 1))]);
    fill_32(mem, 0, 10 * 2);
    uint_to_base10(
      mem,
      SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE) | NUMBER_OFFSET,
      score,
      8
      );
  }

  if (level != last.level) {
    void * mem = (void *)&(vram.screen_block[31][(32 * (LEVEL_ROW + 1))]);
    fill_32(mem, 0, 10 * 2);
    uint_to_base10(
      mem,
      SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE) | NUMBER_OFFSET,
      level,
      8
      );
  }

  if (lines != last.lines) {
    void * mem = (void *)&(vram.screen_block[31][(32 * (LINES_ROW + 1))]);
    fill_32(mem, 0, 10 * 2);
    uint_to_base10(
      mem,
      SCREEN_TEXT__COLOR_PALETTE(NUMBER_PALETTE) | NUMBER_OFFSET,
      lines,
      8
      );
  }
}
