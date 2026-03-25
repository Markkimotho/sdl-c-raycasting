#!/bin/bash
cd /Users/ktinega/Documents/My_Projects/sdl-c-raycasting
rm -f src/main.o src/init.o src/textures.o game

CFLAGS="-Wall -Wextra -Wpedantic -std=c99 -Iheaders -O3 -DNDEBUG"
CFLAGS="$CFLAGS $(pkg-config --cflags sdl2 SDL2_image)"
LDFLAGS="$(pkg-config --libs sdl2 SDL2_image) -lm"

echo "Compiling..."
gcc $CFLAGS -c src/main.c -o src/main.o
gcc $CFLAGS -c src/init.c -o src/init.o
gcc $CFLAGS -c src/textures.c -o src/textures.o

echo "Linking..."
gcc src/main.o src/init.o src/textures.o -o game $LDFLAGS

if [ -f game ]; then
    echo "Build successful!"
    ls -lh game
else
    echo "Build failed"
    exit 1
fi
