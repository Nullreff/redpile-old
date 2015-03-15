FIND_PACKAGE(FLEX REQUIRED)
FIND_PACKAGE(BISON REQUIRED)

BISON_TARGET(CommandParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
FLEX_TARGET(CommandScanner lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)
ADD_FLEX_BISON_DEPENDENCY(CommandScanner CommandParser)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB C_FILES *.c)
ADD_LIBRARY(redpile-core STATIC ${C_FILES} ${FLEX_CommandScanner_OUTPUTS} ${BISON_CommandParser_OUTPUTS})

SET(RUST_ARGS -L ./ -L ../deps/lua -L ../deps/linenoise --extern getopts=../deps/getopts/target/release/libgetopts-4f06c9c36f482ad0.rlib)
IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    SET(RUST_ARGS -g ${RUST_ARGS})
ENDIF()
ADD_CUSTOM_TARGET(redpile ALL
    COMMAND rustc ${CMAKE_CURRENT_SOURCE_DIR}/redpile.rs ${RUST_ARGS}
    DEPENDS redpile-core lua linenoise)
