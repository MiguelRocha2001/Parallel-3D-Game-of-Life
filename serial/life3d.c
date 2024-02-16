#include <omp.h>
#include <stdio.h>
#include "world_gen.c"

void showCube(char ***grid, int n){
    for (int x = 0; x < n; x++){
        printf("Layer %d:\n", x);
        for(int y = 0; y < n; y++){
            for(int z = 0; z < n; z++){
                if (grid[x][y][z]) {printf("%d ", grid[x][y][z]);}
                else { printf("- "); }
            }
            printf("\n");
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    int nGen, N, seed;
    float density; 
    char ***grid;
    double exec_time;

    nGen = atoi(argv[1]);
    N =  atoi(argv[2]);
    density = atof(argv[3]);
    seed =  atoi(argv[4]);

    grid = gen_initial_grid(N, density, seed);
    
    showCube(grid, N);
    
    exec_time = -omp_get_wtime();
    //simulation();

    exec_time += omp_get_wtime();
    fprintf(stderr, "%.1fs\n", exec_time);
    //print_result(); // to the stdout!

    return 0;
}
