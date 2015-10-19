#!/bin/sh
#PBS -q plgrid-testing
#PBS -l walltime=0:30:00
#PBS -l nodes=2:ppn=12
#PBS -A plgjsawicki2015b

rm -rf /tmp/$USER/AR
mkdir -p /tmp/$USER/AR

module load plgrid/tools/mpich/3.0.4

cd /people/plgjsawicki/AR/lab1
make

cd /tmp/$USER/AR
cp /people/plgjsawicki/AR/lab1/CA ./CA
mkdir -p results

STEPS=500

for PROC in {1..24}; do
    mpiexec -n $PROC ./CA 1000 $STEPS >> results/proc_num

    SIZE=`echo "100 $PROC" | awk '{print int($1*sqrt($2))}'`
    mpiexec -n $PROC ./CA $SIZE $STEPS >> results/scaled
done

cp -r /tmp/$USER/AR/results /people/plgjsawicki/AR/lab1
