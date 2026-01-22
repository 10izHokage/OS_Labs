@echo off

git pull

if not exist build mkdir build

cd build

cmake ..

cmake --build .

echo --- DONE ---
pause
