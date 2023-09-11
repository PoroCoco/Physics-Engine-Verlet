#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

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
        }
    }

    // Allocating the matrix for the count of the elements
    g->matrix_count = malloc(sizeof(*g->matrix_count) * g->col_count);
    assert(g->matrix_count);
    for (size_t col = 0; col < g->col_count; col++)
    {
        g->matrix_count[col] = malloc(sizeof(*g->matrix_count[col]) * g->row_count);
        assert(g->matrix_count[col]);
    }

    g->get_element_x = get_element_x;
    g->get_element_y = get_element_y;

    // To fill the matrix with NULL and reset the counts to 0
    spacehash_reset(g);

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
    printf("add element : x_coord %f\n", g->get_element_x(element));
    int col = g->get_element_x(element) / g->cell_width;
    int row = g->get_element_y(element) / g->cell_width;
    if (col >= g->col_count) col = g->col_count - 1; 
    if (col < 0) col = 0; 
    if (row >= g->row_count) row = g->row_count - 1; 
    if (row < 0) row = 0; 
    size_t elements_in_cell = g->matrix_count[col][row];

    if (elements_in_cell == MAX_ELEMENTS_IN_CELL){
        fprintf(stderr, "Spatial hashing : Couldn't fit more elements in the cell when adding a new element ! Increase the maximum amount of element in a cell.\n");
        return;
    }
    printf("Adding element %p at (%d,%d)\n", element, col, row);
    g->matrix[col][row][elements_in_cell] = element;
    g->matrix_count[col][row] += 1;
}


void spacehash_query(sp_grid* g, void *element, void **neighbors, size_t max_neighbors){
    if (max_neighbors == 0) return;
    int col = g->get_element_x(element) / g->cell_width;
    int row = g->get_element_y(element) / g->cell_width;
    if (col >= g->col_count) col = g->col_count - 1; 
    if (col < 0) col = 0; 
    if (row >= g->row_count) row = g->row_count - 1; 
    if (row < 0) row = 0; 

    size_t elements_in_cell = g->matrix_count[col][row];
    if (elements_in_cell > max_neighbors){
        fprintf(stderr, "Spatial hashing : Couldn't fit all the neighbors into the given array when querying (%zu > %zu)! Increase the size of the neighbors array.\n", elements_in_cell, max_neighbors);
    }

    size_t i = 0;
    neighbors[0] = NULL;
    while(i < max_neighbors && g->matrix[col][row][i] != NULL){
        neighbors[i] = g->matrix[col][row][i];
        if (i+1 < max_neighbors) neighbors[i+1] = NULL;
        i++;
    }
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

