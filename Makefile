PROJECT_NAME = BOBA_BUDDIES
ROM = $(PROJECT_NAME).nes

CC65_ROOT = $(realpath tools/cc65)
CC = $(CC65_ROOT)/bin/cc65
AS = $(CC65_ROOT)/bin/ca65
LD = $(CC65_ROOT)/bin/ld65

C65FLAGS += \
	-t nes -Oirs \
	--register-space 16 \
	-I ext/pixler/lib \

ASMINC = \
	-I ext/pixler/lib \

SRC = \
	src/main.c \

ASM = \
	src/data.s \
	src/misc.s \
	audio/audio.s \
	ext/famitone5/famitone5.s \

OBJS = \
	$(SRC:.c=.o) \
	$(ASM:.s=.o) \

CHR = \
	chr/0.png \
	chr/Boby.png \
	chr/Baby_Boba.png \

MAPS = \
	map/splash.bin \
	map/Level1.bin \
	map/Level2.bin \
	map/Level3.bin \
	map/Level4.bin \
	map/Level5.bin \
	map/Level6.bin \
	map/Level7.bin \
	map/Level8.bin \
	map/Level9.bin \
	map/WinScreen.bin \

SONGS = \
	audio/GameJamMusic.txt \

default: $(ROM)
rom: $(ROM)

PX_LIB_PATH = ext/pixler/lib
PX_LIB = $(PX_LIB_PATH)/px.lib
$(PX_LIB):
	$(MAKE) CC65_ROOT=$(CC65_ROOT) -C $(PX_LIB_PATH)

run-mac: rom
	open -a Nestopia $(ROM)

run-linux: rom
	mesen $(ROM)
#	nestopia -w -l 1 -n -s 2 -t $(ROM)

run-win: rom
	wintools/Mesen.exe $(ROM)

BIN = $(ROM:.nes=.bin)
$(BIN): $(ROM)
	dd if=$< ibs=1 skip=16 > $@	

romviz.png: $(BIN)
	tools/chr2png "1D 16 1A 11" $(BIN) $@

$(ROM): ld65.cfg $(OBJS) $(PX_LIB)
	$(LD) -C ld65.cfg --dbgfile $(ROM:.nes=.dbg) $(OBJS) $(PX_LIB) nes.lib -m link.log -o $@

%.s: %.c
	$(CC) -g $(C65FLAGS) $< --add-source $(INCLUDE) -o $@

%.s %.o: %.c
	$(CC65_ROOT)/bin/cl65 -c -g $(C65FLAGS) $(INCLUDE) $< -o $@

%.o: %.s
	$(AS) -g $< $(ASMINC) -o $@

%.chr: %.png
	tools/png2chr $< $@

%.lz4: %.chr
	tools/lz4x -f9 $< $@

%.bin: %.tmx
	python ext/pixler/tools/tmx2bin.py $< $@

%.lz4: %.bin
	tools/lz4x -f9 $< $@

src/data.o: $(CHR:.png=.lz4) $(MAPS)

tiles: chr/0.chr
	tools/chr2png "2B 3B 1C 0C" chr/0.chr chr/0-pal0.png
	tools/chr2png "2B 3B 1C 0C" chr/0.chr chr/0-pal1.png
	tools/chr2png "2B 3B 1C 0C" chr/0.chr chr/0-pal2.png
	tools/chr2png "2B 3B 1C 0C" chr/0.chr chr/0-pal3.png

audio/sounds.s: audio/sounds.nsf
	tools/nsf2data5 $< -ca65 -ntsc

audio/%.s: audio/%.txt
	tools/text2vol5 -ca65 $<

audio/audio.o: $(SONGS:.txt=.s) audio/sounds.s

tools:
	echo foobar

clean:
	-rm $(ROM) $(BIN) $(OBJS) $(CHR:.png=.chr) $(CHR:.png=.lz4)
	-rm $(MAPS)
	-rm $(SONGS:.txt=.s)
	-rm $(ROM:.nes=.dbg) link.log
	-rm romviz.png
	$(MAKE) -C $(PX_LIB_PATH) clean
.phony: default rom tiles clean
