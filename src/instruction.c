#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "instruction.h"
#include "location.h"
#include "redpile.h"

#define PARSE_NUMBER(NAME)\
    char* str_ ## NAME = strtok(NULL, " ");\
    if (str_ ## NAME == NULL)\
        goto error;\
    NAME = strtol(str_ ## NAME , &parse_error, 10);\
    if (*parse_error)\
        goto error;

int command_parse(char* command, Command* result)
{
    if (strcmp(command, "ON") == 0)
        *result = CMD_ON;
    else if (strcmp(command, "OFF") == 0)
        *result = CMD_OFF;
    else if (strcmp(command, "TOGGLE") == 0)
        *result = CMD_TOGGLE;
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
    long x, y, z;

    char* str_command = strtok(parts, " ");
    if (str_command == NULL || command_parse(str_command, &command) == -1)
        goto error;

    if (command == CMD_TICK)
    {
        x = y = z = 0;
        goto success;
    }

    char* parse_error;
    PARSE_NUMBER(x)
    PARSE_NUMBER(y)
    PARSE_NUMBER(z)

success:
    *result = (Instruction){command, (Location){x, y, z}};
    free(parts_ptr);
    return 0;

error:
    free(parts_ptr);
    return -1;
}
