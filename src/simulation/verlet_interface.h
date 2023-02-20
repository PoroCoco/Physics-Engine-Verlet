#ifndef _VERLET_INTERFACE_H__
#define _VERLET_INTERFACE_H__

#include <stdlib.h>
#include "circle_verlet.h"

enum constraint_shape {
    CIRCLE,
    SQUARE
};


typedef struct verlet_sim verlet_sim_t;


verlet_sim_t *init_simulation(enum constraint_shape shape, float constraint_center_x, float constraint_center_y, unsigned int constraint_radius, unsigned int width, unsigned int height);

void update_simulation(verlet_sim_t *sim, float dt);

size_t sim_get_current_step(verlet_sim_t *sim);

size_t sim_get_object_count(verlet_sim_t *sim);

enum constraint_shape sim_get_shape(verlet_sim_t *sim);

unsigned int sim_get_constraint_x(verlet_sim_t *sim);

unsigned int sim_get_constraint_y(verlet_sim_t *sim);

unsigned int sim_get_constraint_radius(verlet_sim_t *sim);

unsigned int sim_get_grid_height(verlet_sim_t *sim);

unsigned int sim_get_grid_width(verlet_sim_t *sim);

verlet_circle *sim_get_nth_circle(verlet_sim_t *sim, unsigned int n);


void add_circle(verlet_sim_t *sim, unsigned int radius, float px, float py, color_t color, float acc_x, float acc_y);

void destroy_simulation(verlet_sim_t *s);


color_t rainbow_color(float t);

color_t random_color(void);


#endif // _VERLET_INTERFACE_H__
