#ifndef _GRID_H__
#define _GRID_H__

//How many elements are allocated at the creation of the dynamics arrays. 
#define START_ALLOCATED 1024

#include "misc.h"


struct dynamic {
    uint * array;
    size_t count;
    size_t allocated;
};

struct grid {
    struct dynamic **grid;
    uint width;
    uint height;
};

struct grid *create_grid(uint width, uint height);
void add_grid(struct grid *g, uint y, uint x, uint index);
void destroy_grid(struct grid *g);

#endif // _GRID_H__
