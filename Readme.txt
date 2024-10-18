Steps to run:
- install raylib inside libs/ folder with the necessary dependencies
- runt the client_compile.sh and server_compile.sh scripts from /client
and /server to compile the client and server

For raylib installation:
- sudo apt update
- sudo apt install -y build-essential git cmake
- sudo apt install -y libasound2-dev libgl1-mesa-dev libglu1-mesa-dev libopenal-dev
- git clone https://github.com/raysan5/raylib.git
- cd raylib
- mkdir build && cd build
- cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DRAYLIB_BUILD_EXAMPLES=OFF -DRAYLIB_BUILD_TESTS=OFF -DRAYLIB_BUILD_GAMES=OFF
- make -j$(nproc)

- sudo make install (optional) to install the library system-wide (default: /usr/local/)

Compile and test a program: 
- gcc test_raylib.c -o test_raylib -lraylib -lm -lpthread -ldl -lrt -lX11
- ./test_raylib

P.S. you can delete the big file from raylib/.git/objects/pack/ to save space, it's almost 400mb for some reason

All the necessary packages:
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libasound2-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libopenal-dev \
    libx11-dev \
    libxcursor-dev \
    libxi-dev \
    libxinerama-dev \
    libxrandr-dev \
    libx11-xcb-dev \
    libwayland-dev \
    wayland-protocols \
    pkg-config \
    libxkbcommon-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev