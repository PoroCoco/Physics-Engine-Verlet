#include "simulation.h"
#include "circle_verlet.h"
#include "error_handler.h"

vector gravity = {.x = 0, .y = 1000};


void apply_gravity(simulation *sim){
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        accelerate_circle(sim->circles + i, &gravity);
    }
}

void update_positions(simulation *sim, float dt){
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        update_position_circle(sim->circles + i, dt);
    }
}


void update_simulation(simulation *sim, float dt){
    apply_gravity(sim);
    update_positions(sim, dt);
}

simulation *init_simulation(void){
    simulation *s = malloc(sizeof(simulation));
    _check_malloc(s, __LINE__, __FILE__);
    s->circle_count = 0;
    s->circles = 0;

    return s;
}

void destroy_simulation(simulation *s){
    if (!s) return;
    if (s->circles) free(s->circles);
    free(s);
}