BUILD_DIR := build
REDPILE := ./build/src/redpile conf/redstone.lua
BENCHMARK := ${REDPILE} --benchmark
VALGRIND := valgrind --error-exitcode=1 --leak-check=full --show-reachable=yes
RSPEC := rspec
COMPILE := make --no-print-directory

.PHONY: all clean release debug install test bench memcheck docs publish_docs

all: release

${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR}

release: ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake -DCMAKE_BUILD_TYPE=RELEASE .. && ${COMPILE}

debug: ${BUILD_DIR}
	cd ${BUILD_DIR} && cmake -DCMAKE_BUILD_TYPE=DEBUG .. && ${COMPILE}

install:
	cd ${BUILD_DIR} && make install --no-print-directory

test: debug
	${RSPEC}
	TEST_INTERACTIVE=true ${RSPEC}

memtest: debug
	TEST_VALGRIND=true ${RSPEC}
	TEST_VALGRIND=true TEST_INTERACTIVE=true ${RSPEC}

memcheck: debug
	${VALGRIND} ${REDPILE} -i

bench: release
	${BENCHMARK} 1000

docs:
	./docs/generate.rb

publish: docs
	scp docs/*.html redpile:~/webapps/redpile_org

