#!/bin/sh
mkdir -p build/
cd build/
cmake ../src/
make
cd -
bundle exec rspec
