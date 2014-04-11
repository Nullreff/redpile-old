/* bucket.c - Linked list with access to adjacent elements
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

#include <stdlib.h>
#include "bucket.h"

Bucket bucket_empty(void)
{
    return bucket_create(-1);
}

Bucket bucket_create(int index)
{
    return (Bucket){index, {NULL, NULL, NULL, NULL, NULL, NULL}, NULL};
}

void bucket_allocate(Bucket** bucket, int index)
{
    *bucket = malloc(sizeof(Bucket));
    **bucket = bucket_create(index);
}

