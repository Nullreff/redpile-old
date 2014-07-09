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

#include "type.h"

TypeData* type_data_allocate(void)
{
    TypeData* type_data = malloc(sizeof(TypeData));
    type_data->type_count = 0;
    type_data->behavior_count = 0;
    type_data->types = NULL;
    type_data->behaviors = NULL;
    return type_data;
}

void type_data_free(TypeData* type_data)
{
    Type* type = type_data->types;
    while (type != NULL)
    {
        Type* temp = type->next;
        free(type);
        type = temp;
    }

    Behavior* behavior = type_data->behaviors;
    while (behavior != NULL)
    {
        Behavior* temp = behavior->next;
        free(behavior);
        behavior = temp;
    }

    free(type_data);
}

Type* type_data_append_type(TypeData* type_data, char* name, unsigned int field_count, unsigned int behavior_count)
{
    Type* type = malloc(sizeof(Type) + (sizeof(Behavior*) * behavior_count));
    type->name = name;
    type->field_count = field_count;
    type->behavior_count = behavior_count;

    type->next = type_data->types;
    type_data->types = type;
    type_data->type_count++;
    return type;
}

Behavior* type_data_append_behavior(TypeData* type_data, char* name, unsigned int mask, int function_ref)
{
    Behavior* behavior = malloc(sizeof(Behavior));
    behavior->name = name;
    behavior->mask = mask;
    behavior->function_ref = function_ref;

    behavior->next = type_data->behaviors;
    type_data->behaviors = behavior;
    type_data->behavior_count++;
    return behavior;
}

Type** type_data_type_indexes_allocate(TypeData* type_data)
{
    Type** indexes = malloc(sizeof(Type*) * type_data->type_count);

    Type* type = type_data->types;
    for (int i = 0; i < type_data->type_count; i++)
    {
        assert(type != NULL);
        indexes[i] = type;
        type = type->next;
    }

    return indexes;
}

