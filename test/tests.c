#include <stdio.h>
#include <stdlib.h>
#include "minunit.h"
#include "common.h"
#include "../src/instruction.h"
#include "../src/location.h"
#include "../src/world.h"
#include "../src/block.h"

int tests_run = 0;
char* message;

#define TEST_COMMAND(cmd) do {\
    int success = command_parse(#cmd, &command);\
    MU_ASSERT("Failed to parse the '"#cmd"' command", success == 0);\
    MU_ASSERT("Incorrect result when parsing the '"#cmd"' command", command == CMD_ ## cmd);\
} while(0);

static char* test_command_parsing()
{
    Command command;

    TEST_COMMAND(ON)
    TEST_COMMAND(OFF)
    TEST_COMMAND(TOGGLE)
    TEST_COMMAND(TICK)
    MU_ASSERT("Invalid command didn't fail parsing", 
            command_parse("BADCMD", &command) == -1);

    return 0;
}

#define TEST_INSTRUCTION_LOCATION(inst,coord,val) \
    MU_ASSERT("Incorrect "#coord" when parsing the '"#inst"' instruction", instruction.target.coord == val)

#define TEST_INSTRUCTION(inst) do {\
    int success = instruction_parse(#inst" 5 4 3", &instruction);\
    MU_ASSERT("Falied to parse the '"#inst"' instruction", success == 0);\
    MU_ASSERT("Incorrect command when parsing the '"#inst"' instruction", instruction.cmd == CMD_ ## inst);\
    TEST_INSTRUCTION_LOCATION(inst,x,5);\
    TEST_INSTRUCTION_LOCATION(inst,y,4);\
    TEST_INSTRUCTION_LOCATION(inst,z,3);\
} while (0)

static char* test_instruction_parsing()
{
    Instruction instruction;

    TEST_INSTRUCTION(ON);
    TEST_INSTRUCTION(OFF);
    TEST_INSTRUCTION(TOGGLE);
    int success = instruction_parse("TICK 5 4 3", &instruction);
    MU_ASSERT("Falied to parse the 'TICK' instruction", success == 0);
    MU_ASSERT("Incorrect command when parsing the 'TICK' instruction", instruction.cmd == CMD_TICK);
    MU_ASSERT("Invalid instruction didn't fail parsing", 
            instruction_parse("BADCMD 0 0 0", &instruction) == -1);
    MU_ASSERT("Instruction with missing parameter didn't fail parsing", 
            instruction_parse("ON 0 0", &instruction) == -1);
    MU_ASSERT("Instruction with non numeric parameter didn't fail parsing", 
            instruction_parse("ON 0 0 a", &instruction) == -1);
    return 0;
}

const Material MATERIALS[4] = { WIRE, CONDUCTOR, INSULATOR, AIR };

static char* test_world_block_creation() {
    World* world = malloc(sizeof(World));
    world_intialize(world, 16);

    CUBE_RANGE(-5,5)
        Location loc = {x,y,z};
        int val = x + y + z;
        Block block = {MATERIALS[val % 4], loc, val % 12};
        world_add_block(world, &block);
        Block* found_block = world_get_block(world, block.location);

        sprintf(message, "Unable to find stored block at (%d, %d, %d)", x, y, z);
        MU_ASSERT(message, found_block != NULL);

        sprintf(message, "Block at (%d, %d, %d) has incorrect power (%d != %d)", x, y, z, found_block->power, block.power);
        MU_ASSERT(message, found_block->power == block.power);

        sprintf(message, "Block at (%d, %d, %d) has incorrect material (%d != %d)", x, y, z, found_block->material, block.material);
        MU_ASSERT(message, found_block->material == block.material);

        sprintf(message, "Block at (%d, %d, %d) has incorrect location (%d, %d, %d)", x, y, z, found_block->location);
        MU_ASSERT(message, LOCATION_EQUALS(found_block->location, block.location));

    CUBE_RANGE_END

    world_free(world);
    free(world);
    return 0;
}

static char * all_tests() {
    MU_RUN_TEST(test_command_parsing);
    MU_RUN_TEST(test_instruction_parsing);
    MU_RUN_TEST(test_world_block_creation);
    return 0;
}

int main(int argc, char* argv[])
{
    message = malloc(sizeof(char) * 200);
    char* result = all_tests();

    printf("\n");
    if (result != 0)
    {
        printf("%s\n", result);
    }
    else
    {
        printf("ALL UNIT TESTS PASSED\n");
    }

    free(message);
    printf("Tests run: %d\n\n", tests_run);
    return result != 0;
}
