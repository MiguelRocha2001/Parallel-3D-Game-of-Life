#include <omp.h>
#include <stdio.h>
#include "world_gen.c"
#include "utils.h"
#include <string.h>
#include <mpi.h>

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


char*** allocate_process_3d_array(int rows, int n)
{
    char *** array = (char ***) malloc(rows * sizeof(char **));
    if(array == NULL) {
        printf("Failed to allocate matrix\n");
        exit(1);
    }

    for(char x = 0; x < rows; x++) {
        array[x] = (char **) malloc(n * sizeof(char *));
        if(array[x] == NULL) {
            printf("Failed to allocate matrix\n");
            exit(1);
        }
        array[x][0] = (char *) calloc(n * n, sizeof(char));
        if(array[x][0] == NULL) {
            printf("Failed to allocate matrix\n");
            exit(1);
        }

        #pragma omp parallel for
        for (char y = 1; y < n; y++)
            array[x][y] = array[x][0] + y * n;
    }

    return array;
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
            return grid[x][y][z];
            //return -1;
        }
    }
    else { return 0; }
}

/**
 * Computes first row and sends assyncronously it's result.
*/
void update_and_send_first_row(char ***grid, int n, int number_of_rows, char ***new_cells_state, int* specie_counter, int id, int p)
{
    int previous_process_id;
    
    if (id == 0)
        previous_process_id = p - 1;
    else
        previous_process_id = id - 1;
    
    MPI_Request request;

    int sender_tag_1 = previous_process_id + 900;

    int x = 1;
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
    MPI_Isend(new_cells_state[1][0], n*n, MPI_CHAR, previous_process_id, sender_tag_1, MPI_COMM_WORLD, &request); // send first updated row to previous process
}

/**
 * Computes last row and sends assyncronously it's result.
*/
void update_and_send_last_row(char ***grid, int n, int number_of_rows, char ***new_cells_state, int* specie_counter, int id, int p, int count_species) 
{
    int next_process_id;
    if (id == p - 1)
        next_process_id = 0;
    else
        next_process_id = id + 1;

    MPI_Request request;

    int sender_tag_2 = next_process_id + 500;

    int x = number_of_rows - 2;
    for(int y = 0; y < n; y++)
    {
        for(int z = 0; z < n; z++)
        {
            if (count_species &&  grid[x][y][z])
            {
                specie_counter[grid[x][y][z] - 1] += 1;
            }
            new_cells_state[x][y][z] = evaluate_cell(grid, n, x, y, z);
        }
    }
    MPI_Isend(new_cells_state[number_of_rows-2][0], n*n, MPI_CHAR, next_process_id, sender_tag_2, MPI_COMM_WORLD, &request); // send last updated row to next process
}

/**
 * Computes last row and sends assyncronously it's result.
*/
void update_middle_rows(char ***grid, int n, int number_of_rows, char ***new_cells_state, int* specie_counter) 
{    
    // iterate through all cells and reduces the counter array
    #pragma omp parallel for reduction(+:specie_counter[:N_SPECIES])
    for (int x = 2; x < number_of_rows - 2; x++) // skip first and last row because it is only utilitary
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
}

void set_row_receivers(char *first_row, char *last_row, MPI_Request *requests, int id, int p, int n, int number_of_rows)
{
    int previous_process_id, next_process_id;
    
    if (id == 0)
        previous_process_id = p - 1;
    else
        previous_process_id = id - 1;
    
    if (id == p - 1)
        next_process_id = 0;
    else
        next_process_id = id + 1;

    int receiver_tag_1 = id + 900; // 900 was just a random number so receiver_tag_1 doesnt colide with receiver_tag_2
    int receiver_tag_2 = id + 500;

    MPI_Irecv(last_row, n*n, MPI_CHAR, next_process_id, receiver_tag_1, MPI_COMM_WORLD, &requests[0]); // receive last utilitary row that was updated by next process
    MPI_Irecv(first_row, n*n, MPI_CHAR, previous_process_id, receiver_tag_2, MPI_COMM_WORLD, &requests[1]); // receive first utilitary row that was updated by previous process
}

void apply_received_updates(char ***grid, char *first_row, char *last_row, int n, int number_of_rows)
{
    memcpy(grid[0][0], first_row, n*n);
    memcpy(grid[number_of_rows-1][0], last_row, n*n);
}

