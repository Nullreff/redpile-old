CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(redpile)

FIND_PACKAGE(FLEX REQUIRED)
FIND_PACKAGE(BISON REQUIRED)

BISON_TARGET(CommandParser src/command/parser.y src/command/parser.c)
FLEX_TARGET(CommandScanner src/command/lexer.l  src/command/lexer.c)
ADD_FLEX_BISON_DEPENDENCY(CommandScanner CommandParser)

FILE(GLOB SOURCE_FILES src/*.c src/command/*.c)
INCLUDE_DIRECTORIES(deps/linenoise)

ADD_DEFINITIONS(-std=gnu99 -Wall)
ADD_EXECUTABLE(redpile
    ${SOURCE_FILES}
    ${BISON_CommandParser_OUTPUTS}
    ${FLEX_CommandScanner_OUTPUTS}
    deps/linenoise/linenoise.c
)
TARGET_LINK_LIBRARIES(redpile ${FLEX_LIBRARIES} ${BISON_LIBRARIES})

