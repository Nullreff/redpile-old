/* type.c - Type information and behavior dispatch
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redpile nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "type.h"

static Fields* fields_allocate(unsigned int count)
{
    Fields* fields = malloc(sizeof(Fields) + (sizeof(FieldType) * count));
    fields->count = count;
    return fields;
}

static Behaviors* behaviors_allocate(unsigned int count)
{
    Behaviors* behaviors = malloc(sizeof(Behaviors) + (sizeof(Behavior*) * count));
    behaviors->count = count;
    return behaviors;
}

Field field_type_create(char* name, FieldType type)
{
    return (Field){name, type};
}

TypeData* type_data_allocate(void)
{
    TypeData* type_data = malloc(sizeof(TypeData));
    type_data->type_count = 0;
    type_data->behavior_count = 0;
    type_data->message_type_count = 0;
    type_data->types = NULL;
    type_data->behaviors = NULL;
    type_data->message_types = NULL;
    type_data->default_type = NULL;

    type_data_append_message_type(type_data, strdup("SYSTEM_MOVE"));
    type_data_append_message_type(type_data, strdup("SYSTEM_FIELD"));
    type_data_append_message_type(type_data, strdup("SYSTEM_REMOVE"));
    type_data_append_message_type(type_data, strdup("SYSTEM_DATA"));

    return type_data;
}

void type_data_free(TypeData* type_data)
{
    Type* type = type_data->types;
    while (type != NULL)
    {
        Type* temp = type->next;
        free(type->name);
        free(type->behaviors);
        for (unsigned int i = 0; i < type->fields->count; i++)
            free(type->fields->data[i].name);
        free(type->fields);
        free(type);
        type = temp;
    }

    Behavior* behavior = type_data->behaviors;
    while (behavior != NULL)
    {
        Behavior* temp = behavior->next;
        free(behavior->name);
        free(behavior);
        behavior = temp;
    }

    MessageType* message_type = type_data->message_types;
    while (message_type != NULL)
    {
        MessageType* temp = message_type->next;
        free(message_type->name);
        free(message_type);
        message_type = temp;
    }

    free(type_data);
}

Type* type_data_append_type(TypeData* type_data, char* name, unsigned int field_count, unsigned int behavior_count)
{
    Type* type = malloc(sizeof(Type));
    type->name = name;
    type->fields = fields_allocate(field_count);
    type->behaviors = behaviors_allocate(behavior_count);
    type->behavior_mask = 0;

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

MessageType* type_data_append_message_type(TypeData* type_data, char* name)
{
    MessageType* message_type = malloc(sizeof(MessageType));
    message_type->name = name;
    message_type->id = 1 << type_data->message_type_count;

    message_type->next = type_data->message_types;
    type_data->message_types = message_type;
    type_data->message_type_count++;
    return message_type;
}

MessageType* type_data_find_message_type(TypeData* type_data, const char* name)
{
    FOR_MESSAGE_TYPES(message_type, type_data)
    {
        if (strcasecmp(name, message_type->name) == 0)
            return message_type;
    }
    return NULL;
}

Type** type_data_type_indexes_allocate(TypeData* type_data)
{
    Type** indexes = malloc(sizeof(Type*) * type_data->type_count);

    Type* type = type_data->types;
    for (unsigned int i = 0; i < type_data->type_count; i++)
    {
        assert(type != NULL);
        indexes[i] = type;
        type = type->next;
    }

    return indexes;
}

Type* type_data_find_type(TypeData* type_data, const char* name)
{
    FOR_TYPES(type, type_data)
    {
        if (strcasecmp(name, type->name) == 0)
            return type;
    }
    return NULL;
}

void type_data_set_default_type(TypeData* type_data, Type* type)
{
    type_data->default_type = type;
}

Type* type_data_get_default_type(TypeData* type_data)
{
    return type_data->default_type;
}

Behavior* type_data_find_behavior(TypeData* type_data, const char* name)
{
    FOR_BEHAVIORS(behavior, type_data)
    {
        if (strcasecmp(name, behavior->name) == 0)
            return behavior;
    }
    return NULL;
}

Field* type_find_field(Type* type, const char* name, unsigned int* index)
{
    for (unsigned int i = 0; i < type->fields->count; i++)
    {
        Field* field = type->fields->data + i;
        if (strcasecmp(field->name, name) == 0)
        {
            *index = i;
            return field;
        }
    }

    return NULL;
}
