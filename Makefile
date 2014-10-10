BUILD_DIR := build
REDPILE := ./build/src/redpile conf/redstone.lua
BENCHMARK := ${REDPILE} --benchmark
VALGRIND := valgrind --error-exitcode=1 --leak-check=full --show-reachable=yes
RSPEC := rspec
COMPILE := make --no-print-directory

.PHONY: all clean release debug run install test bench memcheck docs publish help

all: release

${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR}

release: ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake -DCMAKE_BUILD_TYPE=RELEASE .. && ${COMPILE}

debug: ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake -DCMAKE_BUILD_TYPE=DEBUG .. && ${COMPILE}

run:
	${REDPILE} -i

install:
	cd ${BUILD_DIR} && make install --no-print-directory

test: debug
	${RSPEC}
	TEST_INTERACTIVE=true ${RSPEC}

memtest: debug
	TEST_VALGRIND=true ${RSPEC}

memcheck: debug
	${VALGRIND} ${REDPILE} -i

bench: release
	${BENCHMARK} 1000

docs:
	./docs/generate.rb

publish: docs
	scp docs/*.html redpile:~/webapps/redpile_org

help:
	# release  - Build redpile in release mode
	# debug    - Build redpile in debug mode
	# clean    - Remove all build files
	# run      - Start an interactive session in redpile
	# install  - Install binaries in the local system
	# test     - Run all tests
	# memtest  - Run all tests under valgrind
	# memcheck - Run redpile under valgrind
	# bench    - Run benchmarks
	# docs     - Generate documentation
	# publish  - Publish documentation

