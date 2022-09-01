struct note {
  u16 frequency;
  u16 duration;
};

struct measure {
  struct note * n; // [measure_length]
};

struct library {
  u16 notes_per_measure;
  u16 steps_per_measure;
  u32 measure_count;
  struct measure * m; // [measure_count]
};

struct voice {
  s32 * measure_ref; // [sequence_length]
};

struct sequence {
  u32 sequence_length;
  struct voice * v; // [3]
};

struct score {
  struct library l;
  struct sequence s;
};

void music_step(void);
void music_tick(void);
void music_init(void);
