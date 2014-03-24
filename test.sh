#!/bin/sh
mkdir -p build/
cd build/
cmake ../src/
make
cd -
bundle install
bundle exec rspec
