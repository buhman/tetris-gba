#include "type.h"

#include "tetris.h"
#include "vram.h"

#include "block.h"
#include "obj_tet.h"

/*
y,x
0,0 (shift 4, y-offset 0)
0,1 (shift 0, y-offset 0)

1,0 (shift 4, y-offset 4)
1,1 (shift 4, y-offset 4)

0 1110 0000
  1110 0000
1 1110 0000
  0000 0000

2 0000 0000
  0000 0000
3 0000 0000
  0000 0000


0 0000 1110
  0000 1110
1 0000 1110
  0000 0000

2 0000 0000
  0000 0000
3 0000 0000
  0000 0000
*/

/*
static inline u32
pixel_data(u8 shift, u8 value)
{
  return
    ( ( value << shift )
    | ( value << (shift + 4) )
    | ( value << (shift + 8) )
    );
}

static inline void
block_4x4_2x2(void * buf, int u, int v, u8 value)
{
  int u2 = u & 1;
  int v2 = v & 1;
  u = u >> 1;
  v = v >> 1;

  u32 * mem = (u32*)&((u8*)buf)[((v * 2) + u) * (8 * 8 / 2)];
  u32 data = pixel_data((u2 * 16), value);
  mem[0 + (4*v2)] |= data;
  mem[1 + (4*v2)] |= data;
  mem[2 + (4*v2)] |= data;
}

void
obj_tet_mini_init(void)
{
  for (int tet = 0; tet < TET_EMPTY; tet++) {
    int c = OBJ_TET_MINI_CHAR(tet);
    const coord * offset = offsets[tet][0];
    //int shift = OBJ_TET_SHIFT(tet, dir);
    int shift = 1;
    for (int i = 0; i < 4; i++) {
      block_4x4_2x2((void*)&vram.obj[tile(c)],
                    shift + offset[i].u, shift + offset[i].v, 1+tet);

    }
    break;
  }
}
*/

void
obj_tet_init(void)
{
  for (int tet = 0; tet < TET_EMPTY; tet++) {
    for (int dir = 0; dir < DIR_LAST; dir++) {
      int c = OBJ_TET_CHAR(tet, dir);

      for (int i = 0; i < 4; i++) {
        const coord * offset = offsets[tet][dir];

        int shift = OBJ_TET_SHIFT(tet, dir);
        int u = shift + offset[i].u;
        int v = shift + offset[i].v;

        block_tile((void*)&vram.obj[tile(c + (v * 4) + u)], tet + 1);
      }
    }
  }
}
