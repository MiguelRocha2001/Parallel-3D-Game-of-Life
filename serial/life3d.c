#include <omp.h>
#include <stdio.h>
#include "world_gen.c"

void deleteGrid(char ***grid, int N){
    for (int x = 0; x < N; x++){
        free(grid[x]);
    }
    free(grid);
}

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

/**
 * Iterates and evaluates all grid cells 
*/
int check_upper_nine(char ***grid, int n, int x, int y, int z)
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

int check_eight(char ***grid, int n, int x, int y, int z)
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

int check_under_nine(char ***grid, int n, int x, int y, int z)
{
    int count = 0;
    
    int prev_x = get_prev_x(x, n);

    int prev_y = get_prev_y(y, n);
    int next_y = get_next_y(y, n);

    int prev_z = get_prev_z(z, n);
    int next_z = get_next_z(z, n);

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
int get_neigbours_most_common_specie(char ***grid, int n, int x, int y, int z)
{
    int species[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    int prev_x = get_prev_x(x, n);
    int next_x = get_next_x(x, n);

    int prev_y = get_prev_y(y, n);
    int next_y = get_next_y(y, n);

    int prev_z = get_prev_z(z, n);
    int next_z = get_next_z(z, n);

    // checks upper nine
    if (grid[next_x][prev_y][prev_z]) { species[grid[next_x][prev_y][prev_z]] += 1; }
    if (grid[next_x][prev_y][z]) { species[grid[next_x][prev_y][z]] += 1; }
    if (grid[next_x][prev_y][next_z]) { species[grid[next_x][prev_y][next_z]] += 1; }

    if (grid[next_x][y][prev_z]) { species[grid[next_x][y][prev_z]] += 1; }
    if (grid[next_x][y][z]) { species[grid[next_x][y][z]] += 1; }
    if (grid[next_x][y][next_z]) { species[grid[next_x][y][next_z]] += 1; }

    if (grid[next_x][next_y][prev_z]) { species[grid[next_x][next_y][prev_z]] += 1; }
    if (grid[next_x][next_y][z]) { species[grid[next_x][next_z][z]] += 1; }
    if (grid[next_x][next_y][next_z]) { species[grid[next_x][next_y][next_z]] += 1; }


    // checks middle eight
    if (grid[x][prev_y][prev_z]) { species[grid[x][prev_y][prev_z]] += 1; }
    if (grid[x][prev_y][z]) { species[grid[x][prev_y][z]] += 1; }
    if (grid[x][prev_y][next_z]) { species[grid[x][prev_y][next_z]] += 1; }

    if (grid[x][y][prev_z]) { species[grid[x][y][prev_z]] += 1; }
    if (grid[x][y][z]) { species[grid[x][y][z]] += 1; }
    if (grid[x][y][next_z]) { species[grid[x][y][next_z]] += 1; }

    if (grid[x][next_y][prev_z]) { species[grid[x][next_y][prev_z]] += 1; }
    if (grid[x][next_y][z]) { species[grid[x][next_y][z]] += 1; }
    if (grid[x][next_y][next_z]) { species[grid[x][next_y][next_z]] += 1; }


    // checks under nine
    if (grid[prev_x][prev_y][prev_z]) { species[grid[prev_x][prev_y][prev_z]] += 1; }
    if (grid[prev_x][prev_y][z]) { species[grid[prev_x][prev_y][z]] += 1; }
    if (grid[prev_x][prev_y][next_z]) { species[grid[prev_x][prev_y][next_z]] += 1; }

    if (grid[prev_x][y][prev_z]) { species[grid[prev_x][y][prev_z]] += 1; }
    if (grid[prev_x][y][z]) { species[grid[prev_x][y][z]] += 1; }
    if (grid[prev_x][y][next_z]) { species[grid[prev_x][y][next_z]] += 1; }

    if (grid[prev_x][next_y][prev_z]) { species[grid[prev_x][next_y][prev_z]] += 1; }
    if (grid[prev_x][next_y][z]) { species[grid[prev_x][next_y][z]] += 1; }
    if (grid[prev_x][next_y][next_z]) { species[grid[prev_x][next_y][next_z]] += 1; }


    int max = -1, idx = -1;
    for (int u = 0; u < 9; u++)
    {
        if (species[u] > max) 
        { 
            idx = u;
            max = species[u]; 
        }
    } 

    //printf("idx: %d\n", idx);

    return idx;
}

int get_total_neighbours(char ***grid, int n, int x, int y, int z)
{
    int count = 0;

    //printf("Before check_upper_nine function\n");
    count += check_upper_nine(grid, n, x, y, z);
    
    //printf("Before check_under_nine function\n");
    count += check_under_nine(grid, n, x, y, z);

    //printf("Before check_eight function\n");
    count += check_eight(grid, n, x, y, z);

    return count;
}

/**
 * Evaluates a single cell.
 * @return the specie id if it should live and the cell is dead;
 * @return 0 if it should die;
 * @return -1 if it should not change (is alive and should continue living).
*/
int evaluate_cell(char ***grid, int n, int x, int y, int z)
{
    int is_alive = grid[x][y][z];
    
    int count = get_total_neighbours(grid, n, x, y, z);

    if (should_live(is_alive, count))
    {
        if (!is_alive)
        {
            // determine the specie id
            return get_neigbours_most_common_specie(grid, n, x, y, z);
        }
        else
        {
            return -1;
        }
    }
    else { return 0; }
}

/**
 * Changes grid cells based on @var new_cells_state.
*/
void apply_grid_updates(char *** grid, int n, int new_cells_state[n][n][n])
{
    // iterate through all cells
    for (int x = 0; x < n; x++)
    {
        for(int y = 0; y < n; y++)
        {
            for(int z = 0; z < n; z++)
            {
                if (new_cells_state[x][y][z] != -1) // -1 means no change
                {
                    grid[x][y][z] = new_cells_state[x][y][z];
                }
            }
        }
    }
}

void simulate(char ***grid, int n)
{
    int new_cells_state[n][n][n];

    // iterate through all blocks
    for (int x = 0; x < n; x++)
    {
        for(int y = 0; y < n; y++)
        {
            for(int z = 0; z < n; z++)
            {
                new_cells_state[x][y][z] = evaluate_cell(grid, n, x, y, z);
            }
        }
    }

    apply_grid_updates(grid, n, new_cells_state);
}

/**
 * Iterates and evaluates all grid cells 
*/
void simulation(char *** grid, int nGen, int n, int debug)
{
    for (int cur_gen = 0; cur_gen < nGen; cur_gen++)
    {
        if (debug){ 
            printf("Generation %d --------------\n", cur_gen);
            showCube(grid, n);
        }
        simulate(grid, n);
    }
    if (debug){
        printf("Generation %d --------------\n", nGen);
        showCube(grid, n);
    }
}

int main(int argc, char *argv[]) 
{
    int nGen, N, seed;
    float density;
    char ***grid;
    
    double exec_time;

    // Check if argv has the correct size and if it's in debug mode
    int debug = 0;
    if (argc < 5 || argc > 6){
        printf("Incorrect number of arguments\n");
        return 1;
    }
    // In case of 6 arguments, assumes debug mode
    else if(argc == 6){
        printf("Debug mode enabled\n");
        debug = 1;
    }

    nGen = atoi(argv[1]);
    N =  atoi(argv[2]);
    density = atof(argv[3]);
    seed =  atoi(argv[4]);

    grid = gen_initial_grid(N, density, seed);
    
    exec_time = -omp_get_wtime();

    simulation(grid, nGen, N, debug);

    exec_time += omp_get_wtime();
    fprintf(stderr, "%.1fs\n", exec_time);
    deleteGrid(grid, N);

    //print_result(); // to the stdout!

    return 0;
}
