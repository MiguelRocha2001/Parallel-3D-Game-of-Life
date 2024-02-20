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
    if
    (
        (is_alive && live_neigbours >= 5 && live_neigbours <= 13)
        ||
        (!is_alive && live_neigbours >= 7 && live_neigbours <= 10) 
    ) 
    { return 1; } // lives
    
    return 0; // dies
}

int get_prev_coord(int coord, int n)
{
    return (coord + n - 1) % n;
}

int get_next_coord(int coord, int n)
{
    return (coord + n + 1) % n;
}

/**
 * @return the number of neighbours and the most common specie among them
*/
void get_neighbours(char ***grid, int n, int x, int y, int z, int* result)
{
    int species[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // idx=0 => specie 1
    int count = 0;

    int prev_x = get_prev_coord(x, n);

    int prev_y = get_prev_coord(y, n);

    int prev_z = get_prev_coord(z, n);

    // iterate over neighbours
    for (int xIdx = prev_x, a = 0; a < 3; a++)
    {
        for (int yIdx = prev_y, b = 0; b < 3; b++)
        {
            for (int zIdx = prev_z, c = 0; c < 3; c++)
            {
                if (!(xIdx == x && yIdx == y && zIdx == z)) // dont count self
                {
                    if (grid[xIdx][yIdx][zIdx]) // neighbour found
                    {
                        species[grid[xIdx][yIdx][zIdx]-1] += 1; 
                        count += 1;
                    }
                }
                if (++zIdx == n) { zIdx = 0; }
            }
            if (++yIdx == n) { yIdx = 0; }
        }
        if (++xIdx == n) { xIdx = 0; }
    }

    result[0] = count; // assigns the number of neighbours

    if (count != 0)
    {
        // checks the most common neighbour
        int idx = 0;
        for (int u = 1, max = -1; u < 9; u++)
        {
            if (species[u] > max) 
            { 
                idx = u;
                max = species[u];
            }
        }
        result[1] = idx + 1; // should be +1 because specie id is idx + 1
    }
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
    int neighbours[2]; // { count, most-common-specie }

    get_neighbours(grid, n, x, y, z, neighbours);

    if (should_live(is_alive, neighbours[0]))
    {
        //printf("%d", neighbours[1]);
        if (!is_alive)
        {
            return neighbours[1];
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

void simulate(char ***grid, int n, int* specie_counter)
{
    int new_cells_state[n][n][n];

    // iterate through all cells
    for (int x = 0; x < n; x++)
    {
        for(int y = 0; y < n; y++)
        {
            for(int z = 0; z < n; z++)
            {
                if (grid[x][y][z]) // if the cell is alive
                {
                    specie_counter[grid[x][y][z] - 1] += 1;
                }
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
    // used to count the number of each specie before new generation
    // idx == 0 means specie 1
    int specie_counter[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
    int specie_counter_aux[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
    int specie_counter_iter[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1}; // olds the iteration 

    for (int cur_gen = 0; cur_gen < nGen; cur_gen++)
    {
        if (debug){ 
            printf("Generation %d --------------\n", cur_gen);
            showCube(grid, n);
        }
        
        simulate(grid, n, specie_counter_aux);

        for (int u = 0; u < 9; u++)
        {
            if (specie_counter_aux[u] > specie_counter[u])
            {
                specie_counter[u] = specie_counter_aux[u];
                specie_counter_iter[u] = cur_gen;
            }
        }
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