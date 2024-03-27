#!/bin/bash

for t in 64 32 16 8
do
    echo "Running test 1 with $t processes..."
    srun -n $t --cpus-per-task=4 life3d 1000 64 .4 0 > slurm/output/$t_$64.txt
    echo "Finished 64_$t.txt"
    
    echo "Running test 2 with $t processes..."
    srun -n $t --cpus-per-task=4 life3d 200 128 .5 1000 > slurm/output/$t_$128.txt
    echo "Finished 128_$t.txt"
    
    echo "Running test 3 with $t processes..."
    srun -n $t --cpus-per-task=4 life3d 10 512 .4 0 > slurm/output/$t_$512.txt
    echo "Finished 512_$t.txt"
    
    echo "Running test 4 with $t processes..."
    srun -n $t --cpus-per-task=4 life3d 3 1024 .4 100 > slurm/output/$t_$1024.txt
    echo "Finished 1024_$t.txt"
done