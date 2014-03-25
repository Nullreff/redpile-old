#define RANGE(var,start,end) int var; for (var = start; var <= end; var++)
#define CUBE_RANGE(start,end)\
    RANGE(x,start,end) {\
    RANGE(y,start,end) {\
    RANGE(z,start,end) {
#define CUBE_RANGE_END }}}
