"""

struct note {
  u16 frequency;
  u16 duration;
}

struct measure {
  note n[measure_length];
}

struct library {
  u16 notes_per_measure;
  u16 steps_per_measure;
  u32 measure_count;
  struct measure m[measure_count];
};

struct voice {
  s32 measure_ref[sequence_length];
};

struct sequence {
  u32 sequence_length;
  struct voice voice[3];
}

struct score {
  struct library l;
  struct sequence s;
};

"""

from typing import Union
import struct

from score.types import *
from score import types


def _as_n(freq: float) -> int:
    assert int(freq) != 0
    n = (2048 * (freq - 64)) / freq
    if (n < 1 and n > 0):
        n = 1 # forcibly round 0 up to 1
    assert (round(n) > 0)
    return round(n)


def encode_pitch_rest(pitch: Union[Pitch, Rest]) -> int:
    if type(pitch) == Pitch:
        return _as_n(types.frequencies[pitch])
    elif type(pitch) == Rest:
        return 0
    else:
        raise TypeError(pitch)


def encode_library(mem: memoryview,
                   notes_per_measure: int,
                   steps_per_measure: int,
                   forward: list[tuple[Note]]
                   ) -> int:
    NOTE_SIZE = 2 + 2
    MEASURE_SIZE = notes_per_measure * NOTE_SIZE
    LIBRARY_SIZE = 2 + 2 + 4 + (len(forward) * MEASURE_SIZE)

    def encode_note(mem: memoryview, note: Note) -> int:
        frequency = encode_pitch_rest(note.pitch)
        struct.pack_into("<HH", mem, 0, frequency, note.duration)
        return NOTE_SIZE

    def encode_measure(mem: memoryview, notes: tuple[Note]) -> int:
        offset = 0
        for note in notes:
            size = encode_note(mem, note)
            offset += size
            mem = mem[size:]
        assert offset <= MEASURE_SIZE
        return MEASURE_SIZE

    print("notes_per_measure", notes_per_measure)
    print("steps_per_measure", steps_per_measure)
    print("measure_count", len(forward))
    struct.pack_into("<HHL", mem, 0, notes_per_measure, steps_per_measure, len(forward))
    mem = mem[8:]
    for notes in forward:
        size = encode_measure(mem, notes)
        mem = mem[size:]

    return LIBRARY_SIZE


LibraryID = int


def encode_sequence(mem: memoryview,
                    flat_sequence: list[list[LibraryID]],
                    ) -> int:
    sequence_length = len(flat_sequence[0])
    assert {len(s) for s in flat_sequence} == {sequence_length}
    VOICE_SIZE = 4 * sequence_length

    struct.pack_into("<L", mem, 0, sequence_length)
    mem = mem[4:]
    for voice in flat_sequence:
        for ix, (_, library_id) in enumerate(voice):
            struct.pack_into("<l", mem, ix * 4, library_id)
        mem = mem[VOICE_SIZE:]

    return 4 + VOICE_SIZE * len(flat_sequence)
