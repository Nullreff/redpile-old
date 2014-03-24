#ifndef REDPILE_INSTRUCTION_H
#define REDPILE_INSTRUCTION_H

#include "location.h"

typedef enum {
    CMD_ON,
    CMD_OFF,
    CMD_TOGGLE,
    CMD_TICK
} Command;

typedef struct {
    Command cmd;
    Location target;
} Instruction;

int command_parse(char* command, Command* result);
int instruction_parse(char* instruction, Instruction* result);

#endif

