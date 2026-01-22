# Лабораторная №1
## Структура:

**1.** CMakeLists.txt — CMake-скрипт сборки

**2.** src/main.cpp — исходный код программы

**3.** build.cmd — скрипт сборки для Windows

**4.** build.sh — скрипт сборки для Linux

*.gitignore — чтобы папка build создавалась локально на компьютере и не добавлялась в репозиторий*

**Запуск на Windows**

**1.** Клонировать репозиторий
```
git clone https://github.com/10izHokage/OS_Lab1.git
cd OS_Lab1
```
**2.** Запустить скрипт сборки
`scripts\build.cmd`

**3.** Запустить программу
`build\hello.exe`

**Запуск на Linux**

**1.** Клонировать репозиторий
```
git clone https://github.com/10izHokage/OS_Lab1.git
cd OS_Lab1
```
**2.** Сделать файл сборки исполняемым:
`chmod +x build.sh`

**3.** Запустить автоматическую сборку:
`./build.sh`

**4.** Выполнить программу:
`./build/hello`
