#!/bin/sh
mkdir -p build/
cd build/
cmake cmake -DCMAKE_BUILD_TYPE:STRING=Debug ../src/
make
cd -
bundle exec rspec
