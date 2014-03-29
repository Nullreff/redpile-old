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

typedef struct {
    int x;
    int y;
    int z;
} Location;

typedef struct {
    Material material;
    Location location;
    int power;
} Block;

#define LOCATION_EQUALS(l1,l2) ((l1).x == (l2).x && (l1).y == (l2).y && (l1).z == (l2).z)
#define LOCATION_HASH(loc, max) abs((loc).x ^ (loc).y ^ (loc).z) % max

#endif
