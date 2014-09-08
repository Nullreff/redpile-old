/* hashmap.h - Location based hashmap
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

#ifndef REDPILE_BUCKET_H
#define REDPILE_BUCKET_H

#include "location.h"
#include "common.h"

typedef struct Bucket {
    Location key;
    void* value;
    struct Bucket* next;
} Bucket;

typedef struct {
    Bucket* data;

    // Stats
    unsigned int size;
    unsigned int min_size;
    unsigned int overflow;
    unsigned int resizes;
    unsigned int max_depth;
} Hashmap;

// From http://stackoverflow.com/a/365068
#define ROUND_TO_POW_2(x)\
    x--;\
    x |= x >> 1;\
    x |= x >> 2;\
    x |= x >> 4;\
    x |= x >> 8;\
    x |= x >> 16;\
    x++

Hashmap* hashmap_allocate(unsigned int size);
void hashmap_free(Hashmap* hashmap, void (*free_values)(void* value));
Bucket* hashmap_get(Hashmap* hashmap, Location key, bool create);
void* hashmap_remove(Hashmap* hashmap, Location key);

#endif
