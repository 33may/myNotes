#!/usr/bin/bash
export CMAKE_ROOT=/usr/share/cmake
export PATH="/usr/bin:/usr/local/bin:/usr/sbin:/sbin:$PATH"
rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja -C build
