CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(redpile)

FILE(GLOB SOURCE_FILES src/*.c)

INCLUDE_DIRECTORIES(deps/linenoise)

ADD_DEFINITIONS(-std=gnu99 -Wall)
ADD_EXECUTABLE(redpile
    ${SOURCE_FILES}
    deps/linenoise/linenoise.c
)

