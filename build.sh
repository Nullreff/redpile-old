#!/bin/sh
mkdir -p build/
cd build/
cmake $1 ..
make

