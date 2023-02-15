#ifndef _CIRCLE_VERLET_H__
#define _CIRCLE_VERLET_H__

#include "color.h"

typedef struct vector{
    float x;
    float y;
} vector;

typedef struct verlet_circle {
    vector position_current;
    vector position_old;
    vector acceleration;
    
    color_t color;
    unsigned int radius;
} verlet_circle;

void update_position_circle(verlet_circle *c, float dt);
void accelerate_circle(verlet_circle *c, vector *acc);

float vector_length(vector v);
vector vector_create(float x, float y);

#endif // _CIRCLE_VERLET_H__
