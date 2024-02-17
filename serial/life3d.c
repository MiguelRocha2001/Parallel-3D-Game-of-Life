#include <omp.h>
#include <stdio.h>
#include "world_gen.c"

char ***grid;

void showCube(char ***grid, int n) 
{
    for (int x = 0; x < n; x++)
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

/**
 * @return if the cell should live on to the next generation
*/
int should_live(int is_alive, int live_neigbours)
{
    if (is_alive && live_neigbours <= 4) { return 0; }
    if (is_alive && live_neigbours <= 13) { return 1; }
    if (is_alive && live_neigbours >= 13) { return 0; }
    if (!is_alive && live_neigbours >= 7 && live_neigbours <= 10) { return 1; }
}


// TODO: we consider that the sides wrap-around, i.e., cells with indices i and i + N are the same)
int check_eight(int n, int x, int y, int z)
{
    int count = 0;

    if (grid[x][y-1][z-1]) { count += 1; }
    if (grid[x][y-1][z]) { count += 1; }
    if (grid[x][y-1][z+1]) { count += 1; }

    if (grid[x][y][z-1]) { count += 1; }
    if (grid[x][y][z]) { count += 1; }
    if (grid[x][y][z+1]) { count += 1; }

    if (grid[x][y+1][z-1]) { count += 1; }
    if (grid[x][y+1][z]) { count += 1; }
    if (grid[x][y+1][z+1]) { count += 1; }

    return count;
}

int check_under_nine(int n, int x, int y, int z)
{
    int count = 0;

    if (grid[x-1][y-1][z-1]) { count += 1; }
    if (grid[x-1][y-1][z]) { count += 1; }
    if (grid[x-1][y-1][z+1]) { count += 1; }

    if (grid[x-1][y][z-1]) { count += 1; }
    if (grid[x-1][y][z]) { count += 1; }
    if (grid[x-1][y][z+1]) { count += 1; }

    if (grid[x-1][y+1][z-1]) { count += 1; }
    if (grid[x-1][y+1][z]) { count += 1; }
    if (grid[x-1][y+1][z+1]) { count += 1; }

    return count;
}

/**
 * Iterates and evaluates all grid cells 
*/
int check_upper_nine(int n, int x, int y, int z)
{
    int count = 0;

    if (grid[x+1][y-1][z-1]) { count += 1; }
    if (grid[x+1][y-1][z]) { count += 1; }
    if (grid[x+1][y-1][z+1]) { count += 1; }

    if (grid[x+1][y][z-1]) { count += 1; }
    if (grid[x+1][y][z]) { count += 1; }
    if (grid[x+1][y][z+1]) { count += 1; }

    if (grid[x+1][y+1][z-1]) { count += 1; }
    if (grid[x+1][y+1][z]) { count += 1; }
    if (grid[x+1][y+1][z+1]) { count += 1; }

    return count;
}

/**
 * Evaluates a single cell.
 * @return 1 if the cell should live or 0 otherwise
*/
int evaluate_cell(int n, int x, int y, int z)
{
    int count = 0;
    int is_alive = grid[x][y][z];
    
    count += check_under_nine(n, x, y, z);
    count += check_eight(n, x, y, z);
    count += check_upper_nine(n, x, y, z);

    return should_live(is_alive, count);
}

/**
 * Changes grid cells based on @var new_cells_state.
*/
void apply_grid_updates(int n, int new_cells_state[n][n][n])
{
    // iterate through all blocks
    for (int x = 0; x < n; x++)
    {
        for(int y = 0; y < n; y++)
        {
            for(int z = 0; z < n; z++)
            {
                grid[x][y][z] = new_cells_state[x][y][z];
            }
        }
    }
}

/**
 * Iterates and evaluates all grid cells 
*/
void simulation(int n)
{
    int new_cells_state[n][n][n];

    // iterate through all blocks
    for (int x = 0; x < n; x++)
    {
        for(int y = 0; y < n; y++)
        {
            for(int z = 0; z < n; z++)
            {
                new_cells_state[x][y][z] = evaluate_cell(n, x, y, z);
            }
        }
    }

    apply_grid_updates(n, new_cells_state);
}

int main(int argc, char *argv[]) 
{
    int nGen, N, seed;
    float density; 
    
    
    double exec_time;

    nGen = atoi(argv[1]);
    N =  atoi(argv[2]);
    density = atof(argv[3]);
    seed =  atoi(argv[4]);

    grid = gen_initial_grid(N, density, seed);
    
    showCube(grid, N);
    
    exec_time = -omp_get_wtime();
    simulation(N);

    exec_time += omp_get_wtime();
    fprintf(stderr, "%.1fs\n", exec_time);
    //print_result(); // to the stdout!

    return 0;
}
