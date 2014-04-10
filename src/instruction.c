#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "instruction.h"
#include "redpile.h"
#include "block.h"
#include "world.h"
#include "redstone.h"

#define PARSE_NUMBER(NAME)\
    char* str_ ## NAME = strtok(NULL, " ");\
    if (str_ ## NAME == NULL)\
        goto error;\
    NAME = strtol(str_ ## NAME , &parse_error, 10);\
    if (*parse_error)\
        goto error;

int command_parse(char* command, Command* result)
{
    if (strcmp(command, "SET") == 0)
        *result = CMD_SET;
    else if (strcmp(command, "GET") == 0)
        *result = CMD_GET;
    else if (strcmp(command, "TICK") == 0)
        *result = CMD_TICK;
    else
        return -1;

    return 0;
}

int instruction_parse(char* instruction, Instruction* result)
{
    char* parts = strdup(instruction);
    char* parts_ptr = parts;
    CHECK_OOM(parts);

    Command command;
    int x, y, z, value;
    x = y = z = value = 0;

    char* str_command = strtok(parts, " ");
    if (str_command == NULL || command_parse(str_command, &command) == -1)
        goto error;

    if (command == CMD_TICK)
    {
        goto success;
    }

    char* parse_error;
    PARSE_NUMBER(x)
    PARSE_NUMBER(y)
    PARSE_NUMBER(z)

    if (command == CMD_GET)
    {
        goto success;
    }

    PARSE_NUMBER(value)

success:
    *result = (Instruction){command, (Location){x, y, z}, value};
    free(parts_ptr);
    return 0;

error:
    free(parts_ptr);
    return -1;
}

void instruction_run(World* world, Instruction* inst, void (*block_modified_callback)(Block*))
{
    Block new_block;
    Block* block;

    switch (inst->cmd)
    {
        case CMD_SET:
            new_block = (Block){(Material)inst->value, inst->target, 0};
            block = world_add_block(world, &new_block);
            block_modified_callback(block);
            return;

        case CMD_GET:
            block = world_get_block(world, inst->target);
            if (block == NULL)
            {
                new_block = (Block){M_EMPTY, inst->target, 0};
                block_modified_callback(&new_block);
            }
            else
            {
                block_modified_callback(block);
            }
            break;

        case CMD_TICK:
            redstone_tick(world, block_modified_callback);
            break;
    }
}

