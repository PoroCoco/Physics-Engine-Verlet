#ifndef _SIM_H__
#define _SIM_H__


#include "verlet_interface.h"
#include "circle_verlet.h"
#include "error_handler.h"
#include "misc.h"
#include "circle_verlet.h"
#include "sticks.h"
#include "spatial_hashing.h"

typedef struct verlet_sim {
    size_t circle_count;
    verlet_circle circles[SIM_MAX_CIRCLES];

    size_t stick_count;
    stick sticks[SIM_MAX_STICKS];

    sp_grid* space_grid;
    uint biggest_circle_radius;
    vector constraint_center;
    uint constraint_radius;

    uint total_frames;
    uint sub_steps;
    uint thread_count;
    enum constraint_shape constraint_shape;

    uint width;
    uint height;
    vector gravity;

} verlet_sim_t;


void solve_circle_collision(verlet_circle *c1, verlet_circle *c2);


#endif // _SIM_H__
