#!/bin/bash

[ -d build ] || mkdir build

cd build

#create Makefile
cmake -DCMAKE_BUILD_TYPE=Debug ..

#Run make
cmake --build . -j4
