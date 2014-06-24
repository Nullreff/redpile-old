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

Messages* messages_resize(Messages* messages, unsigned int size)
{
    messages = realloc(messages, MESSAGES_ALLOC_SIZE(size));
    CHECK_OOM(messages);
    messages->size = size;
    return messages;
}

unsigned int messages_max_power(Messages* messages)
{
    unsigned int max = 0;
    for (int i = 0; i < messages->size; i++)
    {
        if (messages->data[i].type == MESSAGE_POWER && messages->data[i].message > max)
            max = messages->data[i].message;
    }
    return max;
}

bool messages_power_check(Messages* messages, Location loc, unsigned int power)
{
    for (int i = 0; i < messages->size; i++)
    {
        if (LOCATION_EQUALS(messages->data[i].source.location, loc) &&
            messages->data[i].type == MESSAGE_POWER &&
            messages->data[i].message >= power)
            return false;
    }
    return true;
}

Message* messages_find_move(Messages* messages)
{
    for (int i = 0; i < messages->size; i++)
    {
        if (messages->data[i].type == MESSAGE_MOVE)
            return messages->data + i;
    }
    return NULL;
}

MessageStore* message_store_allocate(unsigned long long tick)
{
    MessageStore* queue = malloc(sizeof(MessageStore));
    CHECK_OOM(queue);
    queue->messages = messages_allocate(0);
    queue->tick = tick;
    queue->next = NULL;
    return queue;
}

static void message_store_free_one(MessageStore* queue)
{
    free(queue->messages);
    free(queue);
}

void message_store_free(MessageStore* queue)
{
    while (queue != NULL)
    {
        MessageStore* temp = queue->next;
        message_store_free_one(queue);
        queue = temp;
    }
}

MessageStore* message_store_find(MessageStore* queue, unsigned long long tick)
{
    for (;queue != NULL; queue = queue->next)
    {
        if (queue->tick == tick)
            return queue;
    }

    return NULL;
}

Messages* message_store_find_instructions(MessageStore* queue, unsigned long long tick)
{
    MessageStore* found_queue = message_store_find(queue, tick);
    return found_queue != NULL ? found_queue->messages : NULL;
}

// Discard any queues that are older than the current tick
MessageStore* message_store_discard_old(MessageStore* queue, unsigned long long current_tick)
{
    MessageStore* return_queue = queue;

    // Remove items at the head of the list
    while (true)
    {
        if (queue == NULL)
            return NULL;

        if (queue->tick >= current_tick)
            break;

        return_queue = queue->next;
        message_store_free_one(queue);
        queue = return_queue;
    }

    // Remove items further in
    while (queue->next != NULL)
    {
        if (queue->next->tick >= current_tick)
        {
            queue = queue->next;
        }
        else
        {
            MessageStore* temp = queue->next->next;
            message_store_free_one(queue->next);
            queue->next = temp;
        }
    }

    return return_queue;
}

void message_type_print(MessageType type, unsigned int message)
{
    switch (type)
    {
        case MESSAGE_POWER:  printf("POWER %u\n", message); break;
        case MESSAGE_MOVE:   printf("MOVE %s\n", Directions[message]); break;
        case MESSAGE_REMOVE: printf("REMOVE\n"); break;
    }
}

