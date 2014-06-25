CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(redpile)

FIND_PACKAGE(BISON REQUIRED)
SET(BisonOutput ${CMAKE_SOURCE_DIR}/src/command/parser.c)
IF(BISON_FOUND)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${BisonOutput}
      COMMAND ${BISON_EXECUTABLE}
              --defines=${CMAKE_SOURCE_DIR}/src/command/parser.h
              --output=${BisonOutput}
              ${CMAKE_SOURCE_DIR}/src/command/parser.y
      COMMENT "Generating command/parser.c"
    )
ENDIF()

FIND_PACKAGE(FLEX REQUIRED)
SET(FlexOutput ${CMAKE_SOURCE_DIR}/src/command/lexer.c)
IF(FLEX_FOUND)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${FlexOutput}
      COMMAND ${FLEX_EXECUTABLE}
              --outfile=${FlexOutput}
              ${CMAKE_SOURCE_DIR}/src/command/lexer.y
      COMMENT "Generating command/lexer.c"
    )
ENDIF()

FILE(GLOB SOURCE_FILES src/*.c)
INCLUDE_DIRECTORIES(deps/linenoise)

ADD_DEFINITIONS(-std=gnu99 -Wall)
ADD_EXECUTABLE(redpile
    ${SOURCE_FILES}
    src/command/lexer.c
    src/command/parser.c
    deps/linenoise/linenoise.c
)

