#include "vram.h"
#include "copy.h"
#include "nib.h"
#include "register_values.h"
#include "oam.h"

#include "tetris.h"
#include "obj_tet.h"

static const u16 colors[] = {
  [TET_Z] = PRAM_RGB15(29, 4, 5),
  [TET_L] = PRAM_RGB15(29, 15, 4),
  [TET_O] = PRAM_RGB15(31, 26, 0),
  [TET_S] = PRAM_RGB15(8, 22, 8),
  [TET_I] = PRAM_RGB15(6, 24, 29),
  [TET_J] = PRAM_RGB15(11, 12, 21),
  [TET_T] = PRAM_RGB15(21, 9, 19),
  [TET_EMPTY] = PRAM_RGB15(0, 0, 0),
  [TET_LAST] = PRAM_RGB15(6, 6, 6),
};

static const u16 colors_lock[] = {
  [TET_Z] = PRAM_RGB15(31, 6, 7),
  [TET_L] = PRAM_RGB15(31, 17, 6),
  [TET_O] = PRAM_RGB15(31, 26, 2),
  [TET_S] = PRAM_RGB15(10, 24, 10),
  [TET_I] = PRAM_RGB15(8, 26, 31),
  [TET_J] = PRAM_RGB15(13, 14, 23),
  [TET_T] = PRAM_RGB15(23, 11, 21),
  [TET_EMPTY] = PRAM_RGB15(2, 2, 2),
  [TET_LAST] = PRAM_RGB15(8, 8, 8),
};

static const u16 colors_ghost[] = {
  [TET_Z] = PRAM_RGB15(10, 1, 2),
  [TET_L] = PRAM_RGB15(10, 5, 1),
  [TET_O] = PRAM_RGB15(10, 9, 0),
  [TET_S] = PRAM_RGB15(3, 7, 3),
  [TET_I] = PRAM_RGB15(2, 8, 10),
  [TET_J] = PRAM_RGB15(4, 4, 7),
  [TET_T] = PRAM_RGB15(7, 3, 6),
  [TET_EMPTY] = PRAM_RGB15(0, 0, 0),
  [TET_LAST] = PRAM_RGB15(2, 2, 2),
};

void
init_palettes(void)
{
  for (int i = 0; i <= TET_LAST; i++) {
    pram.bg[i][1] = colors[i];
    pram.bg[i][2] = colors_ghost[i];
    pram.bg[i][3] = colors_lock[i];
  }

  for (int i = 0; i <= TET_LAST; i++) {
    pram.obj[1][i + 1] = colors[i];
    pram.obj[2][i + 1] = colors_ghost[i];
    pram.obj[3][i + 1] = colors_lock[i];
  }
}

void
render_piece(void)
{
  int shift = OBJ_TET_SHIFT(frame.piece.tet, frame.piece.dir);
  int x = ((frame.piece.pos.u + 10) - shift) * 8;
  int y = ((frame.piece.pos.v - 20) - shift) * 8;
  int y_ghost = ((frame.piece.drop_row - 20) - shift) * 8;
  int c = OBJ_TET_CHAR(frame.piece.tet, frame.piece.dir);
  if (x < 0) x = 256 + x;
  if (y < 0) y = 256 + y;
  if (y_ghost < 0) y_ghost = 256 + y_ghost;

  /* oam piece */

  oam.obj[0].attr[0] =
    ( OBJ_A0__SHAPE_SQUARE
    | OBJ_A0__MODE_NORMAL
    | OBJ_A0__Y_COORDINATE(y)
    );

  oam.obj[0].attr[1] =
    ( OBJ_A1__SIZE(OBJ__SQUARE_32_32)
    | OBJ_A1__X_COORDINATE(x)
    );

  oam.obj[0].attr[2] =
    ( OBJ_A2__COLOR_PALETTE(frame.piece.lock_delay.locking ? 3 : 1)
    | OBJ_A2__PRIORITY(1)
    | OBJ_A2__CHARACTER(c)
    );

  /* oam ghost */

  oam.obj[1].attr[0] =
    ( OBJ_A0__SHAPE_SQUARE
    | OBJ_A0__MODE_NORMAL
    | OBJ_A0__Y_COORDINATE(y_ghost)
    );

  oam.obj[1].attr[1] =
    ( OBJ_A1__SIZE(OBJ__SQUARE_32_32)
    | OBJ_A1__X_COORDINATE(x)
    );

  oam.obj[1].attr[2] =
    ( OBJ_A2__COLOR_PALETTE(2)
    | OBJ_A2__PRIORITY(1)
    | OBJ_A2__CHARACTER(c)
    );
}

