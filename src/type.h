/* type.c - Type information and behavior dispatch
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

#ifndef REDPILE_TYPE_H
#define REDPILE_TYPE_H

#include <stdbool.h>

#define EMPTY ((Type*)NULL)

// TODO: Remove or hide circular dependancies
struct BehaviorData;

typedef struct {
    const char* name;
    unsigned int mask;
    int function_ref;
} Behavior;

typedef struct BehaviorList {
    unsigned int count;
    Behavior data[];
} BehaviorList;

typedef struct {
    const char* name;
    unsigned int field_count;
    unsigned int behavior_count;
    Behavior* behaviors;
} Type;

typedef struct {
    unsigned int count;
    Type data[];
} TypeList;

BehaviorList* behavior_list_allocate(unsigned int count);
TypeList* type_list_allocate(unsigned int count);
TypeList* type_list_realloc(TypeList* types, unsigned int count);
void type_list_free(TypeList* types);

#endif
