from collections import defaultdict

from score.types import *


def measure_collate(measure: Measure) -> list[VoiceMeasure]:
    voices = defaultdict(list)

    for note in measure.notes:
        voices[note.voice].append(note)

    def do():
        for voice, notes in voices.items():
            yield VoiceMeasure(voice, tuple(notes))

    return list(do())


def measure_library(voice_measure_list: list[list[VoiceMeasure]]) -> tuple[list[VoiceMeasure, dict[VoiceMeasure, int]]]:
    library = list(set(vm.notes for vms in voice_measure_list for vm in vms))
    def do():
        for i, notes in enumerate(library):
            yield notes, i

    return library, dict(do())


def max_notes_per_measure(forward: list[VoiceMeasure]) -> int:
    return max(len(notes) for notes in forward)


#fixme: hack
_voices = {
    1: 0,
    5: 1,
    6: 2,
    2: 2
}

LibraryID = int
VoiceID = int

def sequence(reverse: dict[tuple[Note], LibraryID],
             voice_measure_list: list[list[VoiceMeasure]]
             ) -> list[tuple[VoiceID, LibraryID]]:

    def do(voice_measures: list[VoiceMeasure]
           ) -> tuple[VoiceID, LibraryID]:
        voices = set(_voices.keys())
        voice_ix = 0

        for voice_measure in voice_measures:
            voices.remove(voice_measure.voice)
            library_id = reverse[voice_measure.notes]
            voice_ix += 1
            yield voice_measure.voice, library_id

        assert not (voice_ix > 3)

        while voice_ix < 3:
            voice_ix += 1
            yield voices.pop(), -1

    return [
        list(sorted(
            do(voice_measures), key=lambda v_i: _voices[v_i[0]]
        ))
        for voice_measures in voice_measure_list
    ]


def flatten_sequence(sequence: list[tuple[VoiceID, LibraryID]]
                     ) -> list[list[LibraryID]]:
    return [
        [voices[ix] for voices in sequence]
        for ix in range(3)
    ]
