/* message.c - Message passing and storage
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

#include <stdlib.h>
#include "message.h"
#include "repl.h"

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
    for (unsigned int i = 0; i < messages->size; i++)
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

    for (unsigned int i = 0; i < first->size; i++)
    {
        if (location_equals(first->data[i].source.location, second->data[i].source.location) &&
            first->data[i].type == second->data[i].type &&
            first->data[i].value == second->data[i].value)
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

Message* messages_find_first(Messages* messages)
{
    return messages->size != 0 ? messages->data : NULL;
}

Message* messages_find_max(Messages* messages)
{
    unsigned int max = 0;
    Message* found = NULL;
    for (unsigned int i = 0; i < messages->size; i++)
    {
        if (messages->data[i].value > max)
        {
            max = messages->data[i].value;
            found = messages->data + i;
        }
    }
    return found;
}

Message* messages_find_source(Messages* messages, Location source)
{
    for (unsigned int i = 0; i < messages->size; i++)
    {
        if (location_equals(messages->data[i].source.location, source))
            return messages->data + i;
    }
    return NULL;
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

