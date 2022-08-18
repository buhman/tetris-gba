#define OBJ_TET (1)

#define OBJ_TET_SHIFT(tet, dir) \
  (((tet) == TET_I && ((dir) == 1 || (dir) == 2)) ? 2 : 1)

#define OBJ_TET_CHAR(tet, dir) \
  (OBJ_TET + ((DIR_LAST * (tet) + (dir)) * (4 * 4)))

#define OBJ_TET_MINI_CHAR(tet) \
  (OBJ_TET_CHAR(TET_LAST, 0) + ((tet) * (2 * 2)))

void
obj_tet_init(void);

void
obj_tet_mini_init(void);
