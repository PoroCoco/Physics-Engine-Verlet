#ifndef _VERLET_INTERFACE_H__
#define _VERLET_INTERFACE_H__

#include <stdlib.h>
#include "circle_verlet.h"
#include "sticks.h"

enum constraint_shape {
    CIRCLE,
    SQUARE
};


typedef struct verlet_sim verlet_sim_t;


verlet_sim_t *init_simulation(enum constraint_shape shape, float constraint_center_x, float constraint_center_y, unsigned int constraint_radius, unsigned int width, unsigned int height, unsigned int grid_width, unsigned int grid_height, int grav_x, int grav_y);

void update_simulation(verlet_sim_t *sim, float dt);

size_t sim_get_current_step(verlet_sim_t *sim);

size_t sim_get_object_count(verlet_sim_t *sim);

enum constraint_shape sim_get_shape(verlet_sim_t *sim);

unsigned int sim_get_constraint_x(verlet_sim_t *sim);

unsigned int sim_get_constraint_y(verlet_sim_t *sim);

unsigned int sim_get_constraint_radius(verlet_sim_t *sim);

void sim_set_constraint_radius(verlet_sim_t *sim, int radius);

void sim_set_sub_steps(verlet_sim_t *sim, unsigned int sub_steps);

void sim_set_thread_count(verlet_sim_t *sim, unsigned int thread_count);

unsigned int sim_get_grid_height(verlet_sim_t *sim);

unsigned int sim_get_grid_width(verlet_sim_t *sim);

unsigned int sim_get_width(verlet_sim_t *sim);

unsigned int sim_get_height(verlet_sim_t *sim);

vector sim_get_gravity(verlet_sim_t *sim);

void sim_set_gravity(verlet_sim_t *sim, vector gravity);

size_t sim_get_stick_count(verlet_sim_t *sim);

verlet_circle *sim_get_nth_circle(verlet_sim_t *sim, unsigned int n);

stick * add_stick(verlet_sim_t *sim, verlet_circle *p0, verlet_circle *p1, float len);

stick * sim_get_nth_stick(verlet_sim_t *sim, unsigned int n);

verlet_circle * sim_get_circle_at_coord(verlet_sim_t *sim, float x, float y);


verlet_circle * add_circle(verlet_sim_t *sim, unsigned int radius, float px, float py, color_t color, float acc_x, float acc_y, bool pinned);

void destroy_simulation(verlet_sim_t *s);


color_t rainbow_color(float t);

color_t random_color(void);

void sim_save_current_state(verlet_sim_t *sim, char* filename);

verlet_sim_t * sim_load_file(char * file_path);

#endif // _VERLET_INTERFACE_H__
