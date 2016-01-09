#!/bin/env bash

. /people/plgjsawicki/.bash_profile
cd /people/plgjsawicki/AR/lab3
. ve/bin/activate

for PROC in {2..36}; do
    /usr/bin/time -f "3 $PROC %E %U %S" mpiexec -np $PROC python ./par.py ./problem3
    /usr/bin/time -f "4 $PROC %E %U %S" mpiexec -np $PROC python ./par.py ./problem4
done
