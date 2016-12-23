#!/bin/bash

WIRE=100
TORCH=80
COMMANDS="NODE -$WIRE..$WIRE,0,-$WIRE..$WIRE wire\nNODE -$WIRE..$WIRE%10,0,-$WIRE..$WIRE%10 torch direction:UP\nTICKQ 2\nFIELD -40..40,0,-40..40 power"
OUTPUT=`echo -e $COMMANDS | ./build/src/redpile conf/redstone.lua | tr ',' ' ' | sed 's/nil/0/'`

echo "
set term png
set output \"graph.png\"
set view map
plot '-' using 3:1:4 with image
$OUTPUT
" | gnuplot
