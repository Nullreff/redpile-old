/* hashmap.c - Location based hashmap
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hashmap.h"
#include "redpile.h"

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
    for (int i = 0; i < hashmap->size; i++)
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
    hashmap->data = malloc(size * sizeof(Bucket));
    CHECK_OOM(hashmap->data);

    for (int i = 0; i < size; i++)
        hashmap->data[i] = bucket_empty();

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

    for (int i = 0; i < hashmap->size; i++)
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
    if (hashmap->overflow > hashmap->size)
        hashmap_resize(hashmap, hashmap->size * 2);

    int depth = 0;
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
        while (!LOCATION_EQUALS(bucket->key, key))
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
        int half_size = hashmap->size / 2;
        int new_size = hashmap->min_size > half_size ? hashmap->min_size : half_size;
        hashmap_resize(hashmap, new_size);
    }

    int hash = location_hash(key, hashmap->size);
    Bucket* bucket = hashmap->data + hash;
    Bucket* last_bucket = NULL;

    if (bucket->value == NULL)
        return NULL;

    while (!LOCATION_EQUALS(bucket->key, key))
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
