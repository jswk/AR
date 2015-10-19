#!/bin/sh
#PBS -q plgrid-testing
#PBS -l walltime=0:30:00
#PBS -l nodes=1:ppn=12
#PBS -A plgjsawicki2015b

rm -rf /tmp/$USER/AR
mkdir -p /tmp/$USER/AR

module load plgrid/tools/mpich/3.0.4

cd /people/plgjsawicki/AR/lab1
make

cd /tmp/$USER/AR
cp /people/plgjsawicki/AR/lab1/CA ./CA
mkdir -p results

for PROC in {1..12}; do
    mpiexec -n $PROC ./CA 100 10 2&>1 | awk '{print "$PROC "$3}' >> results/proc_num
done

cp -r /tmp/$USER/AR/results /people/plgjsawicki/AR/lab1/results
