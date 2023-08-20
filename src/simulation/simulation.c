#include "verlet_interface.h"
#include "circle_verlet.h"
#include "error_handler.h"
#include <time.h>
#include <stdio.h>
#include <pthread.h>

#include <stdlib.h>
#include "misc.h"
#include "circle_verlet.h"
#include "grid.h"

typedef struct verlet_sim {
    size_t circle_count;
    size_t allocated_circles;
    verlet_circle *circles;

    struct grid *grid;
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



verlet_sim_t *init_simulation(enum constraint_shape shape, float constraint_center_x, float constraint_center_y, unsigned int constraint_radius, unsigned int width, unsigned int height, unsigned int grid_width, unsigned int grid_height){
    verlet_sim_t *s = malloc(sizeof(verlet_sim_t));
    _check_malloc(s, __LINE__, __FILE__);
    s->circle_count = 0;
    s->circles = malloc(sizeof(verlet_circle));
    _check_malloc(s->circles, __LINE__, __FILE__);
    s->allocated_circles = 1;
    s->total_frames = 0;

    s->constraint_shape = shape;
    s->constraint_center = vector_create(constraint_center_x, constraint_center_y);
    s->constraint_radius = constraint_radius;

    s->grid = create_grid(grid_width, grid_height);    
    s->height = height;
    s->width = width;

    vector gravity = {.x = 0, .y = 1000};
    s->gravity = gravity;
    s->sub_steps = SUB_STEPS;
    s->thread_count = THREAD_COUNT;
    return s;
}

void destroy_simulation(verlet_sim_t *s){
    if (!s) return;
    if (s->circles) free(s->circles);
    if (s->grid) destroy_grid(s->grid);
    free(s);
}

void apply_gravity(verlet_sim_t *sim){
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        accelerate_circle(sim->circles + i, &(sim->gravity));
    }
}

void update_positions(verlet_sim_t *sim, float dt){
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        update_position_circle(sim->circles + i, dt);
    }
}

void apply_constraint(verlet_sim_t *sim){
    if (sim->constraint_shape == SQUARE){
        for (size_t i = 0; i < sim->circle_count; i++)
        {
            verlet_circle *c = sim->circles + i;
            if (c->position_current.y - c->radius < sim->constraint_center.y - sim->constraint_radius) c->position_current.y = sim->constraint_center.y - sim->constraint_radius + c->radius;
            if (c->position_current.y + c->radius > sim->constraint_center.y + sim->constraint_radius) c->position_current.y = sim->constraint_center.y + sim->constraint_radius - c->radius;
            if (c->position_current.x + c->radius > sim->constraint_center.x + sim->constraint_radius) c->position_current.x = sim->constraint_center.x + sim->constraint_radius - c->radius;
            if (c->position_current.x - c->radius < sim->constraint_center.x - sim->constraint_radius) c->position_current.x = sim->constraint_center.x - sim->constraint_radius + c->radius;
        }
    }else if (sim->constraint_shape == CIRCLE){
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
    }
}

