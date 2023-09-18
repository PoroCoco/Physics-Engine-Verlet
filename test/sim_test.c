#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "config.h"
#include "verlet_interface.h"


void spawn_random_circles(verlet_sim_t *sim, size_t count, int height, int width){
    for (size_t i = 0; i < count; i++)
        add_circle(sim, CIRCLE_RADIUS, rand()%width, rand()%height, rainbow_color(sim_get_object_count(sim)), 0, 0, false);
}

void spawn_random_sticks(verlet_sim_t *sim, size_t count, int height, int width){
    for (size_t i = 0; i < count; i++){
        verlet_circle *p0 = add_circle(sim, CIRCLE_RADIUS, rand()%width, rand()%height, rainbow_color(sim_get_object_count(sim)), 0, 0, false);
        verlet_circle *p1 = add_circle(sim, CIRCLE_RADIUS, rand()%width, rand()%height, rainbow_color(sim_get_object_count(sim)), 0, 0, false);
        add_stick(sim, p0, p1, 10.0);
    }
}

int main(int argc, char const *argv[])
{
    argc = argc;
    argv = argv;
    verlet_sim_t *sim = init_simulation(SQUARE, 1920/2, 1080/2, 1080/2, WINDOW_WIDTH, WINDOW_HEIGHT, GRAV_X, GRAV_Y);
    spawn_random_circles(sim, 1000, WINDOW_HEIGHT, WINDOW_WIDTH);
    // spawn_random_sticks(sim, 100, WINDOW_HEIGHT, WINDOW_WIDTH);

    for (size_t i = 0; i < 300; i++)
    {
        update_simulation(sim, 1/60.0);
    }
    
    sim_save_current_state(sim, "test_save.data");

    destroy_simulation(sim);

    return 0;
}
