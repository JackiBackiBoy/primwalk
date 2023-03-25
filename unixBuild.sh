#!/bin/bash
mkdir -p build
cd build
cmake -S ../ -B . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
make
cd bin
./fzcoach
cd ../..
