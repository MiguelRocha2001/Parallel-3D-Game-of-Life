#!/bin/bash

for t in 8 4 2 1
do
    export OMP_NUM_THREADS=$t
    ./life3d-omp 1000 64 0.4 0 > tests/64_$t.txt
    ./life3d-omp 200 128 .5 1000 > tests/128_$t.txt
    ./life3d-omp 10 512 .4 0 > tests/512_$t.txt
    ./life3d-omp 3 1024 .4 100 > tests/1024_$t.txt
done
