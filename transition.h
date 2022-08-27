#pragma once

typedef enum state {
  STATE_TITLE = 0,
  STATE_RESET,
  STATE_RUNNING,
  STATE_PAUSED,
  STATE_TOPPED_OUT,
  STATE_LAST,
} state;

void transition(enum state state);
