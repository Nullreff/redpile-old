BUILD_DIR := build
BENCHMARK := ./build/redpile --benchmark
VALGRIND := valgrind --error-exitcode=1 --leak-check=full ${BENCHMARK} 20
RSPEC := rspec
COMPILE := make --no-print-directory

.PHONY: all clean cmake_release cmake_debug test bench memcheck

all: cmake_release

${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR}

cmake_release: ${BUILD_DIR}
	cd ${BUILD_DIR}; cmake -DCMAKE_BUILD_TYPE=RELEASE .. && ${COMPILE}

cmake_debug: ${BUILD_DIR}
	cd ${BUILD_DIR}; cmake -DCMAKE_BUILD_TYPE=DEBUG .. && ${COMPILE}

test: cmake_debug
	${RSPEC}

bench: cmake_release
	${BENCHMARK} 50

memcheck: cmake_debug
	${VALGRIND}

