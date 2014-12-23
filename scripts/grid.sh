#!/bin/bash

COMMANDS="NODE -40..40,0,-40..40 wire\nNODE -20..20%10,0,-20..20%10 torch direction:UP\nTICKQ 2\nFIELD -40..40,0,-40..40 power"
OUTPUT=`echo -e $COMMANDS | ./build/src/redpile conf/redstone.lua | tr ',' ' '`

echo "
set term png
set output \"graph.png\"
set view map
plot '-' using 3:1:4 with image
$OUTPUT
" | gnuplot
