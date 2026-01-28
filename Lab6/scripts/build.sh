#!/bin/bash

cd "$(dirname "$0")/.."

rm -rf build

mkdir build

cd build

cmake -DCMAKE_BUILD_TYPE=Release ..

make -j$(nproc)

echo
echo "===== BUILD DONE ====="