void solve_circle_collision(verlet_circle *c1, verlet_circle *c2){
    if (c1 == c2) return;
    vector axis_collision = {
        .x = c1->position_current.x - c2->position_current.x,
        .y = c1->position_current.y - c2->position_current.y
    };
    
    //if circles are on the exact same position, prevents division by 0 and nan result. 
    if(axis_collision.x == 0.0 && axis_collision.y == 0.0) axis_collision.x = 0.01;

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

struct arguments_collision {
    verlet_sim_t *sim;
    uint col_begin;
    uint col_end;
};

void *solve_cell_collisions(void * thread_data){
    // printf("entering solveing\n");
    struct arguments_collision *arg_c = (struct arguments_collision*) thread_data;
    verlet_sim_t *sim = arg_c->sim;
    // printf("thread %ld args = sim:%p, begin:%u, end:%u\n",  pthread_self(), arg_c->sim, arg_c->col_begin, arg_c->col_end);
    // printf("thread %ld working on %u to %u\n", pthread_self(), arg_c->col_begin, arg_c->col_end);
    for (size_t x = arg_c->col_begin; x < arg_c->col_end; x++)
    {
        for (size_t y = 0; y < sim->grid->height; y++)
        {
            // if (sim->grid->grid[y][x].count > 0) printf("%zu objects at cord %zu,%zu\n", sim->grid->grid[y][x].count, x, y);
            // printf("thread %d working on %zu,%zu\n", pthread_self(), x, y);
            for (size_t i = 0; i < sim->grid->grid[y][x].count; i++)
            {
                for (int dx = -1; dx < 1; dx++)
                {
                    if (x+dx > sim->grid->width) continue;
                    for (int dy = -1; dy < 1; dy++)
                    {
                        if (y+dy > sim->grid->height) continue;
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
    return NULL;
}

void update_grid(verlet_sim_t *sim){
    //resets the dynamic array of indexes
    for (size_t y = 0; y < sim->grid->height; y++){
        for (size_t x = 0; x < sim->grid->width; x++){
            sim->grid->grid[y][x].count = 0;
        }
    }

    for (size_t i = 0; i < sim->circle_count; i++)
    {
        uint col = (int)(sim->circles[i].position_current.x /(sim->width/sim->grid->width));
        uint row = (int)(sim->circles[i].position_current.y /(sim->height/sim->grid->height));
        add_grid(sim->grid, row, col, i);
    }

}

void solve_threaded_collision(verlet_sim_t *sim){
    uint thread_count = sim->thread_count; 

    pthread_t threads[thread_count];
    struct arguments_collision *args_to_free[thread_count];

    int ret;
    for (size_t i = 0; i < thread_count; i++)
    {
        struct arguments_collision *args = malloc(sizeof(struct arguments_collision));
        args->sim = sim;
        args->col_begin = i * (sim->grid->width/thread_count);
        args->col_end = ((i+1)*(sim->grid->width/thread_count));
        // printf("sending thread %zu with section %u to %u\n", i, args.col_begin, args.col_end);
        ret = pthread_create(threads+i, NULL, solve_cell_collisions, (void *) args);   
        if(ret) {
            fprintf(stderr, "Thread creation error %d\n",ret);
        }
        args_to_free[i] = args;
    }

    for (size_t i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
    }

    for (size_t i = 0; i < thread_count; i++)
    {
        free(args_to_free[i]);
    }
    
}

void seq_col(verlet_sim_t *sim){
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        for (size_t j = 0; j < sim->circle_count; j++)
        {
            solve_circle_collision(&sim->circles[i], &sim->circles[j]);
        }
        
    }
    
}

void update_simulation(verlet_sim_t *sim, float dt){

    uint sub_steps = sim->sub_steps;
    float sub_dt = dt/(float)sub_steps;
    for (size_t i = 0; i < sub_steps; i++)
    {
        apply_gravity(sim);
        apply_constraint(sim);
        update_grid(sim);
        solve_threaded_collision(sim);
        // seq_col(sim);
        update_positions(sim, sub_dt);
    }
    sim->total_frames++;
}


void add_circle(verlet_sim_t *sim, uint radius, float px, float py, color_t color, float acc_x, float acc_y){
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

size_t sim_get_current_step(verlet_sim_t *sim){
    return sim->total_frames;
}

size_t sim_get_object_count(verlet_sim_t *sim){
    return sim->circle_count;
}

enum constraint_shape sim_get_shape(verlet_sim_t *sim){
    return sim->constraint_shape;
}

unsigned int sim_get_constraint_x(verlet_sim_t *sim){
    return sim->constraint_center.x;
}

unsigned int sim_get_constraint_y(verlet_sim_t *sim){
    return sim->constraint_center.y;
}

unsigned int sim_get_constraint_radius(verlet_sim_t *sim){
    return sim->constraint_radius;
}

unsigned int sim_get_grid_height(verlet_sim_t *sim){
    return sim->grid->height;
}

unsigned int sim_get_grid_width(verlet_sim_t *sim){
    return sim->grid->width;
}

verlet_circle *sim_get_nth_circle(verlet_sim_t *sim, unsigned int n){
    return sim->circles + n;
}

unsigned int sim_get_width(verlet_sim_t *sim){
    return sim->width;
}

unsigned int sim_get_height(verlet_sim_t *sim){
    return sim->height;
}


vector sim_get_gravity(verlet_sim_t *sim){
    return sim->gravity;
}

void sim_set_gravity(verlet_sim_t *sim, vector gravity){
    sim->gravity = gravity;
}

void sim_set_constraint_radius(verlet_sim_t *sim, int radius){
    sim->constraint_radius = radius;
}

void sim_set_sub_steps(verlet_sim_t *sim, uint sub_steps){
    sim->sub_steps = sub_steps;
}

void sim_set_thread_count(verlet_sim_t *sim, uint thread_count){
    sim->thread_count = thread_count;
}
