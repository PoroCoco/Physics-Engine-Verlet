#ifndef _SIMULATION_H__
#define _SIMULATION_H__

#include <stdlib.h>
#include "misc.h"
#include "circle_verlet.h"

typedef struct simulation{
    size_t circle_count;
    size_t allocated_circles;
    verlet_circle *circles;
    uint total_frames;

} simulation;


simulation *init_simulation(void);

void update_simulation(simulation *sim, float dt);
void add_circle(simulation *sim, uint radius, float px, float py, color_t color, float acc_x, float acc_y);

void destroy_simulation(simulation *s);

#endif // _SIMULATION_H__
