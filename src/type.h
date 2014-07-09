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

#include "common.h"

typedef struct Behavior {
    struct Behavior* next;
    char* name;
    unsigned int mask;
    int function_ref;
} Behavior;

typedef struct Type {
    struct Type* next;
    char* name;
    unsigned int field_count;
    unsigned int behavior_count;
    Behavior* behaviors[];
} Type;

typedef struct {
    unsigned int type_count;
    unsigned int behavior_count;
    Type* types;
    Behavior* behaviors;
} TypeData;

#define EMPTY ((Type*)NULL)
#define FOR_TYPE(TYPE,DATA) for (Type* TYPE = (DATA)->types; TYPE != NULL; TYPE = TYPE->next)

TypeData* type_data_allocate(void);
void type_data_free(TypeData* type_data);
Type* type_data_append_type(TypeData* type_data, char* name, unsigned int field_count, unsigned int behavior_count);
Behavior* type_data_append_behavior(TypeData* type_data, char* name, unsigned int mask, int function_ref);
Type** type_data_type_indexes_allocate(TypeData* type_data);

#endif
