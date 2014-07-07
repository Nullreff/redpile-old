/* message.h - Message passing and storage
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

#ifndef REDPILE_MESSAGE_H
#define REDPILE_MESSAGE_H

#include "hashmap.h"
#include "type.h"

typedef enum {
    MESSAGE_POWER  = 1 << 0,
    MESSAGE_PUSH   = 1 << 1,
    MESSAGE_PULL   = 1 << 2,
    MESSAGE_REMOVE = 1 << 3
} MessageType;

typedef struct {
    struct {
        Location location;
        Type* type;
    } source;
    MessageType type;
    unsigned int message;
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
unsigned int messages_find_first(Messages* messages);
unsigned int messages_find_max(Messages* messages);
unsigned int messages_find_source(Messages* messages, Location source);

MessageStore* message_store_allocate(unsigned long long tick);
void message_store_free(MessageStore* store);
Messages* message_store_find_instructions(MessageStore* store, unsigned long long tick);
MessageStore* message_store_find(MessageStore* store, unsigned long long tick);
MessageStore* message_store_discard_old(MessageStore* store, unsigned long long current_tick);

void message_type_print(MessageType type, unsigned int message);

#endif
