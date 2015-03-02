#!/bin/sh
clang++ --std=c++11 -c -g -Iinclude -o build/ops.o src/ops.cpp
clang++ --std=c++11 -c -g -Iinclude -o build/cpu.o src/cpu.cpp
clang++ --std=c++11 -c -g -Iinclude -o build/main.o src/main.cpp

clang++ --std=c++11 -g -o bin/nesemul build/ops.o build/cpu.o build/main.o