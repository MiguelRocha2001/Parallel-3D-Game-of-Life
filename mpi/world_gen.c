#include <stdlib.h>
#include <stdio.h>

#define N_SPECIES 9

unsigned int seed;

#define min(a, b) ((a) < (b) ? (a) : (b))

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

char ***gen_initial_grid(long long N, float density, int input_seed, int id, int p)
{
    int x, y, z;
    char *** grid;

    int start_row = id * (N/p) + min(id, N % p) - 1;
    if (id == 0)
        start_row = N - 1;

    //int last_row = (id+1) * N/p + min(id+1, N % p);
    
    int number_of_rows = N/p + 2;
    if (id < N % p)
        number_of_rows++;

    /*    
    if (id == p-1)
    {
        printf("ID: %d, Start row: %d; number of rows: %d\n", id, start_row, number_of_rows);
    }
    */

    //printf("ID: %d, Start Row: %d; Rows: %d\n", id, start_row, rows);
    
    grid = (char ***) malloc(number_of_rows * sizeof(char **));
    if(grid == NULL) {
        printf("Failed to allocate matrix\n");
        exit(1);
    }

    for(int x = 0; x < number_of_rows; x++) {
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
                        if (x >= start_row && x < start_row + number_of_rows)
                            grid[x - start_row][y][z] = value;
                    }
                    else if (id == 0)
                    {
                        if (x < number_of_rows - 1) 
                        {
                            grid[x+1][y][z] = value;
                        }

                        else if (x == N-1) 
                        {
                            grid[0][y][z] = value;
                        }
                    }
                    else if (id == p-1)
                    {
                        if (x >= start_row)
                        {
                            grid[x - start_row][y][z] = value;

                            /*
                            if (id == p-1)
                            {
                                if (x == N-1)
                                {
                                    printf("%d ", x - start_row);
                                }
                            }
                            */
                        }

                        else if (x == 0)
                        {
                            grid[number_of_rows-1][y][z] = value;
                        }
                    }
                }

    //printf("Process id: %d, end of gen_initial_grid\n", id);

    return grid;
}
