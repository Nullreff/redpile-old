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

#include <stdbool.h>
#include "bucket.h"
#include "redpile.h"

Bucket bucket_empty(void)
{
    return bucket_create(location_empty(), -1);
}

Bucket bucket_create(Location key, int index)
{
    return (Bucket){key, index, {NULL, NULL, NULL, NULL, NULL, NULL}, NULL};
}

BucketList* bucket_list_allocate(unsigned int size)
{
    BucketList* buckets = malloc(sizeof(BucketList));
    CHECK_OOM(buckets);

    buckets->size = size;
    buckets->index = size / 2;
    buckets->hashmap_size = buckets->index;
    buckets->data = malloc(size * sizeof(Bucket));
    CHECK_OOM(buckets->data);

    int i;
    for (i = 0; i < size; i++)
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

// Currently supports resizing only
void bucket_list_resize(BucketList* buckets, int new_size)
{
    assert(new_size > buckets->size);

    BucketList* new_buckets = bucket_list_allocate(new_size);

    // TODO: Rebuild hashmap

    free(buckets->data);
    memcpy(buckets, new_buckets, sizeof(BucketList));
    free(new_buckets);
}

void bucket_list_copy(BucketList* dest, BucketList* source)
{
    memcpy(dest, source, sizeof(BucketList));
}

void bucket_list_update_adjacent(BucketList* buckets, Bucket* bucket, bool force)
{
    int i;
    for (i = 0; i < 6; i++)
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

Bucket* bucket_list_next(BucketList* buckets)
{
    if (buckets->index >= buckets->size)
    {
        bucket_list_resize(buckets, buckets->size * 2);
    }

    return buckets->data + buckets->index++;
}

// Finds the bucket used to store the block at the specified location
// If the bucket can't be found and allocate is true, a new bucket
// will be created.  If allocate is false, it will return NULL.
Bucket* bucket_list_get(BucketList* buckets, Location key, bool allocate)
{
    int hash = location_hash(key, buckets->hashmap_size);
    Bucket* bucket = buckets->data + hash;

    if (bucket->index == -1)
    {
        bucket->key = key;
    }
    else
    {
        while (!location_equals(bucket->key, key))
        {
            if (bucket->next == NULL)
            {
                if (allocate)
                {
                    bucket->next = bucket_list_next(buckets);
                    bucket->next->key = key;
                    bucket_list_update_adjacent(buckets, bucket, false);
                }
                bucket = bucket->next;
                break;
            }

            bucket = bucket->next;
        }
    }

    return bucket;
}

