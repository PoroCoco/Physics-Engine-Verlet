#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "spatial_hashing.h"

#define MAX_ELEMENTS_IN_CELL 2000

struct spacehash_grid {
    float width;
    float height;
    float cell_width;
    size_t col_count;
    size_t row_count;
    void**** matrix;
    size_t** matrix_count;
    float (*get_element_x)(void *);
    float (*get_element_y)(void *);
};

sp_grid* spacehash_init(float width, float height, float cell_width, float (*get_element_x)(void *), float (*get_element_y)(void *)){
    assert(width > 0.);
    assert(height > 0.);
    assert(cell_width > 0.);
    assert(cell_width <= width);
    assert(cell_width <= height);
    assert(get_element_x != NULL);
    assert(get_element_y != NULL);
    sp_grid * g = malloc(sizeof(*g));
    assert(g);
    g->cell_width = cell_width;
    g->width = width;
    g->height = height;
    //rounded up division
    g->col_count = (width+cell_width-1)/cell_width;
    g->row_count = (height+cell_width-1)/cell_width;

    // Allocating the matrix for the elements
    g->matrix = malloc(sizeof(*g->matrix) * g->col_count);
    assert(g->matrix);
    for (size_t col = 0; col < g->col_count; col++)
    {
        g->matrix[col] = malloc(sizeof(*g->matrix[col]) * g->row_count);
        assert(g->matrix[col]);
        for (size_t row = 0; row < g->row_count; row++)
        {
            g->matrix[col][row] = malloc(sizeof(*g->matrix[col][row]) * MAX_ELEMENTS_IN_CELL);
            assert(g->matrix[col][row]);
            for (size_t element = 0; element < MAX_ELEMENTS_IN_CELL; element++)
            {
                g->matrix[col][row][element] = NULL;
            }
            
        }
    }

    // Allocating the matrix for the count of the elements
    g->matrix_count = malloc(sizeof(*g->matrix_count) * g->col_count);
    assert(g->matrix_count);
    for (size_t col = 0; col < g->col_count; col++)
    {
        g->matrix_count[col] = malloc(sizeof(*g->matrix_count[col]) * g->row_count);
        assert(g->matrix_count[col]);
        for (size_t row = 0; row < g->row_count; row++)
        {
            g->matrix_count[col][row] = 0;
        }        
    }

    g->get_element_x = get_element_x;
    g->get_element_y = get_element_y;

    return g;
}

void spacehash_update_cell_width(sp_grid** g, float new_cell_width){
    sp_grid * new_grid = spacehash_init((*g)->width, (*g)->height, new_cell_width, (*g)->get_element_x, (*g)->get_element_y);
    spacehash_free(*g);
    *g = new_grid;
}

void spacehash_reset(sp_grid* g){
    for(size_t col = 0; col < g->col_count; col++){
        for(size_t row = 0; row < g->row_count; row++){
            size_t i = 0;
            while(i < MAX_ELEMENTS_IN_CELL && g->matrix[col][row][i] != NULL){
                g->matrix[col][row][i] = NULL;
            }
            g->matrix_count[col][row] = 0;
        }
    }
}

void spacehash_add(sp_grid* g, void *element){
    assert(element != NULL);
    int col = g->get_element_x(element) / g->cell_width;
    int row = g->get_element_y(element) / g->cell_width;
    if (col >= (int)g->col_count) col = g->col_count - 1; 
    if (col < 0) col = 0; 
    if (row >= (int)g->row_count) row = g->row_count - 1; 
    if (row < 0) row = 0; 
    size_t elements_in_cell = g->matrix_count[col][row];

    if (elements_in_cell == MAX_ELEMENTS_IN_CELL){
        fprintf(stderr, "Spatial hashing : Couldn't fit more elements in the cell when adding a new element ! Increase the maximum amount of element in a cell.\n");
        return;
    }
    g->matrix[col][row][elements_in_cell] = element;
    g->matrix_count[col][row] += 1;
}


void spacehash_query_neighbors(sp_grid* g, void *element, void **neighbors, size_t max_neighbors){
    if (max_neighbors == 0) return;
    int col = g->get_element_x(element) / g->cell_width;
    int row = g->get_element_y(element) / g->cell_width;
    if (col >= (int)g->col_count) col = g->col_count - 1; 
    if (col < 0) col = 0; 
    if (row >= (int)g->row_count) row = g->row_count - 1; 
    if (row < 0) row = 0; 
    
    size_t neighbor_index = 0;
    neighbors[0] = NULL;
    for (int x = -1; x < 2; x++){
        for (int y = -1; y < 2; y++){
            size_t cell_elements_index = 0;
            int transform_col = col + x;
            int transform_row = row + y;
            if (transform_col >= (int)g->col_count) transform_col = g->col_count - 1; 
            if (transform_col < 0) transform_col = 0; 
            if (transform_row >= (int)g->row_count) transform_row = g->row_count - 1; 
            if (transform_row < 0) transform_row = 0; 
            size_t elements_in_bucket =  g->matrix_count[transform_col][transform_row];
            size_t current_elements_in_neighbors = neighbor_index;

            neighbor_index += elements_in_bucket;

            if (neighbor_index > max_neighbors){
                fprintf(stderr, "Spatial hashing : Couldn't fit all the neighbors into the given array when querying (%zu >= %zu)! Increase the size of the neighbors array.\n", neighbor_index, max_neighbors);
                return;
            }

            memcpy(&neighbors[current_elements_in_neighbors], g->matrix[transform_col][transform_row], sizeof(*neighbors) * elements_in_bucket);

        }
    }

    if (neighbor_index < max_neighbors){
        neighbors[neighbor_index] = NULL;
    }
}


void spacehash_query_bucket(sp_grid* g, size_t row, size_t col, void **elements, size_t max_elements){
    if (max_elements == 0) return;
    assert(row <= g->row_count);
    assert(col <= g->col_count);
    elements[0] = NULL;
    if (row > g->row_count) return;
    if (col > g->col_count) return;

    size_t elements_in_bucket = g->matrix_count[col][row];

    if (elements_in_bucket > max_elements){
        fprintf(stderr, "Spatial hashing : Couldn't fit all the elements into the given array when querying bucket (exceeded %zu)! Increase the size of the elements array.\n", max_elements);
    }

    memcpy(elements, g->matrix[col][row], sizeof(*elements) * elements_in_bucket);

    if (elements_in_bucket != max_elements){
        elements[elements_in_bucket] = NULL;
    }

    // for (size_t i = 0; i < elements_in_bucket; i++)
    // {
    //     if (g->matrix[col][row][i] == NULL) return;
    //     elements[i] = g->matrix[col][row][i];
    //     if (i+1 < max_elements) elements[i+1] = NULL;
    // }
}


size_t spacehash_get_column_count(sp_grid* g){
    return g->col_count;
}

size_t spacehash_get_row_count(sp_grid* g){
    return g->row_count;
}

void spacehash_free(sp_grid* g){
    if (g){
        for (size_t col = 0; col < g->col_count; col++){
            for (size_t row = 0; row < g->row_count; row++){
                if (g->matrix && g->matrix[col] && g->matrix[col][row]) free(g->matrix[col][row]);
            }
            if (g->matrix && g->matrix[col]) free(g->matrix[col]);
            if (g->matrix_count && g->matrix_count[col]) free(g->matrix_count[col]);
        }
        if(g->matrix) free(g->matrix);
        if(g->matrix_count) free(g->matrix_count);
        
        free(g);
    }
}

