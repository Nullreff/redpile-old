/* message.h - Message passing and storage
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

#ifndef REDPILE_MESSAGE_H
#define REDPILE_MESSAGE_H

#include "hashmap.h"
#include "type.h"

typedef enum {
    MESSAGE_POWER  = 1 << 0,
    MESSAGE_PUSH   = 1 << 1,
    MESSAGE_PULL   = 1 << 2,
    MESSAGE_REMOVE = 1 << 3,
    MESSAGE_FIELD  = 1 << 4
} MessageType;

typedef struct {
    struct {
        Location location;
        Type* type;
    } source;
    MessageType type;
    int64_t value;
} Message;

typedef struct {
    unsigned int size;
    Message data[];
} Messages;

typedef struct MessageStore {
    Messages* messages;
    unsigned long long tick;
    struct MessageStore* next;
} MessageStore;

#define MESSAGES_ALLOC_SIZE(SIZE) (sizeof(Messages) + sizeof(Message) * (SIZE))

Messages* messages_allocate(unsigned int size);
void messages_copy(Message* dest, Messages* source);
Messages* messages_filter_copy(Messages* messages, unsigned int mask);
Messages* messages_filter_copy(Messages* messages, unsigned int mask);
bool messages_equal(Messages* first, Messages* second);
Messages* messages_resize(Messages* insts, unsigned int size);
Message* messages_find_first(Messages* messages);
Message* messages_find_max(Messages* messages);
Message* messages_find_source(Messages* messages, Location source);

MessageStore* message_store_allocate(unsigned long long tick);
void message_store_free(MessageStore* store);
Messages* message_store_find_instructions(MessageStore* store, unsigned long long tick);
MessageStore* message_store_find(MessageStore* store, unsigned long long tick);
MessageStore* message_store_discard_old(MessageStore* store, unsigned long long current_tick);

void message_type_print(MessageType type, int64_t message);

#endif
