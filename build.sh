#!/bin/sh
mkdir -p ./build
cd src
g++ ./main.cpp ./misc.cpp -o ../build/miscedit-x64

# Optional
#aarch64-linux-gnu-g++ ./main.cpp ./misc.cpp -o ../build/miscedit-arm64