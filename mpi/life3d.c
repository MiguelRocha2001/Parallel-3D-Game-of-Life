#include <omp.h>
#include <stdio.h>
#include "world_gen.c"
#include "utils.h"
#include <mpi.h>

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

/**
 * The get_neighbours(), I think that the best justification, not to use any paralelism, is because we do small loops (check neighbours and counting neighbours), which only depend on the number of neighbours, which is always the same. Therefore, there isnt any need to create more threads, and have the overhead of doing so. The only thing we are doing is paralelize the call to this function, since it makes sense.
 * @return the number of neighbours and the most common specie among them
*/
void get_neighbours(char ***grid, int n, int x, int y, int z, int* result)
{
    int species[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // idx=0 => specie 1
    int count = 0;

    int prev_x, prev_y, prev_z;
    
    //prev_x = get_prev_coord(x, n);
    prev_x = x - 1; // in mpi implementation, previous plan is in grid[x - 1]
    prev_y = get_prev_coord(y, n);
    prev_z = get_prev_coord(z, n);

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
        int idx = -1;
        for (int u = 0, max = -1; u < 9; u++)
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
 * @return the same specie id if it should not change (is alive and should continue living).
*/
int evaluate_cell(char ***grid, int n, int x, int y, int z)
{
    int is_alive = grid[x][y][z];
    int neighbours[2]; // { count, most-common-specie }

    get_neighbours(grid, n, x, y, z, neighbours);

    if (should_live(is_alive, neighbours[0]))
    {
        if (!is_alive)
        {
            return neighbours[1];
        }
        else // should not change
        {
            //return grid[x][y][z];
            return -1;
        }
    }
    else { return 0; }
}

/**
 * Counts the number of species of the current grid and then simulates
 * a new generation and updates the grid.
*/
void simulate(char ***grid, int n, int number_of_rows, int ***new_cells_state)
{
    // iterate through all cells and reduces the counter array
    //#pragma omp parallel for reduction(+:specie_counter[:N_SPECIES]) //collapse(3) 
    for (int x = 1; x < number_of_rows - 1; x++) // skip first and last row because it is only utilitary
    {
        for(int y = 0; y < n; y++)
        {
            for(int z = 0; z < n; z++)
            {
                // TODO: count species...
                new_cells_state[x][y][z] = evaluate_cell(grid, n, x, y, z);
            }
        }
    }

    apply_grid_updates(grid, n, new_cells_state, number_of_rows);
}

void count_species(char ***grid, int n, int* specie_counter)
{
    // iterate through all cells
    #pragma omp parallel for reduction(+:specie_counter[:N_SPECIES]) //collapse(3) dont use colapse for the same reason as apply_grid_updates() function
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
            }
        }
    }
}

int*** allocate_3d_array(int n) {
    int *** array = (int ***)malloc(n * sizeof(int **));
    if (array == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    #pragma parallel omp for
    for (int i = 0; i < n; i++) 
    {
        (array)[i] = (int **)malloc(n * sizeof(int *));
        if ((array)[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        
        #pragma omp for
        for (int j = 0; j < n; j++) 
        {
            (array)[i][j] = (int *)malloc(n * sizeof(int));
            if ((array)[i][j] == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
    }

    return array;
}


int*** allocate_process_3d_array(int rows, int n)
{
    int *** array = (int ***) malloc(rows * sizeof(int **));
    if(array == NULL) {
        printf("Failed to allocate matrix\n");
        exit(1);
    }

    for(int x = 0; x < rows; x++) {
        array[x] = (int **) malloc(n * sizeof(int *));
        if(array[x] == NULL) {
            printf("Failed to allocate matrix\n");
            exit(1);
        }
        array[x][0] = (int *) calloc(n * n, sizeof(int));
        if(array[x][0] == NULL) {
            printf("Failed to allocate matrix\n");
            exit(1);
        }

        #pragma omp parallel for
        for (int y = 1; y < n; y++)
            array[x][y] = array[x][0] + y * n;
    }

    return array;
}

/**
 * Iterates and evaluates all grid cells 
*/
int **simulation(char *** grid, int nGen, int n, int debug)
{
    int id, p;

    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    /*
    MPI_Request request;

    char *** partial_grid;
    int rows_per_node = n/p;

    if (id == 0) // root node
    {
        for (int i = 0; i < n; i += n/p)
        {
            MPI_Send(grid[n-1], 100, MPI_DOUBLE, 1, n-1, MPI_COMM_WORLD); // send previous
            for (int u = i; u < n/p; u++) // send middle ones
            {
                MPI_Send(grid[u], 100, MPI_DOUBLE, 1, u, MPI_COMM_WORLD);
            }
            MPI_Send(grid[i+(n/p)+1], 100, MPI_DOUBLE, 1, i+(n/p)+1, MPI_COMM_WORLD); // send next
        }
    }

    if (id != 0) // other nodes
    {
        int start_row = id * n/p;

        MPI_Irecv(partial_grid[n-1], 100, MPI_DOUBLE, 1, n-1, MPI_COMM_WORLD, &request); // receive previous
        for (int u = id; u < n/p; u++) // receive middle ones
        {
            MPI_Irecv(partial_grid[u], 100, MPI_DOUBLE, 1, u, MPI_COMM_WORLD, &request);
        }

        MPI_Irecv(partial_grid[id+(n/p)+1], 100, MPI_DOUBLE, 1, id+(n/p)+1, MPI_COMM_WORLD, &request); // receive next
    }
    */

   int number_of_rows = n/p + 2;

    if (id == 0)
        print_partial_cube(grid, number_of_rows, n);

   int ***new_cells_state = allocate_process_3d_array(number_of_rows, n); // TODO: this creates 2 useless rows. Improve later

   simulate(grid, n, number_of_rows, new_cells_state); // this will never modify new_cells_state first and last indexes/rows

    if (id == 0)
        print_partial_cube(grid, number_of_rows, n);

    return 0;
}

int main(int argc, char *argv[]) 
{
    int nGen, N, seed;
    float density;
    char ***grid;
    
    double exec_time;

    // Check if argv has the correct size and if it's in debug mode
    int debug = 0;
    if (argc < 5 || argc > 6)
    {
        printf("Incorrect number of arguments\n");
        return 1;
    }
    // In case of 6 arguments, assumes debug mode
    else if(argc == 6)
    {
        printf("Debug mode enabled\n");
        debug = 1;
    }

    nGen = atoi(argv[1]);
    N =  atoi(argv[2]);
    density = atof(argv[3]);
    seed =  atoi(argv[4]);

    // MPI start
    MPI_Init(&argc, &argv);

    int id, p;

    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    grid = gen_initial_grid(N, density, seed, id, p);

    /*
    int rows = N/p + 2;
    if (id == 3)
        print_partial_cube(grid, rows, N);
    */
    
   /*
    int **result;
    result = (int **) malloc (N_SPECIES * sizeof(int *));
    
    //#pragma omp for
    for (int i = 0; i < N_SPECIES; i++){
        result[i] = (int *) malloc (2 * sizeof(int));
    }
    */
    
    exec_time = -omp_get_wtime();

    simulation(grid, nGen, N, debug);

    exec_time += omp_get_wtime();
    fprintf(stderr, "%.1fs\n", exec_time);
    //deleteGrid(grid, N);

    //print_result(result, N_SPECIES); // to the stdout!

    MPI_Finalize();

    return 0;
}