#!/bin/bash

for t in 64 32 16 8 4 2 1
do
    echo "Running test 1 with $t processes..."
    bash slurm.sh $t 1000 64 .4 0
    echo "Finished 64_$t.txt"
    
    echo "Running test 2 with $t processes..."
    bash slurm.sh $t 200 128 .5 1000
    echo "Finished 128_$t.txt"
    
    echo "Running test 3 with $t processes..."
    bash slurm.sh $t 10 512 .4 0
    echo "Finished 512_$t.txt"
    
    echo "Running test 4 with $t processes..."
    bash slurm.sh $t 3 1024 .4 100
    echo "Finished 1024_$t.txt"
done