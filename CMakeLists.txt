CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(files src/world.c src/instruction.c src/block.c src/redstone.c src/bucket.c src/location.c)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)
ADD_EXECUTABLE(redpile src/redpile.c ${files})
ADD_EXECUTABLE(redpiletest test/tests.c ${files})
ADD_EXECUTABLE(redpilebench test/bench.c ${files})
TARGET_LINK_LIBRARIES(redpilebench rt)
