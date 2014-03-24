#ifndef REDPILE_COMMON_H
#define REDPILE_COMMON_H

typedef struct {
    long x;
    long y;
    long z;
} Location;

#define LOCATION_EQUALS(l1,l2) (l1.x == l2.x && l1.y == l2.y && l1.z == l2.z)

#endif
