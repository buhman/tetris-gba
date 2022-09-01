import sys

from lxml import etree

from score.types import *
from score import parse
from score import transform
from score import encode


with open(sys.argv[1], 'rb') as f:
    s = f.read()


score: list[Measure] = parse.parse_score(etree.fromstring(s))

voice_measure_list: list[list[VoiceMeasure]] = list(map(transform.measure_collate, score))

ID = int
Forward = list[tuple[Note]] # dict[ID, tuple[notes]]
Reverse = dict[tuple[Note], ID]

forward, reverse = transform.measure_library(voice_measure_list)

print(sum(len(vms) for vms in forward))
print(sum([len(measure.notes) for measure in score]))

sequence = transform.sequence(reverse, voice_measure_list)

buf = bytearray(64 * 1024)
mem = memoryview(buf)

flat_sequence = transform.flatten_sequence(sequence)

notes_per_measure = transform.max_notes_per_measure(forward)
library_size = encode.encode_library(
    mem,
    notes_per_measure,
    parse.EXPECTED_TOTAL_DURATION,
    forward
)
mem = mem[library_size:]
sequence_size = encode.encode_sequence(mem, flat_sequence)

print(library_size, sequence_size)

with open(sys.argv[2], 'wb') as f:
    f.write(memoryview(buf)[:library_size + sequence_size])
