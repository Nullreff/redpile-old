BUILD_DIR := build
REDPILE := ./build/redpile
BENCHMARK := ${REDPILE} --benchmark
VALGRIND := valgrind --error-exitcode=1 --leak-check=full --show-reachable=yes
RSPEC := rspec
COMPILE := make --no-print-directory

.PHONY: all clean cmake_release cmake_debug test bench memcheck docs publish_docs

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

memtest: cmake_debug
	VALGRIND=true ${RSPEC}

memcheck: cmake_debug
	${VALGRIND} ${REDPILE} -i

bench: cmake_release
	${BENCHMARK} 1000

docs:
	./docs/generate.rb

publish: docs
	scp docs/*.html redpile:~/webapps/redpile_org

