CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(redpile)

# Flex likes to generate code with comparison of signed and unsigned integers, this supresses it
ADD_DEFINITIONS(-std=gnu99 -Wall -Wextra -Werror -DYY_TYPEDEF_YY_SIZE_T -Dyy_size_t=ssize_t)

INCLUDE_DIRECTORIES(deps/lua)
INCLUDE_DIRECTORIES(deps/linenoise)

ADD_SUBDIRECTORY(deps/lua)
ADD_SUBDIRECTORY(deps/linenoise)
ADD_SUBDIRECTORY(src)

