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

typedef enum {
    MESSAGE_POWER,
    MESSAGE_MOVE,
    MESSAGE_REMOVE
} MessageType;

typedef struct {
    struct {
        Location location;
        Type type;
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
Messages* messages_resize(Messages* insts, unsigned int size);
unsigned int messages_max_power(Messages* inst);
bool messages_power_check(Messages* insts, Location loc, unsigned int power);
Message* messages_find_move(Messages* insts);

MessageStore* message_store_allocate(unsigned long long tick);
void message_store_free(MessageStore* queue);
Messages* message_store_find_instructions(MessageStore* queue, unsigned long long tick);
MessageStore* message_store_find(MessageStore* queue, unsigned long long tick);
MessageStore* message_store_discard_old(MessageStore* queue, unsigned long long current_tick);

void message_type_print(MessageType type, unsigned int message);

#endif
