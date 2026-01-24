#!/bin/bash

cd "$(dirname "$0")/.."

mkdir -p build

cd build || exit 1

cmake ..

cmake --build .

echo "Build finished"
