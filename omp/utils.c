#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


void deleteGrid(char ***grid, int N)
{
    #pragma omp parallel for
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

int get_prev_coord(int coord, int n)
{
    return (coord + n - 1) % n;
}

int get_next_coord(int coord, int n)
{
    return (coord + n + 1) % n;
}

void apply_grid_updates(char *** grid, int n, int ***new_cells_state)
{
    // iterate through all cells
    #pragma omp parallel for collapse(3)
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

void update_specie_counter(
    int *specie_counter, 
    int *specie_counter_iter,
    int *specie_counter_aux,
    int n_species,
    int cur_gen
)
{
    for (int u = 0; u < n_species; u++)
    {
        if (specie_counter_aux[u] > specie_counter[u])
        {
            specie_counter[u] = specie_counter_aux[u];
            specie_counter_iter[u] = cur_gen;
        }
    }
}

void free_3d_array(int ***array, int n) {
    // Free memory for the third dimension
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            free(array[i][j]);
        }
        free(array[i]);
    }
    // Free memory for the first dimension
    free(array);
}

void print_result(int **result, int n_species)
{
    for (int i = 0; i < n_species; i++){
        printf("%d %d %d\n", i + 1, result[i][0], result[i][1]);
    }
}