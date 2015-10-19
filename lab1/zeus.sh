#!/bin/sh
#PBS -q plgrid-testing
#PBS -l walltime=0:30:00
#PBS -l nodes=1:ppn=12
#PBS -A plgjsawicki2015b

mkdir -p /tmp/$USER/AR
cd /tmp/$USER/AR

module load plgrid/tools/mpich/3.0.4

cp /people/plgjsawicki/AR/lab1/CA ./CA
mkdir results

for PROC in {1..12}; do
    mpiexec -n $PROC ./CA 100 10 > results/proc_$PROC
done
