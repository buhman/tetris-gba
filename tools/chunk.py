import sys
import struct
from os import path

from collections import defaultdict


def _chunk(chunks: dict,
           mem: memoryview,
           width: int, height: int,
           cx: int, cy: int
           ):
    bufm = bytearray(8 * 8)

    for y in range(8):
        for x in range(8):
            y_offset = ((cy * 8) + y) * width
            x_offset = (cx * 8) + x
            offset = y_offset + x_offset
            bufm[y * 8 + x] = mem[offset]

    b = bytes(bufm)
    return chunks[b]


def chunk(filename):
    with open(filename, "rb") as f:
        buf = f.read()

    with open(path.splitext(path.splitext(filename)[0])[0] + ".size", 'r') as f:
        width, height = [int(i) for i in f.read().strip().split()]

    assert width % 8 == 0 and height % 8 == 0

    _index = 0
    def next_index():
        nonlocal _index
        i = _index
        _index += 1
        return i

    chunks = defaultdict(next_index)

    def do():
        for cy in range(height // 8):
            for cx in range(width // 8):
                yield _chunk(chunks, memoryview(buf), width, height, cx, cy)

    chunk_indexes = list(do())

    reverse_chunks = sorted(chunks.items(), key=lambda k_v: k_v[1])

    with open(path.splitext(filename)[0] + ".chunks", "wb") as f:
        for chunk, _ in reverse_chunks:
            f.write(chunk)

    with open(path.splitext(filename)[0] + ".indexes", "wb") as f:
        for index in chunk_indexes:
            f.write(struct.pack("<H", index))


    print(sys.argv[1], len(buf), len(chunk_indexes), 8*8*len(chunks))


chunk(sys.argv[1])
