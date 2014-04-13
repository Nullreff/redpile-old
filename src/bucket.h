/* bucket.h - Storage and fast access to blocks
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

#ifndef REDPILE_BUCKET_H
#define REDPILE_BUCKET_H

#define BUCKET_FILLED(bucket) (bucket != NULL && bucket->index != -1)

typedef struct Bucket {
    int index;
    struct Bucket* adjacent[6];
    struct Bucket* next;
} Bucket;

Bucket bucket_empty(void);
Bucket bucket_create(int index);
void bucket_allocate(Bucket** bucket, int index);

#endif
