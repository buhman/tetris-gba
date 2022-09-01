from typing import Union
from dataclasses import dataclass
from enum import Enum, auto


__all__ = [
    'Pitch',
    'Alter',
    'Note',
    'Measure',
    'VoiceMeasure',
    'Step',
    'Rest'
]

class Step(Enum):
    A = auto()
    B = auto()
    C = auto()
    D = auto()
    E = auto()
    F = auto()
    G = auto()

locals().update(Step.__members__)

Octave = int

class Alter(Enum):
    sharp = auto()
    flat = auto()
    natural = auto()

locals().update(Alter.__members__)

Duration = int

Voice = int


@dataclass(frozen=True)
class Pitch:
    step: Step
    alter: Alter
    octave: Octave

Rest = type('Rest', (), {})

@dataclass(frozen=True)
class Note:
    pitch: Union[Pitch, Rest]
    duration: Duration
    voice: Voice

@dataclass(frozen=True)
class Measure:
    notes: tuple[Note]

@dataclass(frozen=True)
class VoiceMeasure:
    voice: Voice
    notes: tuple[Note]

P = Pitch
Frequency = float

frequencies: dict[Pitch, Frequency] = {
    P(C, natural, 2): 65.41,  # ~lowest GBA frequency
    P(C, sharp,   2): 69.30,
    P(D, flat,    2): 69.30,
    P(D, natural, 2): 73.42,
    P(D, sharp,   2): 77.78,
    P(E, flat,    2): 77.78,
    P(E, natural, 2): 82.41,
    P(F, natural, 2): 87.31,
    P(F, sharp,   2): 92.50,
    P(G, flat,    2): 92.50,
    P(G, natural, 2): 98.00,
    P(G, sharp,   2): 103.83,
    P(A, flat,    2): 103.83,
    P(A, natural, 2): 110.00,
    P(A, sharp,   2): 116.54,
    P(B, flat,    2): 116.54,
    P(B, natural, 2): 123.47,
    P(C, natural, 3): 130.81,
    P(C, sharp,   3): 138.59,
    P(D, flat,    3): 138.59,
    P(D, natural, 3): 146.83,
    P(D, sharp,   3): 155.56,
    P(E, flat,    3): 155.56,
    P(E, natural, 3): 164.81,
    P(F, natural, 3): 174.61,
    P(F, sharp,   3): 185.00,
    P(G, flat,    3): 185.00,
    P(G, natural, 3): 196.00,
    P(G, sharp,   3): 207.65,
    P(A, flat,    3): 207.65,
    P(A, natural, 3): 220.00,
    P(A, sharp,   3): 233.08,
    P(B, flat,    3): 233.08,
    P(B, natural, 3): 246.94,
    P(C, natural, 4): 261.63,
    P(C, sharp,   4): 277.18,
    P(D, flat,    4): 277.18,
    P(D, natural, 4): 293.66,
    P(D, sharp,   4): 311.13,
    P(E, flat,    4): 311.13,
    P(E, natural, 4): 329.63,
    P(F, natural, 4): 349.23,
    P(F, sharp,   4): 369.99,
    P(G, flat,    4): 369.99,
    P(G, natural, 4): 392.00,
    P(G, sharp,   4): 415.30,
    P(A, flat,    4): 415.30,
    P(A, natural, 4): 440.00,
    P(A, sharp,   4): 466.16,
    P(B, flat,    4): 466.16,
    P(B, natural, 4): 493.88,
    P(C, natural, 5): 523.25,
    P(C, sharp,   5): 554.37,
    P(D, flat,    5): 554.37,
    P(D, natural, 5): 587.33,
    P(D, sharp,   5): 622.25,
    P(E, flat,    5): 622.25,
    P(E, natural, 5): 659.25,
    P(F, natural, 5): 698.46,
    P(F, sharp,   5): 739.99,
    P(G, flat,    5): 739.99,
    P(G, natural, 5): 783.99,
    P(G, sharp,   5): 830.61,
    P(A, flat,    5): 830.61,
    P(A, natural, 5): 880.00,
    P(A, sharp,   5): 932.33,
    P(B, flat,    5): 932.33,
    P(B, natural, 5): 987.77,
    P(C, natural, 6): 1046.50,
    P(C, sharp,   6): 1108.73,
    P(D, flat,    6): 1108.73,
    P(D, natural, 6): 1174.66,
    P(D, sharp,   6): 1244.51,
    P(E, flat,    6): 1244.51,
    P(E, natural, 6): 1318.51,
    P(F, natural, 6): 1396.91,
    P(F, sharp,   6): 1479.98,
    P(G, flat,    6): 1479.98,
    P(G, natural, 6): 1567.98,
    P(G, sharp,   6): 1661.22,
    P(A, flat,    6): 1661.22,
    P(A, natural, 6): 1760.00,
    P(A, sharp,   6): 1864.66,
    P(B, flat,    6): 1864.66,
    P(B, natural, 6): 1975.53,
    P(C, natural, 7): 2093.00,
    P(C, sharp,   7): 2217.46,
    P(D, flat,    7): 2217.46,
    P(D, natural, 7): 2349.32,
    P(D, sharp,   7): 2489.02,
    P(E, flat,    7): 2489.02,
    P(E, natural, 7): 2637.02,
    P(F, natural, 7): 2793.83,
    P(F, sharp,   7): 2959.96,
    P(G, flat,    7): 2959.96,
    P(G, natural, 7): 3135.96,
    P(G, sharp,   7): 3322.44,
    P(A, flat,    7): 3322.44,
    P(A, natural, 7): 3520.00,
    P(A, sharp,   7): 3729.31,
    P(B, flat,    7): 3729.31,
    P(B, natural, 7): 3951.07,
    P(C, natural, 8): 4186.01,
    P(C, sharp,   8): 4434.92,
    P(D, flat,    8): 4434.92,
    P(D, natural, 8): 4698.63,
    P(D, sharp,   8): 4978.03,
    P(E, flat,    8): 4978.03,
    P(E, natural, 8): 5274.04,
    P(F, natural, 8): 5587.65,
    P(F, sharp,   8): 5919.91,
    P(G, flat,    8): 5919.91,
    P(G, natural, 8): 6271.93,
    P(G, sharp,   8): 6644.88,
    P(A, flat,    8): 6644.88,
    P(A, natural, 8): 7040.00,
    P(A, sharp,   8): 7458.62,
    P(B, flat,    8): 7458.62,
    P(B, natural, 8): 7902.13,
}
