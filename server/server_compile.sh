#!/bin/bash
gcc -Wall -o server src/main.c src/networking.c src/game_logic.c
./server