/**
 * Counts the number of species of the current grid while simulating a new generation.
 * It, also, sends the two new updated rows to the neighbour processes
*/
void count_species_and_simulate(char ***grid, int n, int number_of_rows, char ***new_cells_state, int* specie_counter, int id, int p)
{
    char first_row[n * n * sizeof(char)];
    char last_row[n * n * sizeof(char)];

    MPI_Request requests[2];
    MPI_Status statuses[2];

    int number_of_rows_to_update = number_of_rows - 2;

    set_row_receivers(first_row, last_row, requests, id, p, n, number_of_rows);

    update_and_send_first_row(grid, n, number_of_rows, new_cells_state, specie_counter, id, p);    
    
    int count_species = number_of_rows_to_update > 1;
    update_and_send_last_row(grid, n, number_of_rows, new_cells_state, specie_counter, id, p, count_species);
    
    if (number_of_rows_to_update > 2)
        update_middle_rows(grid, n, number_of_rows, new_cells_state, specie_counter);

    apply_grid_updates(grid, n, new_cells_state, number_of_rows);

    MPI_Waitall(2, requests, statuses);

    apply_received_updates(grid, first_row, last_row, n, number_of_rows);
}

void count_species(char ***grid, int n, int number_of_rows, int* specie_counter)
{
    // iterate through all cells
    #pragma omp parallel for reduction(+:specie_counter[:N_SPECIES])
    for (int x = 1; x < number_of_rows - 1; x++) // skip first and last row because it is only utilitary
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

void update_specie_counter_aux(int* intance_specie_counter, int* total_specie_counter, int* total_specie_counter_iter, int id, int p, int cur_gen)
{
    int intance_specie_counter_reduce[N_SPECIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    MPI_Reduce(intance_specie_counter, intance_specie_counter_reduce, N_SPECIES, MPI_INT,
            MPI_SUM, p-1, MPI_COMM_WORLD);

    // Deals with specie counter colected through reduce operation
    if (id == p-1)
    {
        update_specie_counter(total_specie_counter, total_specie_counter_iter, intance_specie_counter_reduce, N_SPECIES, cur_gen);
    }
}

/**
 * Iterates and evaluates all grid cells 
*/
int **simulation(char *** grid, int nGen, int n, int debug)
{
    int id, p;

    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    int number_of_rows = n/p + 2;
    if (id < n % p)
        number_of_rows++;

    char ***new_cells_state = allocate_process_3d_array(number_of_rows, n); // TODO: this creates 2 useless rows. Improve later

    // TODO: what if there is only one process! Deal with that later

    int total_specie_counter[N_SPECIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    int total_specie_counter_iter[N_SPECIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // Holds the iteration

    for (int cur_gen = 0; cur_gen < nGen; cur_gen++)
    {
        int intance_specie_counter[N_SPECIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

        count_species_and_simulate(grid, n, number_of_rows, new_cells_state, intance_specie_counter, id, p); // this will never modify new_cells_state first and last indexes/rows
        update_specie_counter_aux(intance_specie_counter, total_specie_counter, total_specie_counter_iter, id, p, cur_gen);
    }

    if (id == 0)
        print_partial_cube(grid, number_of_rows, n);

    int intance_specie_counter[N_SPECIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    count_species(grid, n, number_of_rows, intance_specie_counter);
    
    update_specie_counter_aux(intance_specie_counter, total_specie_counter, total_specie_counter_iter, id, p, nGen);

    // root node prints specie counter final result
    if (id == p-1)
    {
        int **result;
        result = (int **) malloc (N_SPECIES * sizeof(int *));
        for (int i = 0; i < N_SPECIES; i++) 
        {
            result[i] = (int *) malloc (2 * sizeof(int));
            result[i][0] = total_specie_counter[i];
            result[i][1] = total_specie_counter_iter[i];     
        }

        print_result(result, N_SPECIES); // to the stdout!
    }

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
    
    exec_time = -omp_get_wtime();
    //double elapsed_time = -MPI_Wtime();

    simulation(grid, nGen, N, debug);
    
    //elapsed_time += MPI_Wtime();
    //printf("Total elapsed time: %10.6f\n", elapsed_time);

    exec_time += omp_get_wtime();
    fprintf(stderr, "%.1fs\n", exec_time);
    //deleteGrid(grid, N);

    //print_result(result, N_SPECIES); // to the stdout!

    MPI_Finalize();

    return 0;
}