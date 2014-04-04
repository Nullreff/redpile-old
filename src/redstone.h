#ifndef REDPILE_REDSTONE_H
#define REDPILE_REDSTONE_H

void redstone_noop_update(World* world, Block* block);
void redstone_wire_update(World* world, Block* block);
void redstone_conductor_update(World* world, Block* block);
void redstone_torch_update(World* world, Block* block);
void redstone_tick(World* world, void (*block_modified_callback)(Block*));

#endif
