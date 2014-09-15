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

#ifndef REDPILE_TYPE_H
#define REDPILE_TYPE_H

#include "common.h"
#include "location.h"

#define FIELD_DATA_COUNT 3
typedef enum {
    FIELD_INTEGER,
    FIELD_DIRECTION,
    FIELD_STRING
} FieldType;

typedef union {
    int integer;
    Direction direction;
    char* string;
} FieldValue;

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
    unsigned int behavior_mask;
    Fields* fields;
    Behaviors* behaviors;
} Type;

typedef struct MessageType {
    struct MessageType* next;
    char* name;
    unsigned int id;
} MessageType;

typedef struct {
    unsigned int type_count;
    unsigned int behavior_count;
    unsigned int message_type_count;
    Type* types;
    Behavior* behaviors;
    MessageType* message_types;
    Type* default_type;
} TypeData;

#define FOR_TYPE(TYPE,DATA) for (Type* TYPE = (DATA)->types; TYPE != NULL; TYPE = TYPE->next)
#define FOR_BEHAVIOR(BEHAVIOR,DATA) for (Behavior* BEHAVIOR = (DATA)->behaviors; BEHAVIOR != NULL; BEHAVIOR = BEHAVIOR->next)

Field field_type_create(char* name, FieldType type);
TypeData* type_data_allocate(void);
void type_data_free(TypeData* type_data);
Type* type_data_append_type(TypeData* type_data, char* name, unsigned int field_count, unsigned int behavior_count);
Behavior* type_data_append_behavior(TypeData* type_data, char* name, unsigned int mask, int function_ref);
MessageType* type_data_append_message_type(TypeData* type_data, char* name);
Type** type_data_type_indexes_allocate(TypeData* type_data);
Type* type_data_find_type(TypeData* type_data, const char* name);
void type_data_set_default_type(TypeData* type_data, Type* type);
Type* type_data_get_default_type(TypeData* type_data);
Behavior* type_data_find_behavior(TypeData* type_data, const char* name);

bool type_find_field(Type* type, const char* name, int* index, FieldType* field_type);

#endif
