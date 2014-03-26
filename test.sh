#!/bin/sh
./build.sh '-DCMAKE_BUILD_TYPE:STRING=Debug' && ./build/redpiletest && rspec
