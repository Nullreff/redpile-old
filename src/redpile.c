/* redpile.c - Voxel logic simulator
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

#include "redpile.h"
#include "parser.h"
#include "command.h"
#include "bench.h"
#include "type.h"
#include "common.h"
#include "repl.h"
#include <getopt.h>
#include <signal.h>
#include <ctype.h>

// All global state lives in these variables
World* world = NULL;
ScriptState* state = NULL;
ReplConfig* config = NULL;

static void signal_callback(int signal)
{
    if (signal == SIGINT)
    {
        redpile_cleanup();
        exit(EXIT_SUCCESS);
    }
}

void setup_signals(void)
{
    signal(SIGINT, signal_callback);
}

// Referenced from common.h
void redpile_cleanup(void)
{
    if (world != NULL)
        world_free(world);

    if (state != NULL)
        script_state_free(state);

    repl_cleanup();

    printf("\n");
}

void set_globals(ReplConfig* config_ptr, ScriptState* state_ptr, World* world_ptr)
{
    config = config_ptr;
    state = state_ptr;
    world = world_ptr;
}

