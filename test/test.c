
#include <stdlib.h>
#include <stdio.h>

#include "verlet_interface.h"


#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define CIRCLE_RADIUS 25
#define CONSTRAINT_CENTER_X (WINDOW_WIDTH/2.0)
#define CONSTRAINT_CENTER_Y (WINDOW_HEIGHT/2.0)
#define CONSTRAINT_RADIUS ((WINDOW_HEIGHT/2.0))
#define GRID_WIDTH 1
#define GRID_HEIGHT 1


void verlet_standard_config(void){
    verlet_sim_t * reference = sim_load_file("standardSave.txt");

    verlet_sim_t * s = init_simulation(CIRCLE, CONSTRAINT_CENTER_X, CONSTRAINT_CENTER_Y, CONSTRAINT_RADIUS, WINDOW_WIDTH, WINDOW_HEIGHT, GRID_WIDTH, GRID_HEIGHT);

    int circle_radius = 5;
    int x_spawn = CONSTRAINT_CENTER_X - CONSTRAINT_RADIUS + circle_radius;
    while (x_spawn < CONSTRAINT_CENTER_X + CONSTRAINT_RADIUS - circle_radius)
    {
        add_circle(s, circle_radius, x_spawn, CONSTRAINT_CENTER_Y, rainbow_color(1.0), 0.0, 0.0);        
        x_spawn += (2 * circle_radius);
        circle_radius--;
        if (circle_radius <= 4) circle_radius = 10;
    }
    
    for (size_t i = 0; i < 5; i++)
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
    


    destroy_simulation(s);
}


int main(void){
    verlet_standard_config();

    return 0;
}