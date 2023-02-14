#ifndef _CIRCLE_VERLET_H__
#define _CIRCLE_VERLET_H__


typedef struct vector{
    float x;
    float y;
} vector;

typedef struct verlet_circle {
    vector position_current;
    vector position_old;
    vector acceleration;

    unsigned int radius;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} verlet_circle;

void update_position_circle(verlet_circle *c, float dt);
void accelerate_circle(verlet_circle *c, vector *acc);

#endif // _CIRCLE_VERLET_H__
