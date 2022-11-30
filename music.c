#include "type.h"
#include "io_reg.h"
#include "register_values.h"

#include "music.h"
#include "res/music.score.h"

static u32 score = (u32)&_binary_res_puzzle_score_start;
static u16 notes_per_measure;
static u16 steps_per_measure;
static u32 measure_count;
static void * _measures;
static u32 sequence_length;
static void * _voices;

static inline struct note * get_measure(u32 m_ref)
{
  return (struct note *)(((u32)_measures) + (m_ref * notes_per_measure * (sizeof (struct note))));
}

static inline s32 * get_voice(u32 v_ref)
{
  return (s32 *)(((u32)_voices) + (v_ref * sequence_length * (sizeof (struct voice))));
}

struct voice_state {
  u8 step;
  u8 note;
};

struct sequence_state {
  u16 measure;
  u16 step;
  struct voice_state voice[4];
};

static struct sequence_state state = {0};

void music_step(void)
{
  for (int i = 0; i < 3; i++) {
    s32 * m = get_voice(i);
    s32 m_ref = m[state.measure];

    u16 frequency;
    u16 duration;
    if (m_ref == -1) {
      frequency = 0;
      duration = steps_per_measure;
    } else {
      struct note * n = get_measure(m_ref);
      frequency = n[state.voice[i].note].frequency;
      duration = n[state.voice[i].note].duration;
    }

    if (state.voice[i].step == 0) {
      switch (i) {
      case 0:
        io_reg.SOUND1_CNT_H =
          ( SOUND1_CNT_H__ENVELOPE_VALUE(frequency == 0 ? 0 : 9)
          | SOUND1_CNT_H__ENVELOPE_STEPS(3)
          | SOUND1_CNT_H__DUTY_CYCLE(2)
          );

        io_reg.SOUND1_CNT_X =
          ( SOUND1_CNT_X__FREQUENCY_DATA(frequency)
          | SOUND1_CNT_X__RESTART
          );
        break;
      case 1:
        io_reg.SOUND2_CNT_L =
          ( SOUND2_CNT_L__ENVELOPE_VALUE(frequency == 0 ? 0 : 9)
          | SOUND2_CNT_L__ENVELOPE_STEPS(3)
          | SOUND2_CNT_L__DUTY_CYCLE(3)
          );
        io_reg.SOUND2_CNT_H =
          ( SOUND2_CNT_H__FREQUENCY_DATA(frequency)
          | SOUND2_CNT_H__RESTART
          );
        break;
      case 2:
        io_reg.SOUND3_CNT_H =
          ( SOUND3_CNT_H__OUTPUT_LEVEL(frequency == 0 ? 0 : 1)
          | SOUND3_CNT_H__SOUND_LENGTH(3)
          );
        io_reg.SOUND3_CNT_X =
          ( SOUND3_CNT_X__FREQUNCY_DATA(frequency)
          | SOUND3_CNT_X__RESTART
          );
        break;
      case 3:
        io_reg.SOUND4_CNT_L =
          ( SOUND4_CNT_L__ENVELOPE_VALUE(frequency == 0 ? 0 : 10)
          | SOUND4_CNT_L__ENVELOPE_STEPS(1)
          | SOUND4_CNT_L__SOUND_LENGTH(30)
          );

        io_reg.SOUND4_CNT_H =
          ( SOUND4_CNT_H__RESTART
          | SOUND4_CNT_H__COUNTER_SHIFT_FREQ(1)
          | SOUND4_CNT_H__COUNTER_PRESCALAR(4)
          );
        break;
      }
    }
    u32 step = ++(state.voice[i].step);
    if (step == duration) {
      state.voice[i].step = 0;
      state.voice[i].note++;
    }
  }

  state.step++;
  if (state.step == steps_per_measure) {
    state.step = 0;
    state.measure++;
    if (state.measure == sequence_length)
      state.measure = 0;

    for (int i = 0; i < 3; i++) {
      state.voice[i].step = 0;
      state.voice[i].note = 0;
    }
  }
}

static u32 ticks = 0;

void music_tick(void)
{
  ticks += 1;

  if (ticks >= 4) {
    ticks = 0;
    music_step();
  }
}

void music_score_init(void)
{
  struct library * l = (struct library *)score;
  notes_per_measure = l->notes_per_measure;
  steps_per_measure = l->steps_per_measure;
  measure_count = l->measure_count;
  _measures = (void*)(score + (4 * 2));
  struct sequence * s = (void *)((u8 *)_measures) + (measure_count *
                                                     notes_per_measure *
                                                     (sizeof (struct note)));
  sequence_length = s->sequence_length;
  _voices = (void*)((u32)s + 4);

  state.measure = 0;
}

void music_init(void)
{
  music_score_init();

  io_reg.SOUNDCNT_X =
    ( SOUNDCNT_X__ENABLE
    | SOUNDCNT_X__ENABLE_1
    | SOUNDCNT_X__ENABLE_2
    | SOUNDCNT_X__ENABLE_3
    | SOUNDCNT_X__ENABLE_4
    );

  io_reg.SOUNDCNT_L =
    ( SOUNDCNT_L__OUTPUT_1_L
    | SOUNDCNT_L__OUTPUT_1_R
    | SOUNDCNT_L__OUTPUT_2_L
    | SOUNDCNT_L__OUTPUT_2_R
    | SOUNDCNT_L__OUTPUT_3_L
    | SOUNDCNT_L__OUTPUT_3_R
    | SOUNDCNT_L__OUTPUT_4_L
    | SOUNDCNT_L__OUTPUT_4_R
    | SOUNDCNT_L__OUTPUT_LEVEL_L(5)
    | SOUNDCNT_L__OUTPUT_LEVEL_R(5)
    );

  io_reg.SOUNDCNT_H =
    ( SOUNDCNT_H__OUTPUT_1234_RATIO_FULL
    );

  io_reg.SOUND1_CNT_L =
    ( SOUND1_CNT_L__SWEEP_TIME(0)
    | SOUND1_CNT_L__SWEEP_DECREASE
    | SOUND1_CNT_L__SWEEP_SHIFTS(0)
    );

  *((volatile u32 *)(&io_reg.WAVE_RAM0_L)) = 0x77665544;
  *((volatile u32 *)(&io_reg.WAVE_RAM1_L)) = 0x33221100;
  *((volatile u32 *)(&io_reg.WAVE_RAM2_L)) = 0x77665544;
  *((volatile u32 *)(&io_reg.WAVE_RAM3_L)) = 0x33221100;
  //*((volatile u32 *)(&io_reg.WAVE_RAM0_L)) = 0xfedcba98;
  //*((volatile u32 *)(&io_reg.WAVE_RAM1_L)) = 0x76543210;
  //*((volatile u32 *)(&io_reg.WAVE_RAM2_L)) = 0xfedcba98;
  //*((volatile u32 *)(&io_reg.WAVE_RAM3_L)) = 0x76543210;

  io_reg.SOUND3_CNT_L =
    ( SOUND3_CNT_L__SOUND_OUTPUT
    | SOUND3_CNT_L__WAVE_BANK(1)
    | SOUND3_CNT_L__WAVE_32_STEP
    );

  // not setting parameters on 4 prior to restart appears to make the first
  // restart sound "weird"
  io_reg.SOUND4_CNT_H =
    ( SOUND4_CNT_H__COUNTER_SHIFT_FREQ(1)
    | SOUND4_CNT_H__COUNTER_PRESCALAR(4)
    );
}
