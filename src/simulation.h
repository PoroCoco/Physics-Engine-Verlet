#ifndef _SIMULATION_H__
#define _SIMULATION_H__

#include <stdlib.h>
#include "misc.h"
#include "circle_verlet.h"

typedef struct simulation{
    size_t circle_count;
    verlet_circle *circles;

} simulation;

simulation *init_simulation(void);
void destroy_simulation(simulation *s);

#endif // _SIMULATION_H__
