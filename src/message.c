/* message.c - Message passing and storage
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

#include <stdlib.h>
#include "message.h"

Messages* messages_allocate(unsigned int size)
{
    Messages* messages = malloc(MESSAGES_ALLOC_SIZE(size));
    messages->size = size;
    return messages;
}

void messages_copy(Message* dest, Messages* source)
{
    memcpy(dest, source->data, sizeof(Message) * source->size);
}

Messages* messages_filter_copy(Messages* messages, unsigned int mask)
{
    Messages* new = messages_allocate(messages->size);
    unsigned int new_index = 0;
    for (int i = 0; i < messages->size; i++)
    {
        if ((messages->data[i].type & mask) != 0)
            memcpy(new->data + new_index++, messages->data + i, sizeof(Message));
    }
    new->size = new_index;
    return new;
}

bool messages_equal(Messages* first, Messages* second)
{
    if (first->size != second->size)
        return false;

    for (int i = 0; i < first->size; i++)
    {
        if (LOCATION_EQUALS(first->data[i].source.location, second->data[i].source.location) &&
            first->data[i].type == second->data[i].type &&
            first->data[i].message == second->data[i].message)
        {
            return true;
        }
    }
    return false;
}

Messages* messages_resize(Messages* messages, unsigned int size)
{
    messages = realloc(messages, MESSAGES_ALLOC_SIZE(size));
    CHECK_OOM(messages);
    messages->size = size;
    return messages;
}

unsigned int messages_find_first(Messages* messages)
{
    return messages->size != 0 ? messages->data[0].message : 0;
}

unsigned int messages_find_max(Messages* messages)
{
    unsigned int max = 0;
    for (int i = 0; i < messages->size; i++)
    {
        if (messages->data[i].message > max)
            max = messages->data[i].message;
    }
    return max;
}

unsigned int messages_find_source(Messages* messages, Location source)
{
    for (int i = 0; i < messages->size; i++)
    {
        if (LOCATION_EQUALS(messages->data[i].source.location, source))
            return messages->data[i].message;
    }
    return 0;
}

MessageStore* message_store_allocate(unsigned long long tick)
{
    MessageStore* store = malloc(sizeof(MessageStore));
    CHECK_OOM(store);
    store->messages = messages_allocate(0);
    store->tick = tick;
    store->next = NULL;
    return store;
}

static void message_store_free_one(MessageStore* store)
{
    free(store->messages);
    free(store);
}

void message_store_free(MessageStore* store)
{
    while (store != NULL)
    {
        MessageStore* temp = store->next;
        message_store_free_one(store);
        store = temp;
    }
}

MessageStore* message_store_find(MessageStore* store, unsigned long long tick)
{
    for (;store != NULL; store = store->next)
    {
        if (store->tick == tick)
            return store;
    }

    return NULL;
}

Messages* message_store_find_instructions(MessageStore* store, unsigned long long tick)
{
    MessageStore* found_store = message_store_find(store, tick);
    return found_store != NULL ? found_store->messages : NULL;
}

// Discard any stores that are older than the current tick
MessageStore* message_store_discard_old(MessageStore* store, unsigned long long current_tick)
{
    MessageStore* return_store = store;

    // Remove items at the head of the list
    while (true)
    {
        if (store == NULL)
            return NULL;

        if (store->tick >= current_tick)
            break;

        return_store = store->next;
        message_store_free_one(store);
        store = return_store;
    }

    // Remove items further in
    while (store->next != NULL)
    {
        if (store->next->tick >= current_tick)
        {
            store = store->next;
        }
        else
        {
            MessageStore* temp = store->next->next;
            message_store_free_one(store->next);
            store->next = temp;
        }
    }

    return return_store;
}

void message_type_print(MessageType type, unsigned int message)
{
    switch (type)
    {
        case MESSAGE_POWER:  printf("POWER %u\n", message); break;
        case MESSAGE_PUSH:   printf("PUSH %s\n", Directions[message]); break;
        case MESSAGE_PULL:   printf("PULL %s\n", Directions[message]); break;
        case MESSAGE_REMOVE: printf("REMOVE\n"); break;
    }
}

