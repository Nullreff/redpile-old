/* common.h - Common includes and macros
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redpile nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef REDPILE_COMMON_H
#define REDPILE_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define WARN(...) fprintf(stderr, __VA_ARGS__)
#define WARN_IF(CONDITION, ...) if (CONDITION) { WARN(__VA_ARGS__); }
#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); redpile_cleanup(); exit(EXIT_FAILURE); } while(0)
#define ERROR_IF(CONDITION, ...) if (CONDITION) { ERROR(__VA_ARGS__); }
#define CHECK_OOM(POINTER) ERROR_IF(!POINTER, "Out of memory!\n")
#define UNUSED __attribute__((unused))

// From http://stackoverflow.com/a/365068
#define ROUND_TO_POW_2(x)\
    x--;\
    x |= x >> 1;\
    x |= x >> 2;\
    x |= x >> 4;\
    x |= x >> 8;\
    x |= x >> 16;\
    x++
#define IS_POWER_OF_TWO(x) ((x & (x - 1)) == 0)

void redpile_cleanup(void);

#endif
