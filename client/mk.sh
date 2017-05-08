#gcc -o sdlvnc sdlvnc.c -lm -lSDL -lSDL_vnc -lSDL_image -lSDL_ttf 
#gcc -g -o sdlvnc sdlvnc.c -lm -lSDL -lSDL_image -lSDL_ttf ../.libs/libSDL_vnc.a -pthread
#gcc -g -Os -o sdlvnc sdlvnc.c -lm -lSDL -lSDL_gfx ../.libs/libSDL_vnc.a -pthread
gcc -g -O2 -o sdlvnc sdlvnc.c -lm -lSDL -lSDL_gfx ../.libs/libSDL_vnc.a -pthread

strip sdlvnc
sstrip sdlvnc
