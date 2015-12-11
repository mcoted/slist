#!/bin/bash
mkdir -p build
cd build
if [ "$1" == "debug" ]
then
	cmake .. -DCMAKE_BUILD_TYPE:STRING=debug
else
	cmake ..
fi
make
