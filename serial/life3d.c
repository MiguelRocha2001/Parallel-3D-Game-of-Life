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

int get_prev_x(int x, int n)
{
    return (x + n - 1) % n;
}

int get_next_x(int x, int n)
{
    return (x + n + 1) % n;
}

int get_prev_y(int y, int n)
{
    return (y + n - 1) % n;
}

int get_next_y(int y, int n)
{
    return (y + n + 1) % n;
}

int get_prev_z(int z, int n)
{
    return (z + n - 1) % n;
}

int get_next_z(int z, int n)
{
    return (z + n + 1) % n;
}


// TODO: we consider that the sides wrap-around, i.e., cells with indices i and i + N are the same)
int check_eight(int n, int x, int y, int z)
{
    int count = 0;

    int prev_y = get_prev_y(y, n);
    int next_y = get_next_y(y, n);

    int prev_z = get_prev_z(z, n);
    int next_z = get_next_z(z, n);

    if (grid[x][prev_y][prev_z]) { count += 1; }
    if (grid[x][prev_y][z]) { count += 1; }
    if (grid[x][prev_y][next_z]) { count += 1; }

    if (grid[x][y][prev_z]) { count += 1; }
    if (grid[x][y][z]) { count += 1; }
    if (grid[x][y][next_z]) { count += 1; }

    if (grid[x][next_y][prev_z]) { count += 1; }
    if (grid[x][next_y][z]) { count += 1; }
    if (grid[x][next_y][next_z]) { count += 1; }

    return count;
}

int check_under_nine(int n, int x, int y, int z)
{
    int count = 0;
    
    int prev_x = get_prev_x(x, n);

    int prev_y = get_prev_y(y, n);
    int next_y = get_next_y(y, n);

    int prev_z = get_prev_z(z, n);
    int next_z = get_next_z(z, n);

    if (x == 0)
    if (grid[prev_x][prev_y][prev_z]) { count += 1; }
    if (grid[prev_x][prev_y][z]) { count += 1; }
    if (grid[prev_x][prev_y][next_z]) { count += 1; }

    if (grid[prev_x][y][prev_z]) { count += 1; }
    if (grid[prev_x][y][z]) { count += 1; }
    if (grid[prev_x][y][next_z]) { count += 1; }

    if (grid[prev_x][next_y][prev_z]) { count += 1; }
    if (grid[prev_x][next_y][z]) { count += 1; }
    if (grid[prev_x][next_y][next_z]) { count += 1; }

    return count;
}

/**
 * Iterates and evaluates all grid cells 
*/
int check_upper_nine(int n, int x, int y, int z)
{
    int count = 0;

    int next_x = get_next_x(x, n);

    int prev_y = get_prev_y(y, n);
    int next_y = get_next_y(y, n);

    int prev_z = get_prev_z(z, n);
    int next_z = get_next_z(z, n);

    if (grid[next_x][prev_y][prev_z]) { count += 1; }
    if (grid[next_x][prev_y][z]) { count += 1; }
    if (grid[next_x][prev_y][next_z]) { count += 1; }

    if (grid[next_x][y][prev_z]) { count += 1; }
    if (grid[next_x][y][z]) { count += 1; }
    if (grid[next_x][y][next_z]) { count += 1; }

    if (grid[next_x][next_y][prev_z]) { count += 1; }
    if (grid[next_x][next_y][z]) { count += 1; }
    if (grid[next_x][next_y][next_z]) { count += 1; }

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
    
    //printf("Before check_under_nine function\n");
    count += check_under_nine(n, x, y, z);

    //printf("Before check_eight function\n");
    count += check_eight(n, x, y, z);

    //printf("Before check_upper_nine function\n");
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

void simulate(int n)
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

/**
 * Iterates and evaluates all grid cells 
*/
void simulation(int nGen, int n)
{
    for (int cur_gen = 0; cur_gen < nGen; cur_gen++)
    {
        printf("Generation %d --------------\n", cur_gen);
        showCube(grid, n);
        simulate(n);
    }
    printf("Generation %d --------------\n", nGen);
    showCube(grid, n);
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
    
    exec_time = -omp_get_wtime();
    simulation(nGen, N);

    exec_time += omp_get_wtime();
    fprintf(stderr, "%.1fs\n", exec_time);
    //print_result(); // to the stdout!

    return 0;
}
