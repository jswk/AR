#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 <data file> <output dir>"
    exit 1
fi

mkdir -p $2

STEP_COUNT=`sort -g -r $1 | head -1 | awk '{print $1}'`
GAME_SIZE=$(( `sort -g -r -k3 $1 | head -1 | awk '{print $3}'`+1 ))

cat $1 | grep ^0 | awk '{print $3" "$4" "$5}' | sort -g -k1 -k2 > $2/data_0
STEP=1
while [ $STEP -le $STEP_COUNT ]; do
    STEP_PREV=$(( $STEP-1 ))
    cat $1 | grep "^$STEP_PREV " | awk '{print $3" "$4" "$6}' | sort -g -k1 -k2 > $2/data_$STEP &
    STEP=$(( $STEP+1 ))
done
wait

STEP=0
while [ $STEP -le $STEP_COUNT ]; do
    gnuplot -e "datafile='$2/data_${STEP}'" \
            -e "outputfile='$2/plot_${STEP}.png'" \
            -e "gamesize=$GAME_SIZE" \
            -e "titletext='Step $STEP'" \
            plot_single.plg &
    STEP=$(( $STEP+1 ))
done
wait

