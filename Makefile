BUILD_DIR := build/

all: build

build_dir:
	mkdir -p ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR}

build: build_dir
	cd ${BUILD_DIR}; cmake .. && make --no-print-directory

test: build_dir
	cd ${BUILD_DIR}; cmake -DCMAKE_BUILD_TYPE:STRING=Debug .. && make --no-print-directory
	rspec && ./build/redpile --benchmark
