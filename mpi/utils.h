
#ifndef UTILS_H
#define UTILS_H

void deleteGrid(char ***grid, int N);
void showCube(char ***grid, int n);
void print_partial_cube(char *** grid, int rows, int n);
int get_prev_coord(int coord, int n);
int get_next_coord(int coord, int n);

/**
 * Changes grid cells based on @var new_cells_state.
*/
void apply_grid_updates(char *** grid, int n, char ***new_cells_state, int number_of_rows);

void update_specie_counter(
    int *specie_counter, 
    int *specie_counter_iter,
    int *specie_counter_aux,
    int n,
    int cur_gen
);

void free_3d_array(int ***array, int n);

void print_result(int **result, int n_species);

#endif