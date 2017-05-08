#ARCH=-m32
DEBUG=#-DDEBUG
#CFLAGS=-g -O2 -I. -Wall -std=c11 -pedantic $(ARCH) $(DEBUG)
CFLAGS=-g -O2 -I. -Wall -std=c99 -pedantic $(ARCH) $(DEBUG)
LDFLAGS=g -lSDL -lm $(ARCH) -pthread

sdlvnc: d3des.o SDL_vnc.o support.o sdlvnc.o
	gcc -g -o sdlvnc sdlvnc.o SDL_vnc.o d3des.o support.o -I . -lSDL -lSDL_gfx -lm $(ARCH) -pthread

test: d3des.o SDL_vnc.o support.o
	gcc -g -o test SDL_vnc.o d3des.o support.o -I . -lSDL -lm  Test/TestVNC.c $(ARCH)

sdlvnc.o: client/sdlvnc.c
	gcc $(CFLAGS) -c -o sdlvnc.o client/sdlvnc.c

d3des.o: d3des.c

support.o: support.c

SDL_vnc.o: SDL_vnc.c
