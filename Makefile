BUILD_DIR := build/
BENCHMARK := ./build/redpile --benchmark
VALGRIND := valgrind --error-exitcode=1 --leak-check=full ${BENCHMARK}
RSPEC := rspec
COMPILE := make --no-print-directory

all: build

build_dir:
	mkdir -p ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR}

build: build_dir
	cd ${BUILD_DIR}; cmake -DCMAKE_BUILD_TYPE=RELEASE .. && ${COMPILE}

debug_build: build_dir
	cd ${BUILD_DIR}; cmake -DCMAKE_BUILD_TYPE=DEBUG .. && ${COMPILE}

test: debug_build
	${RSPEC}

bench: build
	${BENCHMARK}

memcheck: debug_build
	${VALGRIND}

