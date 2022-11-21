from dataclasses import dataclass
import struct
from os import path
import sys

palette = dict()
palette[(0,0,0,0)] = 0
palette_index = 0

@dataclass(frozen=True)
class ImageDesc:
    width: int
    height: int
    path: str

ID = ImageDesc

images = [
    ID(184, 96, "text.data"),
    ID(256, 256, "front.data"),
    ID(256, 256, "back.data"),
    ID(256, 256, "middle.data"),
]


def assign_palette_index(color: memoryview) -> int:
    global palette
    global palette_index

    r, g, b, a = color[:4]
    rgba = (r, g, b, a)
    if a == 0: rgba = (0, 0, 0, 0)
    if rgba == (255, 255, 255, 255): rgba = (248, 248, 248, 255)

    assert a == 255 or rgba == (0, 0, 0, 0), rgba
    if rgba in palette:
        return palette[rgba]
    else:
        palette_index += 1
        palette[rgba] = palette_index
        return palette_index


def palettize_image(dirname, image: ImageDesc):

    with open(path.join(dirname, image.path), "rb") as f:
        buf = f.read()
        expected = (image.width * image.height * 4)
        assert len(buf) == expected, (image.path, len(buf), expected)

    mem = memoryview(buf)

    out = bytearray(image.width * image.height)

    for pixel_ix in range(len(mem) // 4):
        try:
            color: int = assign_palette_index(mem[(pixel_ix * 4):])
        except:
            print(pixel_ix, image.path)
            raise
        out[pixel_ix] = color

    with open(path.join(dirname, image.path + ".indexed"), "wb") as f:
        f.write(out)

        
dirname = path.split(sys.argv[1])[0]        
for image in images:
    palettize_image(dirname, image)


reverse_palette = sorted(palette.items(), key=lambda k_v: k_v[1])

def as_rgb15(color):
    color = color[:3]
    assert all(c % 8 == 0 for c in color), color
    return "PRAM_RGB15({:0},{:1},{:2}),".format(
        *(str(c >> 3).rjust(2, " ") for c in color)
    )

print("static u16 palette[] = {")
for k, v in reverse_palette:
    print(f"  [{v}] = " + as_rgb15(k))
print("};")
