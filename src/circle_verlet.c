#include "circle_verlet.h"


/// @brief 
/// @param c 
void update_position_circle(verlet_circle *c, float dt){
    vector velocity;
    velocity.x = c->position_current.x - c->position_old.x;
    velocity.y = c->position_current.y - c->position_old.y;

    //Saves current
    c->position_old.x = c->position_current.x;
    c->position_old.y = c->position_current.y;

    //Verlet integration
    c->position_current.x = c->position_current.x + velocity.x + c->acceleration.x * dt * dt;

    //Reset Acceleration
    c->acceleration.x = 0;
    c->acceleration.y = 0;
}


void accelerate_circle(verlet_circle *c, vector *acc){
    c->acceleration.x += acc->x;
    c->acceleration.y += acc->y;
}
