/* bucket.c - Location based hashmap with fast access to nearby buckets
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

#include "bucket.h"
#include "redpile.h"

static Bucket bucket_empty(void)
{
    return (Bucket){location_empty(), NULL, NULL};
}

HashMap* bucket_list_allocate(unsigned int size)
{
    HashMap* buckets = malloc(sizeof(HashMap));
    CHECK_OOM(buckets);

    // General
    buckets->size = size;
    buckets->min_size = size;
    buckets->overflow = 0;
    buckets->resizes = 0;
    buckets->max_depth = 0;
    buckets->data = malloc(size * sizeof(Bucket));
    CHECK_OOM(buckets->data);

    for (int i = 0; i < size; i++)
        buckets->data[i] = bucket_empty();

    return buckets;
}

static void bucket_list_free_data(HashMap* buckets)
{
    for (int i = 0; i < buckets->size; i++)
    {
        Bucket* current = buckets->data[i].next;
        while (current != NULL)
        {
            Bucket* temp = current->next;
            free(current);
            current = temp;
        }
    }

    free(buckets->data);
}

void bucket_list_free(HashMap* buckets)
{
    bucket_list_free_data(buckets);
    free(buckets);
}

void bucket_list_resize(HashMap* buckets, unsigned int new_size)
{
    HashMap* new_buckets = bucket_list_allocate(new_size);
    new_buckets->resizes = buckets->resizes + 1;
    new_buckets->min_size = buckets->min_size;

    for (int i = 0; i < buckets->size; i++)
    {
        Bucket* bucket = buckets->data + i;
        do
        {
            if BUCKET_FILLED(bucket)
            {
                Bucket* new_bucket = bucket_list_get(new_buckets, bucket->key, true);
                new_bucket->value = bucket->value;
            }
            bucket = bucket->next;
        }
        while (bucket != NULL);
    }

    bucket_list_free_data(buckets);
    memcpy(buckets, new_buckets, sizeof(HashMap));
    free(new_buckets);
}

Bucket* bucket_add_next(HashMap* buckets, Bucket* bucket)
{
    buckets->overflow++;
    Bucket* new_bucket = malloc(sizeof(Bucket));
    *new_bucket = bucket_empty();
    bucket->next = new_bucket;
    return new_bucket;
}

// Finds the bucket used to store the block at the specified location
// If the bucket can't be found and create is true, a new bucket
// will be created.  If create is false, it will return NULL.
Bucket* bucket_list_get(HashMap* buckets, Location key, bool create)
{
    if (buckets->overflow > buckets->size)
        bucket_list_resize(buckets, buckets->size * 2);

    int depth = 0;
    int hash = location_hash(key, buckets->size);
    Bucket* bucket = buckets->data + hash;

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

                bucket = bucket_add_next(buckets, bucket);
                bucket->key = key;
                break;
            }

            bucket = bucket->next;
            depth++;
        }
    }

    if (buckets->max_depth < depth)
        buckets->max_depth = depth;

    return bucket;
}

void* bucket_list_remove(HashMap* buckets, Location key)
{
    // Resize down the bucket array
    if (buckets->overflow == 0 && buckets->size > buckets->min_size)
    {
        int half_size = buckets->size / 2;
        int new_size = buckets->min_size > half_size ? buckets->min_size : half_size;
        bucket_list_resize(buckets, new_size);
    }

    int hash = location_hash(key, buckets->size);
    Bucket* bucket = buckets->data + hash;
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
        buckets->overflow--;
    }
    else if (bucket->next != NULL)
    {
        last_bucket = bucket->next;
        memcpy(bucket, bucket->next, sizeof(Bucket));
        free(last_bucket);
        buckets->overflow--;
    }
    else
    {
        *bucket = bucket_empty();
    }

    return value;
}
