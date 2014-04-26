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

static Bucket bucket_create(Location key, int index)
{
    return (Bucket){key, index, {NULL, NULL, NULL, NULL, NULL, NULL}, NULL};
}

static Bucket bucket_empty(void)
{
    return bucket_create(location_empty(), -1);
}

static void bucket_print(Bucket* bucket)
{
    printf("%p (%d,%d,%d) %d %p\n",
            (void*)bucket,
            bucket->key.x,
            bucket->key.y,
            bucket->key.z,
            bucket->index,
            (void*)bucket->next);
}

void bucket_list_print(BucketList* buckets, Bucket* selected)
{
    printf("---Hashmap: %d---\n", buckets->hashmap_size);

    for (int i = 0; i < buckets->size; i++)
    {
        if (i == buckets->hashmap_size)
        {
            printf("---Overflow: %d---\n", buckets->size - buckets->hashmap_size);
        }
        if (i == buckets->index)
        {
            printf("---Index: %d---\n", buckets->index - buckets->hashmap_size);
        }
        Bucket* found = buckets->data + i;
        if (found == selected)
        {
            printf("> ");
        }
        bucket_print(buckets->data + i);
    }
    printf("---End: %d---\n", buckets->size);
}

BucketList* bucket_list_allocate(unsigned int size)
{
    BucketList* buckets = malloc(sizeof(BucketList));
    CHECK_OOM(buckets);

    // General
    buckets->size = size;
    buckets->index = size - (size / 2);
    buckets->hashmap_size = buckets->index;

    // Stats
    buckets->resizes = 0;

    // Data
    buckets->data = malloc(size * sizeof(Bucket));
    CHECK_OOM(buckets->data);

    for (int i = 0; i < size; i++)
    {
        buckets->data[i] = bucket_empty();
    }

    return buckets;
}

void bucket_list_free(BucketList* buckets)
{
    free(buckets->data);
    free(buckets);
}

// Currently supports increasing only
void bucket_list_resize(BucketList* buckets, unsigned int new_size)
{
    assert(new_size > buckets->size);

    BucketList* new_buckets = bucket_list_allocate(new_size);
    new_buckets->resizes = buckets->resizes + 1;

    for (int i = 0; i < buckets->size; i++)
    {
        Bucket* bucket = buckets->data + i;
        if BUCKET_FILLED(bucket)
        {
            Bucket* new_bucket = bucket_list_get(new_buckets, bucket->key, true);
            new_bucket->index = bucket->index;
        }
    }

    free(buckets->data);
    memcpy(buckets, new_buckets, sizeof(BucketList));
    free(new_buckets);
}

void bucket_list_update_adjacent(BucketList* buckets, Bucket* bucket, bool force)
{
    for (int i = 0; i < 6; i++)
    {
        if (force || bucket->adjacent[i] == NULL)
        {
            // Find the bucket next to us
            Direction dir = (Direction)i;
            Location location = location_move(bucket->key, dir, 1);
            Bucket* found_bucket = bucket_list_get(buckets, location, false);

            if (found_bucket != NULL)
            {
                // Update it to point both ways
                found_bucket->adjacent[direction_invert(dir)] = bucket;
                bucket->adjacent[i] = found_bucket;
            }
        }
    }
}

Bucket* bucket_add_next(BucketList* buckets, Bucket* bucket)
{
    if (buckets->index >= buckets->size)
    {
        bucket_list_resize(buckets, buckets->size * 2);

        // If we reallocate our list of buckets, the pointer to the bucket
        // passed in most likely points to old memory.
        return NULL;
    }
    else
    {
        Bucket* new_bucket = buckets->data + buckets->index++;
        bucket->next = new_bucket;
        return new_bucket;
    }
}

// Finds the bucket used to store the block at the specified location
// If the bucket can't be found and create is true, a new bucket
// will be created.  If create is false, it will return NULL.
Bucket* bucket_list_get(BucketList* buckets, Location key, bool create)
{
    int hash = location_hash(key, buckets->hashmap_size);
    Bucket* bucket = buckets->data + hash;

    if (bucket->index == -1)
    {
        if (create)
        {
            bucket->key = key;
            bucket_list_update_adjacent(buckets, bucket, false);
        }
        else
        {
            bucket = NULL;
        }
    }
    else
    {
        while (!location_equals(bucket->key, key))
        {
            if (bucket->next == NULL)
            {
                if (create)
                {
                    bucket = bucket_add_next(buckets, bucket);
                    if (bucket != NULL)
                    {
                        bucket->key = key;
                        bucket_list_update_adjacent(buckets, bucket, false);
                    }
                    else
                    {
                        // A reallocation occured while we were searching,
                        // start over from the begining.
                        bucket = bucket_list_get(buckets, key, create);
                    }
                }
                else
                {
                    bucket = NULL;
                }
                break;
            }

            bucket = bucket->next;
        }
    }

    return bucket;
}

