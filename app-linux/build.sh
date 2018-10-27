#!/bin/bash
cd .wio/targets/main
echo "set(CMAKE_EXE_LINKER_FLAGS \"${CMAKE_EXE_LINKER_FLAGS} -pthread\")" >> CMakeLists.txt
cd bin
make -j4

