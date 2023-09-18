#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "config.h"
#include "verlet_interface.h"


void spawn_random_circles(verlet_sim_t *sim, size_t count, int height, int width){
    int left_padding = sim_get_constraint_x(sim) - sim_get_constraint_radius(sim);
    for (size_t i = 0; i < count; i++)
        add_circle(sim, 2, left_padding + rand()%width, rand()%height, rainbow_color(sim_get_object_count(sim)), 0, 0, false);
}

int main(int argc, char const *argv[])
{
    argc = argc;
    argv = argv;
    verlet_sim_t *sim = init_simulation(SQUARE, WINDOW_WIDTH/2, WINDOW_HEIGHT/2, WINDOW_HEIGHT/2, WINDOW_WIDTH, WINDOW_HEIGHT, GRAV_X, GRAV_Y);
    spawn_random_circles(sim, 30000, WINDOW_HEIGHT, WINDOW_WIDTH);

    for (size_t i = 0; i < 30; i++)
    {
        update_simulation(sim, 1/60.0);
    }
    
    destroy_simulation(sim);

    return 0;
}
