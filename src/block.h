#ifndef REDPILE_BLOCK_H
#define REDPILE_BLOCK_H

char* Materials[5];

typedef enum {
    EMPTY,
    AIR,
    WIRE,
    CONDUCTOR,
    INSULATOR
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
