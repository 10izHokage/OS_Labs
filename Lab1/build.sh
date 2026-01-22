#!/bin/bash

cd "$(dirname "$0")"

git pull

makdir -p build

cd build

cmake ..

make

echo "--- DONE ---"
