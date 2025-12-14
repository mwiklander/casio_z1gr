EXE = z1f9
CC = gcc
OBJS = z1.o init.o conf.o i80x86.o i80x86asm.o memory.o io.o bios.o depend.o

# SDL 2.0
CFLAGS = -DI86_USE_SDL -O3 -Wall $(shell sdl2-config --cflags)
LDFLAGS = -s $(shell sdl2-config --libs)

# SDL 1.2
#CFLAGS = -DI86_USE_SDL -O3 -Wall $(shell sdl-config --cflags)
#LDFLAGS = -s $(shell sdl-config --libs)

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
win32exe: $(OBJS) resource.o
	$(CC) -o $(EXE) $(OBJS) resource.o $(LDFLAGS)
.c.o:
	$(CC) -c $(CFLAGS) $<
i80x86.o: i80x86.c i80x86.h
i80x86asm.o: i80x86asm.c i80x86.h
z1.o: z1.c z1.h pseudorom.h
resource.o: resource.rc z1icon.ico
	windres $< -o $@
install:
	cp -p z1f9 /usr/local/bin
src:
	rm -r -f z1f9
	mkdir z1f9
	cp -p Makefile *.c *.h *.xbm *.ico z1f9/
	nkf -w80 -Lu < readme.txt > z1f9/readme.txt
	nkf -w80 -Lu < z1f9config.linux > z1f9/z1f9config
	tar -c z1f9/ | gzip -9 > z1f9.tgz
	rm -r -f z1f9
win32:
	rm -r -f z1f9win32
	mkdir z1f9_win32
	mkdir z1f9_win32\io
	mkdir z1f9_win32\disk
	cp -p $(EXE).exe README-SDL2.txt SDL2.dll z1f9_win32/
	nkf -s -Lw < readme.txt > z1f9_win32/readme.txt
	nkf -s -Lw < z1f9config.win32 > z1f9_win32/z1f9config
	zip -u -9 z1f9win32.zip z1f9_win32/*
	rm -r -f z1f9_win32
mac:
	rm -r -f z1f9_mac
	mkdir z1f9_mac
	cp -p z1f9 z1f9_mac/
	nkf -s -Lm < readme.txt > z1f9_mac/readme.txt
	nkf -s -Lm < z1f9config.mac > z1f9_mac/z1f9config
	zip -u -9 z1f9_mac.zip z1f9_mac/*
	rm -r -f z1f9_mac
clean:
	rm -f *.o
	rm -f $(EXE)
	rm -f $(EXE).exe
	rm -f *.zip
	rm -f *.tgz
	rm -f stdout.txt
	rm -f stderr.txt
