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

static Bucket bucket_create(Location key, BlockNode* node)
{
    return (Bucket){key, node, NULL};
}

static Bucket bucket_empty(void)
{
    return bucket_create(location_empty(), NULL);
}

static void bucket_print(Bucket* bucket)
{
    printf("%p (%d,%d,%d) %p %p\n",
            (void*)bucket,
            bucket->key.x,
            bucket->key.y,
            bucket->key.z,
            (void*)bucket->value,
            (void*)bucket->next);
}

BucketList* bucket_list_allocate(unsigned int size)
{
    BucketList* buckets = malloc(sizeof(BucketList));
    CHECK_OOM(buckets);

    // General
    buckets->size = size;
    buckets->overflow = 0;
    buckets->resizes = 0;
    buckets->max_depth = 0;
    buckets->data = malloc(size * sizeof(Bucket));
    CHECK_OOM(buckets->data);

    for (int i = 0; i < size; i++)
        buckets->data[i] = bucket_empty();

    return buckets;
}

static void bucket_list_free_data(BucketList* buckets)
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

void bucket_list_free(BucketList* buckets)
{
    bucket_list_free_data(buckets);
    free(buckets);
}

void bucket_list_resize(BucketList* buckets, unsigned int new_size)
{
    BucketList* new_buckets = bucket_list_allocate(new_size);
    new_buckets->resizes = buckets->resizes + 1;

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
    memcpy(buckets, new_buckets, sizeof(BucketList));
    free(new_buckets);
}

Bucket* bucket_add_next(BucketList* buckets, Bucket* bucket)
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
Bucket* bucket_list_get(BucketList* buckets, Location key, bool create)
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

