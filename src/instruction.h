#ifndef REDPILE_INSTRUCTION_H
#define REDPILE_INSTRUCTION_H

#include "world.h"
#include "block.h"

typedef enum {
    CMD_SET,
    CMD_POWER,
    CMD_GET,
    CMD_TICK,
    CMD_STATUS
} Command;

typedef struct {
    Command cmd;
    Location target;
    int value;
} Instruction;

int command_parse(char* command, Command* result);
int instruction_parse(char* instruction, Instruction* result);
void instruction_run(World* world, Instruction* inst, void (*block_modified_callback)(Block*));

#endif