void render_static_backgrounds(void)
{
  /* static backgrounds */
  fill_32((void*)&vram.screen_block[31][0],
          half_32(SCREEN_TEXT__CHARACTER(0)),
          SCREEN_BASE_BLOCK_LENGTH);

  fill_32((void*)&vram.screen_block[30][0],
          half_32
            ( SCREEN_TEXT__COLOR_PALETTE(TET_Z)
            | SCREEN_TEXT__CHARACTER(1)
            ),
          SCREEN_BASE_BLOCK_LENGTH);

  // unused
  fill_32((void*)&vram.screen_block[29][0],
          half_32(SCREEN_TEXT__CHARACTER(0)),
          SCREEN_BASE_BLOCK_LENGTH);

  fill_32((void*)&vram.screen_block[28][0],
          half_32( SCREEN_TEXT__COLOR_PALETTE(TET_LAST)
                 | SCREEN_TEXT__CHARACTER(1)
                 ),
          SCREEN_BASE_BLOCK_LENGTH);

  for (int y = 0; y < 20; y++) {
    for (int x = 0; x < 10; x++) {
      vram.screen_block[28][(y * 32) + (x + 10)] =
        ( SCREEN_TEXT__COLOR_PALETTE(TET_EMPTY)
        | SCREEN_TEXT__CHARACTER(1)
        );
    }
  }

  for (int y = 0; y < 20; y++) {
    vram.screen_block[28][(y * 32) + (20)] =
      ( SCREEN_TEXT__COLOR_PALETTE(TET_LAST)
      | SCREEN_TEXT__CHARACTER(3)
      );
  }
}

void render_field(void)
{
  for (int y = 0; y < 20; y++) {
    for (int x = 0; x < 10; x++) {
      vram.screen_block[28][(y * 32) + (x + 10)] =
        ( SCREEN_TEXT__COLOR_PALETTE(frame.field[y + 20][x].color)
        | SCREEN_TEXT__CHARACTER(2)
        );
    }
  }
}

#define R_QUEUE_OFFSET (2) // OAM offset
#define R_SWAP_OFFSET (R_QUEUE_OFFSET + BAG_QUEUE_LEN)
#define R_QUEUE_X (8 * 10 * 2 + 8 * 2)

void render_queue(void)
{
  int queue_ix = frame.queue.index;

  for (int i = 0; i < BAG_QUEUE_LEN; i++) {

    int c = OBJ_TET_CHAR(frame.queue.tet[queue_ix], 0);
    if ((++queue_ix) >= BAG_QUEUE_LEN) queue_ix = 0;

    oam.obj[R_QUEUE_OFFSET + i].attr[2] =
      ( OBJ_A2__COLOR_PALETTE(1)
      | OBJ_A2__PRIORITY(1)
      | OBJ_A2__CHARACTER(c)
      );
  }
}

void render_swap(void)
{
  if (frame.hold.piece != TET_EMPTY) {
    int c = OBJ_TET_CHAR(frame.hold.piece, 0);

    oam.obj[R_SWAP_OFFSET].attr[0] =
      ( OBJ_A0__SHAPE_SQUARE
      | OBJ_A0__MODE_NORMAL
      | OBJ_A0__Y_COORDINATE(0)
      );

    oam.obj[R_SWAP_OFFSET].attr[1] =
      ( OBJ_A1__SIZE(OBJ__SQUARE_32_32)
      | OBJ_A1__X_COORDINATE(8 * 4)
      );

    oam.obj[R_SWAP_OFFSET].attr[2] =
      ( OBJ_A2__COLOR_PALETTE(1)
      | OBJ_A2__PRIORITY(1)
      | OBJ_A2__CHARACTER(c)
      );
  } else {
    oam.obj[R_SWAP_OFFSET].attr[0] = OBJ_A0__DOUBLE_SIZE;
  }
}

void render_init_queue(void)
{
  /*
    The on-screen positions of the queue items do not change, only the tetronimo
    types (OAM characters).

    This is separated from render_queue() to allow OBJ on-screen positions to be
    set ahead of time, as they do not change for the duration of the game.
   */

  for (int i = 0; i < BAG_QUEUE_LEN; i++) {
    oam.obj[R_QUEUE_OFFSET + i].attr[0] =
      ( OBJ_A0__SHAPE_SQUARE
      | OBJ_A0__MODE_NORMAL
      | OBJ_A0__Y_COORDINATE(i * 24)
      );

    oam.obj[R_QUEUE_OFFSET + i].attr[1] =
      ( OBJ_A1__SIZE(OBJ__SQUARE_32_32)
      | OBJ_A1__X_COORDINATE(R_QUEUE_X)
      );
  }
}
