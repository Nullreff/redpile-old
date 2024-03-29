/* command.h - Command line instruction dispatcher
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

#ifndef REDPILE_COMMAND_H
#define REDPILE_COMMAND_H

#include "world.h"
#include "tick.h"
#include "location.h"

typedef struct {
    char* name;
    char* value;
} CommandArg;

typedef struct {
    unsigned int count;
    unsigned int index;
    CommandArg data[];
} CommandArgs;

CommandArgs* command_args_allocate(unsigned int count);
void command_args_free(CommandArgs* args);
void command_args_append(CommandArgs* args, char* name, char* value);

void command_ping(void);
void command_status(void);
void command_node_get(Region* region);
void command_node_set(Region* region, char* type, CommandArgs* fields);
void command_field_get(Region* region, char* name);
void command_field_set(Region* region, char* name, char* value);
void command_delete(Region* region);
void command_plot(Region* region, char* field);
void command_tick(int count, LogLevel log_level);
void command_message(void);
void command_type_list(void);
void command_type_show(char* name);

void command_error(const char* message);

#endif
