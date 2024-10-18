#!/bin/bash
gcc -Wall -o client src/main.c src/network.c src/ui.c -I../libs/raylib/src -L../libs/raylib/build/raylib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
if [ $? -eq 0 ]; then
    ./client
else
    echo "Compilation failed"
fi
