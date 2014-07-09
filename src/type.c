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

BehaviorList* behavior_list_allocate(unsigned int count)
{
    BehaviorList* behaviors = malloc(sizeof(TypeList) + (sizeof(Type) * count));
    behaviors->count = count;
    return behaviors;
}

BehaviorList* behavior_list_realloc(BehaviorList* behaviors, unsigned int count)
{
    behaviors = realloc(behaviors, sizeof(BehaviorList) + (sizeof(Behavior) * count));
    behaviors->count = count;
    return behaviors;
}

void behavior_list_free(BehaviorList* behaviors)
{
    for (int i = 0; i < behaviors->count; i++)
        free(behaviors->data[i].name);
    free(behaviors);
}

TypeList* type_list_allocate(unsigned int count)
{
    TypeList* types = malloc(sizeof(TypeList) + (sizeof(Type) * count));
    types->count = count;
    return types;
}

TypeList* type_list_realloc(TypeList* types, unsigned int count)
{
    types = realloc(types, sizeof(TypeList) + (sizeof(Type) * count));
    types->count = count;
    return types;
}

void type_list_free(TypeList* types)
{
    for (int i = 0; i < types->count; i++)
        free(types->data[i].name);
    free(types);
}

