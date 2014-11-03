/* hashmap.c - Location based hashmap
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

#include "hashmap.h"

static Bucket bucket_empty(void)
{
    return (Bucket){location_empty(), NULL, NULL};
}

static Bucket* hashmap_add_next(Hashmap* hashmap, Bucket* bucket)
{
    hashmap->overflow++;
    Bucket* new_bucket = malloc(sizeof(Bucket));
    CHECK_OOM(new_bucket);
    *new_bucket = bucket_empty();
    bucket->next = new_bucket;
    return new_bucket;
}

static void hashmap_free_buckets(Hashmap* hashmap, void (*free_values)(void* value))
{
    for (unsigned int i = 0; i < hashmap->size; i++)
    {
        Bucket* current = hashmap->data + i;
        if (free_values && current->value != NULL)
            free_values(current->value);

        current = current->next;
        while (current != NULL)
        {
            Bucket* temp = current->next;
            if (free_values)
                free_values(current->value);
            free(current);
            current = temp;
        }
    }

    free(hashmap->data);
}

Hashmap* hashmap_allocate(unsigned int size)
{
    Hashmap* hashmap = malloc(sizeof(Hashmap));
    CHECK_OOM(hashmap);

    // General
    hashmap->size = size;
    hashmap->min_size = size;
    hashmap->overflow = 0;
    hashmap->resizes = 0;
    hashmap->max_depth = 0;
    hashmap->data = calloc(1, size * sizeof(Bucket));
    CHECK_OOM(hashmap->data);

    return hashmap;
}

void hashmap_free(Hashmap* hashmap, void (*free_values)(void* value))
{
    hashmap_free_buckets(hashmap, free_values);
    free(hashmap);
}

void hashmap_resize(Hashmap* hashmap, unsigned int new_size)
{
    Hashmap* new_hashmap = hashmap_allocate(new_size);
    new_hashmap->resizes = hashmap->resizes + 1;
    new_hashmap->min_size = hashmap->min_size;

    for (unsigned int i = 0; i < hashmap->size; i++)
    {
        Bucket* bucket = hashmap->data + i;
        if (bucket->value == NULL)
            continue;

        do
        {
            Bucket* new_bucket = hashmap_get(new_hashmap, bucket->key, true);
            new_bucket->value = bucket->value;
            bucket = bucket->next;
        }
        while (bucket != NULL);
    }

    hashmap_free_buckets(hashmap, NULL);
    memcpy(hashmap, new_hashmap, sizeof(Hashmap));
    free(new_hashmap);
}

Bucket* hashmap_get(Hashmap* hashmap, Location key, bool create)
{
    if (create && hashmap->overflow > hashmap->size)
        hashmap_resize(hashmap, hashmap->size * 2);

    unsigned int depth = 0;
    int hash = location_hash(key, hashmap->size);
    Bucket* bucket = hashmap->data + hash;

    if (bucket->value == NULL)
    {
        if (!create)
            return NULL;

        bucket->key = key;
    }
    else
    {
        while (!location_equals(bucket->key, key))
        {
            if (bucket->next == NULL)
            {
                if (!create)
                    return NULL;

                bucket = hashmap_add_next(hashmap, bucket);
                bucket->key = key;
                break;
            }

            bucket = bucket->next;
            depth++;
        }
    }

    if (hashmap->max_depth < depth)
        hashmap->max_depth = depth;

    return bucket;
}

void* hashmap_remove(Hashmap* hashmap, Location key)
{
    // Resize down the bucket array
    if (hashmap->overflow == 0 && hashmap->size > hashmap->min_size)
    {
        unsigned int half_size = hashmap->size / 2;
        unsigned int new_size = hashmap->min_size > half_size ? hashmap->min_size : half_size;
        hashmap_resize(hashmap, new_size);
    }

    int hash = location_hash(key, hashmap->size);
    Bucket* bucket = hashmap->data + hash;
    Bucket* last_bucket = NULL;

    if (bucket->value == NULL)
        return NULL;

    while (!location_equals(bucket->key, key))
    {
        if (bucket->next == NULL)
            return NULL;

        last_bucket = bucket;
        bucket = bucket->next;
    }

    void* value = bucket->value;

    if (last_bucket != NULL)
    {
        last_bucket->next = bucket->next;
        free(bucket);
        hashmap->overflow--;
    }
    else if (bucket->next != NULL)
    {
        last_bucket = bucket->next;
        memcpy(bucket, bucket->next, sizeof(Bucket));
        free(last_bucket);
        hashmap->overflow--;
    }
    else
    {
        *bucket = bucket_empty();
    }

    return value;
}
