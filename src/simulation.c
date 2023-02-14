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

void apply_constraint(simulation *sim){
    vector position = {.x = WINDOW_WIDTH/2.0, .y = WINDOW_HEIGHT/2.0};
    float radius =  (WINDOW_HEIGHT-100)/2.0;

    for (size_t i = 0; i < sim->circle_count; i++)
    {
        verlet_circle *c = sim->circles + i;
        vector to_circle;
        to_circle.x = c->position_current.x - position.x;
        to_circle.y = c->position_current.y - position.y;
        float dist = vector_length(to_circle);

        if (dist > radius - (float)c->radius){
            vector n = {
                .x = to_circle.x / dist,
                .y = to_circle.y / dist
            };
            c->position_current.x = position.x + n.x * (radius - (float)(c->radius));
            c->position_current.y = position.y + n.y * (radius - (float)(c->radius));
        }
        

    }
    

}


simulation *init_simulation(void){
    simulation *s = malloc(sizeof(simulation));
    _check_malloc(s, __LINE__, __FILE__);
    s->circle_count = 0;
    s->circles = malloc(sizeof(verlet_circle));
    _check_malloc(s->circles, __LINE__, __FILE__);
    s->allocated_circles = 1;

    return s;
}

void destroy_simulation(simulation *s){
    if (!s) return;
    if (s->circles) free(s->circles);
    free(s);
}

void solve_collisions(simulation *sim){
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        verlet_circle *c1 = sim->circles + i;
        for (size_t j = i+1; j < sim->circle_count; j++)
        {
            verlet_circle *c2 = sim->circles + j;
            vector axis_collision = {
                .x = c1->position_current.x - c2->position_current.x,
                .y = c1->position_current.y - c2->position_current.y
            };
            float dist = vector_length(axis_collision);
            int radius_sum = c1->radius + c2->radius;
            if (dist < radius_sum){
                vector n = {
                    .x = axis_collision.x / dist,
                    .y = axis_collision.y / dist
                };
                float delta = radius_sum - dist;
                c1->position_current.x += 0.5 * delta * n.x;
                c1->position_current.y += 0.5 * delta * n.y;

                c2->position_current.x += 0.5 * delta * n.x;
                c2->position_current.y += 0.5 * delta * n.y;
            }
        }
    }
}


void update_simulation(simulation *sim, float dt){
    apply_gravity(sim);
    apply_constraint(sim);
    solve_collisions(sim);
    update_positions(sim, dt);
}


void add_circle(simulation *sim, uint radius, float px, float py, unsigned char red, unsigned char green, unsigned char blue, float acc_x, float acc_y){
    verlet_circle new_circle = {
        .radius = radius,
        .position_old.x = px,
        .position_old.y = py,
        .position_current.x = px,
        .position_current.y = py,
        .acceleration.x = acc_x,
        .acceleration.y = acc_y,
        .r = red,
        .g = green,
        .b = blue
    };
    sim->circles[sim->circle_count] = new_circle;

    sim->circle_count += 1;

    if(sim->circle_count == sim->allocated_circles){
        sim->circles = realloc(sim->circles, (sizeof(verlet_circle) * sim->allocated_circles * 2));
        _check_malloc(sim->circles, __LINE__, __FILE__);

        sim->allocated_circles *= 2;
    }

}