all: main.gba

LIB = $(PWD)/gba-lib

include $(LIB)/common.mk

AFLAGS += -I$(LIB)
CFLAGS += -I$(LIB)

OBJS = $(LIB)/header.o $(LIB)/load.o $(LIB)/copy.o
OBJS += main.o
OBJS += tetris.o
OBJS += render.o
OBJS += obj_tet.o
OBJS += bag.o
OBJS += glyph.o
OBJS += osd.o
OBJS += transition.o
OBJS += music.o
OBJS += res/block.data.o
OBJS += res/block_end.data.o
OBJS += res/BmPlus_ToshibaSat_8x8.glyph.o
OBJS += res/Bm437_CompaqThin_8x8.glyph.o
OBJS += res/Bm437_CL_EagleIII_8x16.glyph.o
OBJS += res/music.score.o

HEADERS = $(wildcard *.h)

main.elf: $(OBJS) | $(LIB)/main.lds
	$(call LINK_ELF,$(LIB)/main.lds)

# external

make-tools:
	$(MAKE) -C tools

%.glyph: %.otb | make-tools
	./tools/otb-convert $< > $@

%.glyph.o: %.glyph
	$(BUILD_BINARY_O)

%.score: %.musicxml | make-tools
	PYTHONPATH=./tools/muse python ./tools/muse/main.py $< $@

%.score.o: %.score
	$(BUILD_BINARY_IMAGE_O)

%.data.o: %.data
	$(BUILD_BINARY_IMAGE_O)
