
#include <stdlib.h>
#include <stdio.h>

#include "verlet_interface.h"
#include "config.h"

void spawn_random_circles(verlet_sim_t *sim, size_t count, int height, int width){
    for (size_t i = 0; i < count; i++)
        add_circle(sim, CIRCLE_RADIUS, rand()%width, rand()%height, rainbow_color(sim_get_object_count(sim)), 0, 0, false);
}

void verlet_standard_config(void){
    verlet_sim_t * reference = sim_load_file("../test/test_save.data");

    verlet_sim_t *s = init_simulation(SQUARE, 1920/2, 1080/2, 1080/2, WINDOW_WIDTH, WINDOW_HEIGHT, GRID_WIDTH, GRID_HEIGHT, GRAV_X, GRAV_Y);
    spawn_random_circles(s, 1000, WINDOW_HEIGHT, WINDOW_WIDTH);

    for (size_t i = 0; i < 300; i++)
    {
        update_simulation(s, 1.0/60.0);
    }

    for (size_t i = 0; i < sim_get_object_count(s); i++)
    {
        verlet_circle *c1 = sim_get_nth_circle(s, i);
        verlet_circle *c2 = sim_get_nth_circle(reference, i);

        if (!(c1->position_current.x == c2->position_current.x && c1->position_current.y == c2->position_current.y)){
            fprintf(stderr, "circles position doesn't match\n");
            return;
        }

    }
    
    printf("Correct simulation behaviour\n");

    destroy_simulation(s);
    destroy_simulation(reference);
}


int main(void){
    verlet_standard_config();

    return 0;
}