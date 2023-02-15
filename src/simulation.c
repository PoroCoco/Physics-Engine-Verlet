#include "simulation.h"
#include "circle_verlet.h"
#include "error_handler.h"
#include <time.h>



vector gravity = {.x = 0, .y = 1000};

simulation *init_simulation(void){
    simulation *s = malloc(sizeof(simulation));
    _check_malloc(s, __LINE__, __FILE__);
    s->circle_count = 0;
    s->circles = malloc(sizeof(verlet_circle));
    _check_malloc(s->circles, __LINE__, __FILE__);
    s->allocated_circles = 1;
    s->total_frames = 0;

    s->constraint_center = vector_create(CONSTRAINT_CENTER_X, CONSTRAINT_CENTER_Y);
    s->constraint_radius = CONSTRAINT_RADIUS;

    s->grid = create_grid(GRID_WIDTH, GRID_HEIGHT);    

    return s;
}

void destroy_simulation(simulation *s){
    if (!s) return;
    if (s->circles) free(s->circles);
    if (s->grid) destroy_grid(s->grid);
    free(s);
}

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
    #ifdef SQUARE_BOUNDARY
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        verlet_circle *c = sim->circles + i;
        if (c->position_current.y - c->radius < sim->constraint_center.y - sim->constraint_radius) c->position_current.y = sim->constraint_center.y - sim->constraint_radius + c->radius;
        if (c->position_current.y + c->radius > sim->constraint_center.y + sim->constraint_radius) c->position_current.y = sim->constraint_center.y + sim->constraint_radius - c->radius;
        if (c->position_current.x + c->radius > sim->constraint_center.x + sim->constraint_radius) c->position_current.x = sim->constraint_center.x + sim->constraint_radius - c->radius;
        if (c->position_current.x - c->radius < sim->constraint_center.x - sim->constraint_radius) c->position_current.x = sim->constraint_center.x - sim->constraint_radius + c->radius;
    }
    #endif



    #ifdef CIRCLE_BOUNDARY
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        verlet_circle *c = sim->circles + i;
        vector to_circle;
        to_circle.x = c->position_current.x - sim->constraint_center.x;
        to_circle.y = c->position_current.y - sim->constraint_center.y;
        float dist = vector_length(to_circle);

        if (dist > sim->constraint_radius - (float)c->radius){
            vector n = {
                .x = to_circle.x / dist,
                .y = to_circle.y / dist
            };
            c->position_current.x = sim->constraint_center.x + n.x * (sim->constraint_radius - (float)(c->radius));
            c->position_current.y = sim->constraint_center.y + n.y * (sim->constraint_radius - (float)(c->radius));
        }
    }
    #endif
    

}

void solve_circle_collision(verlet_circle *c1, verlet_circle *c2){
    if (c1 == c2) return;
    vector axis_collision = {
        .x = c1->position_current.x - c2->position_current.x,
        .y = c1->position_current.y - c2->position_current.y
    };

    float dist = vector_length(axis_collision);
    int radius_sum = c1->radius + c2->radius;

    if (dist < (float)radius_sum){
        vector n = {
            .x = axis_collision.x / dist,
            .y = axis_collision.y / dist
        };
        float delta = radius_sum - dist;
        c1->position_current.x += 0.5 * delta * n.x;
        c1->position_current.y += 0.5 * delta * n.y;

        c2->position_current.x -= 0.5 * delta * n.x;
        c2->position_current.y -= 0.5 * delta * n.y;
    }
}

void solve_cell_collisions(simulation *sim){
    // printf("entering solveing\n");
    for (size_t x = 0; x < sim->grid->width; x++)
    {
        for (size_t y = 0; y < sim->grid->height; y++)
        {
            // if (sim->grid->grid[y][x].count > 0) printf("%zu objects at cord %zu,%zu\n", sim->grid->grid[y][x].count, x, y);
            for (size_t i = 0; i < sim->grid->grid[y][x].count; i++)
            {
                for (int dx = -1; dx < 1; dx++)
                {
                    if (x + dx < 0 || x+dx > sim->grid->width) continue;
                    for (int dy = -1; dy < 1; dy++)
                    {
                        if (y + dy < 0 || y+dy > sim->grid->height) continue;
                        for (size_t j = 0; j < sim->grid->grid[y + dy][x + dx].count; j++)
                        {
                            // printf("solving between grid %zu,%zu and %zu,%zu : c%zu and c%zu\n", y,x, y+dy, x+dx, i, j);
                            solve_circle_collision(sim->circles + sim->grid->grid[y][x].array[i], sim->circles + sim->grid->grid[y + dy][x + dx].array[j]);                            
                        }
                    }
                }
            }
        }
    }
}

void update_grid(simulation *sim){
    //resets the dynamic array of indexes
    for (size_t y = 0; y < sim->grid->height; y++){
        for (size_t x = 0; x < sim->grid->width; x++){
            sim->grid->grid[y][x].count = 0;
        }
    }

    for (size_t i = 0; i < sim->circle_count; i++)
    {
        uint col = (int)(sim->circles[i].position_current.x /(WINDOW_WIDTH/sim->grid->width));
        uint row = (int)(sim->circles[i].position_current.y /(WINDOW_HEIGHT/sim->grid->height));
        add_grid(sim->grid, row, col, i);
    }

}


void update_simulation(simulation *sim, float dt){

    float sub_dt = dt/(float)SUB_STEPS;
    if (sim->circle_count < 1000) add_circle(sim, 4+(rand()%(CIRCLE_RADIUS-4)), WINDOW_WIDTH/2.0 + rand()%5, WINDOW_HEIGHT/2.0+rand()%5, random_color(), 0, 0);
    for (size_t i = 0; i < SUB_STEPS; i++)
    {
        apply_gravity(sim);
        apply_constraint(sim);
        update_grid(sim);
        solve_cell_collisions(sim);
        update_positions(sim, sub_dt);
    }

}


void add_circle(simulation *sim, uint radius, float px, float py, color_t color, float acc_x, float acc_y){
    verlet_circle new_circle = {
        .radius = radius,
        .position_old.x = px,
        .position_old.y = py,
        .position_current.x = px,
        .position_current.y = py,
        .acceleration.x = acc_x,
        .acceleration.y = acc_y,
        .color = color
    };
    sim->circles[sim->circle_count] = new_circle;

    sim->circle_count += 1;

    if(sim->circle_count == sim->allocated_circles){
        sim->circles = realloc(sim->circles, (sizeof(verlet_circle) * sim->allocated_circles * 2));
        _check_malloc(sim->circles, __LINE__, __FILE__);

        sim->allocated_circles *= 2;
    }

}