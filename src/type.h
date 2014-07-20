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

#define FIELD_DATA_COUNT 2
typedef enum {
    FIELD_INT,
    FIELD_DIRECTION
} FieldType;

typedef struct {
    char* name;
    FieldType type;
} Field;

typedef struct {
    unsigned int count;
    Field data[];
} Fields;

typedef struct Behavior {
    struct Behavior* next;
    char* name;
    unsigned int mask;
    int function_ref;
} Behavior;

typedef struct {
    unsigned int count;
    Behavior* data[];
} Behaviors;

typedef struct Type {
    struct Type* next;
    char* name;
    Fields* fields;
    Behaviors* behaviors;
} Type;

typedef struct {
    unsigned int type_count;
    unsigned int behavior_count;
    Type* types;
    Behavior* behaviors;
    Type* default_type;
} TypeData;

#define FOR_TYPE(TYPE,DATA) for (Type* TYPE = (DATA)->types; TYPE != NULL; TYPE = TYPE->next)
#define FOR_BEHAVIOR(BEHAVIOR,DATA) for (Behavior* BEHAVIOR = (DATA)->behaviors; BEHAVIOR != NULL; BEHAVIOR = BEHAVIOR->next)

Field field_type_create(char* name, FieldType type);
TypeData* type_data_allocate(void);
void type_data_free(TypeData* type_data);
Type* type_data_append_type(TypeData* type_data, char* name, unsigned int field_count, unsigned int behavior_count);
Behavior* type_data_append_behavior(TypeData* type_data, char* name, unsigned int mask, int function_ref);
Type** type_data_type_indexes_allocate(TypeData* type_data);
Type* type_data_find_type(TypeData* type_data, const char* name);
void type_data_set_default_type(TypeData* type_data, Type* type);
Type* type_data_get_default_type(TypeData* type_data);
Behavior* type_data_find_behavior(TypeData* type_data, const char* name);

#endif
