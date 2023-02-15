#include <stdlib.h>
#include <stdio.h>
#include "grid.h"
#include "error_handler.h"

struct grid *create_grid(uint width, uint height){
    struct grid *g = malloc(sizeof(struct grid));
    _check_malloc(g, __LINE__, __FILE__);

    g->grid = malloc(height * sizeof(void*));
    _check_malloc(g->grid, __LINE__, __FILE__);
    for (uint i = 0; i < height; i++){
        g->grid[i] = malloc(width * sizeof(struct dynamic));
        _check_malloc(g->grid[i], __LINE__, __FILE__);
        for (size_t j = 0; j < width; j++)
        {
            g->grid[i][j].array = malloc(START_ALLOCATED * sizeof(uint));
            _check_malloc(g->grid[i][j].array, __LINE__, __FILE__);
            g->grid[i][j].allocated = START_ALLOCATED;
            g->grid[i][j].count = 0;
        }
    }
    g->height = height;
    g->width = width;

    return g;
}

void add_grid(struct grid *g, uint y, uint x, uint index){
    if (y > g->height || x > g->width) fprintf(stderr, "WARNING : Tried to add into a grid cell at impossible position x,y %zu,%zu\n", x, y);  
    g->grid[y][x].array[g->grid[y][x].count] = index;

    g->grid[y][x].count++;

    if(g->grid[y][x].count == g->grid[y][x].allocated){
        g->grid[y][x].array = realloc(g->grid[y][x].array, (sizeof(uint) * g->grid[y][x].allocated * 2));
        _check_malloc(g->grid[y][x].array, __LINE__, __FILE__);

        g->grid[y][x].allocated *= 2;
    }
}


void destroy_grid(struct grid *g){
    if (!g) return;
    if (g->grid) free(g->grid);
    free(g);
}