#include <stdlib.h>
#include <stdio.h>

#define N_SPECIES 9

unsigned int seed;

void init_r4uni(int input_seed)
{
    seed = input_seed + 987654321;
}

float r4_uni()
{
    int seed_in = seed;

    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);

    return 0.5 + 0.2328306e-09 * (seed_in + (int) seed);
}

void print_partial_grid(char *** grid, int rows, int n)
{
    for (int x = 0; x < rows; x++)
    {
        printf("Layer %d:\n", x);
        for(int y = 0; y < n; y++)
        {
            for(int z = 0; z < n; z++)
            {
                if (grid[x][y][z]) { printf("%d ", grid[x][y][z]); }
                else { printf("- "); }
            }
            printf("\n");
        }
        printf("\n");
    }
}

char ***gen_initial_grid(long long N, float density, int input_seed, int id, int p)
{
    int x, y, z;
    char *** grid;

    int start_row = id * N/p - 1;
    if (id == 0)
        start_row = N - 1;
    
    int rows = N/p + 2;

    printf("Process id: %d, Start row: %d; rows: %d\n", id, start_row, rows);
    
    grid = (char ***) malloc(rows * sizeof(char **));
    if(grid == NULL) {
        printf("Failed to allocate matrix\n");
        exit(1);
    }

    for(int x = 0; x < rows; x++) {
        grid[x] = (char **) malloc(N * sizeof(char *));
        if(grid[x] == NULL) {
            printf("Failed to allocate matrix\n");
            exit(1);
        }
        grid[x][0] = (char *) calloc(N * N, sizeof(char));
        if(grid[x][0] == NULL) {
            printf("Failed to allocate matrix\n");
            exit(1);
        }

        #pragma omp parallel for
        for (y = 1; y < N; y++)
            grid[x][y] = grid[x][0] + y * N;
    }

    init_r4uni(input_seed);

    for (int x = 0; x < N; x++)
        for (y = 0; y < N; y++)
            for (z = 0; z < N; z++)
                if(r4_uni() < density)
                {
                    int value = (int)(r4_uni() * N_SPECIES) + 1;
                    
                    if (id != 0 && id != p-1)
                    {
                        if (x >= start_row && x < start_row + N/p)
                        grid[x - start_row][y][z] = value;
                        //printf("Value: %d\n", value);
                    }
                    
                    else if (id == 0)
                    {
                        if (x < rows - 1) 
                        {
                            //printf("Value: %d\n", value);
                            grid[x+1][y][z] = value;
                        }

                        else if (x == N-1) 
                        {
                            //printf("Value: %d\n", value);
                            grid[0][y][z] = value;
                        }
                    }
                    else if (id == p-1)
                    {
                        if (x == 0)
                            grid[0][y][z] = value;

                        if (x >= start_row && x < N)
                            grid[x - start_row][y][z] = value;
                    }
                }

    //printf("Process id: %d, end of gen_initial_grid\n", id);
    if (id == 1)
        print_partial_grid(grid, rows, N);

    return grid;
}
