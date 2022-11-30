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


def measure_collate_parts(score: list[tuple[str, list[Measure]]]) -> list[list[VoiceMeasure]]:
    part_lengths = set(len(measures) for _, measures in score)
    assert len(part_lengths) == 1, part_lengths
    part_length = next(iter(part_lengths))

    for measure_ix in range(part_length):
        yield [
            VoiceMeasure(
                voice=part_ix,
                notes=measures[measure_ix].notes
            )
            for part_ix, (part_id, measures) in enumerate(score)
        ]

def measure_library(voice_measure_list: list[list[VoiceMeasure]]) -> tuple[list[VoiceMeasure, dict[VoiceMeasure, int]]]:
    library = list(set(vm.notes for vms in voice_measure_list for vm in vms))
    def do():
        for i, notes in enumerate(library):
            yield notes, i

    return library, dict(do())


def max_notes_per_measure(forward: list[VoiceMeasure]) -> int:
    return max(len(notes) for notes in forward)


def steps_per_measure(forward: list[VoiceMeasure]) -> int:
    spm = set(sum(note.duration for note in notes) for notes in forward)
    assert len(spm) == 1, spm
    return next(iter(spm))


LibraryID = int
VoiceID = int

def sequence(reverse: dict[tuple[Note], LibraryID],
             voice_measure_list: list[list[VoiceMeasure]]
             ) -> list[tuple[VoiceID, LibraryID]]:

    def do(voice_measures: list[VoiceMeasure]
           ) -> tuple[VoiceID, LibraryID]:
        hw_voices = {0, 1, 2, 3}

        for voice_measure in voice_measures:
            library_id = reverse[voice_measure.notes]
            yield hw_voices.pop(), library_id

        while hw_voices:
            yield hw_voices.pop(), -1

    return [
        list(sorted(
            do(voice_measures), key=lambda v_i: v_i[0]
        ))
        for voice_measures in voice_measure_list
    ]


def flatten_sequence(sequence: list[tuple[VoiceID, LibraryID]]
                     ) -> list[list[LibraryID]]:
    return [
        [voices[ix] for voices in sequence]
        for ix in range(4)
    ]
