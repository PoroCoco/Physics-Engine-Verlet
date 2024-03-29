#ifndef _CIRCLE_VERLET_H__
#define _CIRCLE_VERLET_H__

#include <stdbool.h>

typedef struct color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} color_t;

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
    bool pinned;
} verlet_circle;

void update_position_circle(verlet_circle *c, float dt);
void accelerate_circle(verlet_circle *c, vector *acc);
vector circle_get_position(verlet_circle *c);
float circle_get_position_x(verlet_circle *c);
float circle_get_position_y(verlet_circle *c);

float vector_length(vector v);
vector vector_create(float x, float y);

#endif // _CIRCLE_VERLET_H__
