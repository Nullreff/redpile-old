#include <stdlib.h>
#include "block.h"

char* Materials[6] = {"EMPTY", "AIR", "WIRE", "CONDUCTOR", "INSULATOR", "TORCH"};

Location location_move(Location loc, Direction dir, int length)
{
    switch (dir)
    {
        case D_NORTH: return (Location){loc.x, loc.y, loc.z - length};
        case D_SOUTH: return (Location){loc.x, loc.y, loc.z + length};
        case D_EAST: return (Location){loc.x + length, loc.y, loc.z};
        case D_WEST: return (Location){loc.x - length, loc.y, loc.z};
        case D_UP: return (Location){loc.x, loc.y + length, loc.z};
        case D_DOWN: return (Location){loc.x, loc.y - length, loc.z};
    }
}

int location_equals(Location l1, Location l2)
{
    return l1.x == l2.x && l1.y == l2.y && l1.z == l2.z;
}

int location_hash(Location loc, int max)
{
    return abs(loc.x ^ loc.y ^ loc.z) % max;
}
