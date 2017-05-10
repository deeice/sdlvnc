DEBUG=#-DDEBUG
#CFLAGS=-g -O2 -I. -Wall -std=c11 -pedantic $(ARCH) $(DEBUG)
CFLAGS+=-g -O2 -I. $(DEBUG)
LDFLAGS+=-lSDL -lSDL_gfx -lm -pthread

sdlvnc: d3des.o SDL_vnc.o support.o sdlvnc.o
	$(CC) -o sdlvnc sdlvnc.o SDL_vnc.o d3des.o support.o $(LDFLAGS)

test: d3des.o SDL_vnc.o support.o
	$(CC) -g -o test SDL_vnc.o d3des.o support.o -I . -lSDL -lm  Test/TestVNC.c

sdlvnc.o: client/sdlvnc.c
	$(CC) $(CFLAGS) -c -o sdlvnc.o client/sdlvnc.c

d3des.o: d3des.c

support.o: support.c

SDL_vnc.o: SDL_vnc.c
