#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ $# -lt 2 ]; then
    echo "Usage: $0 <procs> <chunk size>"
    exit 1
fi

PROC=$1
CHUNK=$2

for (( IND=0; IND<$PROC; IND++ )); do
    SHIFT=$(($IND*$CHUNK))
    gensort -b$SHIFT $CHUNK "in.$IND"
done

/usr/bin/time mpiexec -np $PROC $DIR/merge_file $CHUNK

for (( IND=0; IND<$PROC; IND++ )); do
    valsort -o "out.${IND}.sum" "out.$IND" 2> /dev/null
done
cat out.*.sum > out.sum
valsort -s out.sum
