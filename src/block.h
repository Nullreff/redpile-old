#ifndef REDPILE_BLOCK_H
#define REDPILE_BLOCK_H

char* Materials[6];

typedef enum {
    M_EMPTY,
    M_AIR,
    M_WIRE,
    M_CONDUCTOR,
    M_INSULATOR,
    M_TORCH
} Material;

typedef enum {
    D_NORTH,
    D_SOUTH,
    D_EAST,
    D_WEST,
    D_UP,
    D_DOWN
} Direction;

typedef struct {
    int x;
    int y;
    int z;
} Location;

typedef struct {
    Material material;
    Location location;
    unsigned int power:4; // 0 - 15
    unsigned int updated:1;
} Block;

Location location_move(Location loc, Direction dir, int length);
int location_equals(Location l1, Location l2);
int location_hash(Location loc, int max);

#endif
