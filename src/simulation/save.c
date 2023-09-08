#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "verlet_interface.h"
#include "error_handler.h"

void sim_save_current_state(verlet_sim_t *sim, char * file_path){
    assert(file_path);
    FILE *file_out = fopen(file_path, "w");
    _check_file(file_out, __LINE__, __FILE__);

    //nb_circles, width, height, steps, grid_width, grid_height, constraint_x, constraint_y, constraint_radius, shape
    fprintf(file_out, "%zu, %u, %u, %zu, %u, %u, %u, %u, %u, %d\n", sim_get_object_count(sim), sim_get_width(sim), sim_get_height(sim), sim_get_current_step(sim), sim_get_grid_width(sim),
                                                                    sim_get_grid_height(sim), sim_get_constraint_x(sim), sim_get_constraint_y(sim), sim_get_constraint_radius(sim), sim_get_shape(sim));

    for (size_t i = 0; i < sim_get_object_count(sim); i++)
    {
        verlet_circle *c = sim_get_nth_circle(sim, i);
        fprintf(file_out, "%f,%f,%f,%f,%u\n", c->position_current.x, c->position_current.y, c->acceleration.x, c->acceleration.y, c->radius);
    }
    
    fclose(file_out);
}

verlet_sim_t * sim_load_file(char * file_path){
    assert(file_path);
    FILE *save_file = fopen(file_path, "r");
    _check_file(save_file, __LINE__, __FILE__);

    size_t circle_count, steps;
    unsigned int width, height, grid_width, grid_height, constraint_x, constraint_y, constraint_radius;
    int shape;
    int matches = fscanf(save_file, "%zu, %u, %u, %zu, %u, %u, %u, %u, %u, %d\n", &circle_count, &width, &height, &steps, &grid_width, &grid_height, &constraint_x, &constraint_y, &constraint_radius, &shape);
    if (matches != 10){
        fprintf(stderr, "Couldn't match the config pattern inside the file \"%s\" to load the simulation\n", file_path);
        exit(1);
    }
    enum constraint_shape constraint_shape = (enum constraint_shape)shape;
    verlet_sim_t * s = init_simulation(constraint_shape, constraint_x, constraint_y, constraint_radius, width, height, grid_width, grid_height, 0, 1000);
    assert(s);

    float circle_x, circle_y, circle_acc_x, circle_acc_y;
    unsigned int circle_radius;
    for (size_t i = 0; i < circle_count; i++)
    {
        matches = fscanf(save_file, "%f,%f,%f,%f,%u\n", &circle_x, &circle_y, &circle_acc_x, &circle_acc_y, &circle_radius);
        if (matches != 5){
            fprintf(stderr, "Couldn't match the circles pattern inside the file \"%s\" to load the simulation\n", file_path);
            exit(1);
        }
        add_circle(s, circle_radius, circle_x, circle_y, rainbow_color(1.0), circle_acc_x, circle_acc_y, false);
    }
    

    fclose(save_file);
    return s;
}
