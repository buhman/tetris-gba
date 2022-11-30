import sys

from lxml import etree

from score.types import *

str_step = {
    "A": Step.A,
    "B": Step.B,
    "C": Step.C,
    "D": Step.D,
    "E": Step.E,
    "F": Step.F,
    "G": Step.G,
}

int_alter = {
    -1: Alter.flat,
    0: Alter.natural,
    1: Alter.sharp,
}



def parse_pitch(pitch: etree.Element) -> Pitch:
    _step, = pitch.xpath('./step/text()')
    step = str_step[_step]

    # alter: -1=flat 0=natural 1=sharp
    _alter = pitch.xpath('./alter/text()')
    _alter = next(iter(_alter), 0)
    alter = int_alter[int(_alter)]

    _octave, = pitch.xpath('./octave/text()')
    octave = int(_octave)

    return Pitch(step, alter, octave)


_valid_durations = {1 << i for i in range(8)}

def parse_note(note: etree.Element) -> Note:
    if note.xpath('./rest'):
        pitch = Rest()
    else:
        pitch = parse_pitch(note.xpath('./pitch')[0])
        if pitch == Pitch(step=Step.A, alter=Alter.natural, octave=1):
            raise Exception(pitch)

    duration = int(note.xpath('./duration/text()')[0])
    #assert duration in _valid_durations, duration

    voice = int(note.xpath('./voice/text()')[0])

    return Note(pitch, int(duration), voice)


# HACK
#EXPECTED_TOTAL_DURATION = 32

def parse_measure(measure: etree.Element) -> list[Note]:
    last_voice = None
    total_duration = 0

    for child in measure:
        if child.tag == 'note':
            try:
                note = parse_note(child)
            except:
                print("measure", measure.get("number"))
                print(etree.tostring(measure).decode('utf-8'))
                raise
            assert last_voice is None or note.voice == last_voice
            if last_voice is None and total_duration > 0:
                yield Note(Rest(), total_duration, note.voice)
            last_voice = note.voice

            total_duration += note.duration
            yield note
        elif child.tag == 'backup':
            backup_duration = int(child.xpath('./duration/text()')[0])
            #assert total_duration == EXPECTED_TOTAL_DURATION
            total_duration -= backup_duration
            assert total_duration >= 0
            last_voice = None
        elif child.tag == 'forward':
            forward_duration = int(child.xpath('./duration/text()')[0])
            assert last_voice == None
            assert total_duration == 0
            total_duration += forward_duration
        else:
            #raise Exception(child.tag)
            #print("ignored", child.tag, file=sys.stderr)
            pass

    # FIXME: hack
    #if total_duration != EXPECTED_TOTAL_DURATION:
    #    print(etree.tostring(measure).decode('utf-8'))
    #assert total_duration == EXPECTED_TOTAL_DURATION, (total_duration, measure.get('number'))


def parse_part(part: etree.Element) -> list[Measure]:
    for measure in part.xpath('./measure'):
        notes = tuple(parse_measure(measure))
        measure = Measure(notes)
        yield measure


def parse_score(root: etree.Element) -> list[str, list[Measure]]:
    for part in root.xpath('//part'):
        yield part.get("id"), list(parse_part(part))
