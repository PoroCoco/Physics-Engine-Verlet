#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "verlet_interface.h"
#include "error_handler.h"

void sim_save_current_state(verlet_sim_t *sim, char * file_path){
    assert(file_path);
    FILE *file_out = fopen(file_path, "w");
    _check_file(file_out, __LINE__, __FILE__);

    //nb_circles, width, height, steps
    fprintf(file_out, "%zu, %u, %u, %zu\n", sim_get_object_count(sim), sim_get_width(sim), sim_get_height(sim), sim_get_current_step(sim));

    for (size_t i = 0; i < sim_get_object_count(sim); i++)
    {
        verlet_circle *c = sim_get_nth_circle(sim, i);
        fprintf(file_out, "%f,%f,%f,%f,%u\n", c->position_current.x, c->position_current.y, c->acceleration.x, c->acceleration.y, c->radius);
    }
    
    fclose(file_out);
}
