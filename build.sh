#!/bin/sh
mkdir -p build/
cd build/
cmake cmake $1 ..
make

