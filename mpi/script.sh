#!/bin/bash

export OMP_NUM_THREADS=4

for t in 64 32 16 8 4 2 1
do
    echo "Running test 1 with $t processes..."
    mpirun --hostfile hostfile -np $t life3d 1000 64 .4 0 > tests/64_$t.txt
    echo "Finished 64_$t.txt"
    
    echo "Running test 2 with $t processes..."
    mpirun --hostfile hostfile -np $t life3d 200 128 .5 1000 > tests/128_$t.txt
    echo "Finished 128_$t.txt"
    
    echo "Running test 3 with $t processes..."
    mpirun --hostfile hostfile -np $t life3d 10 512 .4 0 > tests/512_$t.txt
    echo "Finished 512_$t.txt"
    
    echo "Running test 4 with $t processes..."
    mpirun --hostfile hostfile -np $t life3d 3 1024 .4 100 > tests/1024_$t.txt
    echo "Finished 1024_$t.txt"
done
