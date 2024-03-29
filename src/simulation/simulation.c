#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h> // For gettimeofday on Unix-like systems

#include "verlet_interface.h"
#include "circle_verlet.h"
#include "error_handler.h"
#include "misc.h"
#include "circle_verlet.h"
#include "sticks.h"
#include "spatial_hashing.h"
#include "simd.h"

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



verlet_sim_t *init_simulation(enum constraint_shape shape, float constraint_center_x, float constraint_center_y, unsigned int constraint_radius, unsigned int width, unsigned int height, int grav_x, int grav_y){
    verlet_sim_t *s = malloc(sizeof(verlet_sim_t));
    _check_malloc(s, __LINE__, __FILE__);
    
    s->circle_count = 0;
    s->stick_count = 0;
    s->total_frames = 0;

    s->constraint_shape = shape;
    s->constraint_center = vector_create(constraint_center_x, constraint_center_y);
    s->constraint_radius = constraint_radius;

    s->space_grid = spacehash_init(width, height, 10., (float (*)(void*))circle_get_position_x, (float (*)(void*))circle_get_position_y);
    s->height = height;
    s->width = width;
    s->biggest_circle_radius = 0;

    vector gravity = {.x = grav_x/1.0, .y = grav_y/1.0};
    s->gravity = gravity;
    s->sub_steps = SUB_STEPS;
    s->thread_count = THREAD_COUNT;
    return s;
}

void destroy_simulation(verlet_sim_t *s){
    if (!s) return;
    // if (s->circles) free(s->circles);
    // if (s->sticks) free(s->sticks);
    if (s->space_grid) spacehash_free(s->space_grid);
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
        if (!sim->circles[i].pinned) update_position_circle(sim->circles + i, dt);
    }
}

void apply_constraint(verlet_sim_t *sim){
    if (sim->constraint_shape == SQUARE){
        for (size_t i = 0; i < sim->circle_count; i++)
        {
            verlet_circle *c = sim->circles + i;
            float vel_x = (c->position_current.x - c->position_old.x)/1.5;
            float vel_y = (c->position_current.y - c->position_old.y)/1.5;

            if (c->position_current.y - c->radius < sim->constraint_center.y - sim->constraint_radius){
                c->position_current.y = sim->constraint_center.y - sim->constraint_radius + c->radius;
                c->position_old.y = c->position_current.y + vel_y;
            } 
            if (c->position_current.y + c->radius > sim->constraint_center.y + sim->constraint_radius){
                c->position_current.y = sim->constraint_center.y + sim->constraint_radius - c->radius;
                c->position_old.y = c->position_current.y + vel_y;
            } 
            if (c->position_current.x + c->radius > sim->constraint_center.x + sim->constraint_radius){
                c->position_current.x = sim->constraint_center.x + sim->constraint_radius - c->radius;
                c->position_old.x = c->position_current.x + vel_x;
            } 
            if (c->position_current.x - c->radius < sim->constraint_center.x - sim->constraint_radius){
                c->position_current.x = sim->constraint_center.x - sim->constraint_radius + c->radius;
                c->position_old.x = c->position_current.x + vel_x;
            }
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
        
        if (!c1->pinned && !c2->pinned){
            c1->position_current.x += 0.5 * delta * n.x;
            c1->position_current.y += 0.5 * delta * n.y;

            c2->position_current.x -= 0.5 * delta * n.x;
            c2->position_current.y -= 0.5 * delta * n.y;
        }else if (c1->pinned && !c2->pinned){
            c2->position_current.x -= delta * n.x;
            c2->position_current.y -= delta * n.y;
        }else{
            c1->position_current.x -= delta * n.x;
            c1->position_current.y -= delta * n.y;
        }
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
    size_t max_circles = 5000;
    size_t max_neighbors = 5000;
    verlet_circle * bucket_circles[max_circles];
    verlet_circle * neighbors_circle[max_circles];
    size_t row_count = spacehash_get_row_count(sim->space_grid);
    for (size_t col = arg_c->col_begin; col <= arg_c->col_end; col++)
    {
        for (size_t row = 0; row < row_count; row++)
        {
            spacehash_query_bucket(sim->space_grid, row, col, (void **)bucket_circles, max_circles);

            // For every circle in the bucket
            for (size_t i = 0; i < max_circles && bucket_circles[i] != NULL; i++)
            {
                spacehash_query_neighbors(sim->space_grid, bucket_circles[i], (void **)neighbors_circle, max_neighbors);

                // For every neighboring circles
                for (size_t j = 0; j < max_neighbors && neighbors_circle[j] != NULL; j++)
                {
                    solve_circle_collision(bucket_circles[i], neighbors_circle[j]);
                }
            }
        }
    }
    return NULL;
}

void update_spatial_hashing(verlet_sim_t *sim){
    spacehash_reset(sim->space_grid);

    for (size_t i = 0; i < sim->circle_count; i++)
    {
        spacehash_add(sim->space_grid, &sim->circles[i]);
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
        args->col_begin = i * (spacehash_get_column_count(sim->space_grid) / thread_count);
        args->col_end = ((i+1)*(spacehash_get_column_count(sim->space_grid) / thread_count)) - 1;
        
        // Take all the remaining columns in case of col_count%thread_count != 0
        if (i == thread_count-1) args->col_end = spacehash_get_column_count(sim->space_grid) - 1;

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
    size_t max_neighbors = 1000;
    verlet_circle * neighbors[max_neighbors];
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        spacehash_query_neighbors(sim->space_grid, &sim->circles[i], (void**)neighbors, max_neighbors);
        size_t j = 0;
        while(j < max_neighbors && neighbors[j] != NULL)
        {
            solve_circle_collision(&sim->circles[i], neighbors[j]);
            j++;
        }
        // for (size_t j = 0; j < sim->circle_count; j++)
        // {
        //     solve_circle_collision(&sim->circles[i], &sim->circles[j]);
        // }
        
    }
}


struct args_collide {
    verlet_sim_t *sim;
    bool *collisions;
    uint circle_begin;
    uint circle_end;
};


void *circle_collide(void * thread_data){
    struct args_collide *arg_c = (struct args_collide*) thread_data;
    verlet_sim_t *sim = arg_c->sim;
    bool *collisions = arg_c->collisions;
    if (arg_c->circle_end >= sim->circle_count) arg_c->circle_end = sim->circle_count - 1;

    // printf("Thread starting collision between %u and %u\n",arg_c->circle_begin, arg_c->circle_end );
    for (size_t i = arg_c->circle_begin; i <= arg_c->circle_end; i++)
    {
        for (size_t j = 0; j < sim->circle_count; j++)
        {
            if (i == j){
                collisions[(i*sim->circle_count) + j] = false;    
                continue;
            }
            vector axis_collision = {
                .x = sim->circles[i].position_current.x - sim->circles[j].position_current.x,
                .y = sim->circles[i].position_current.y - sim->circles[j].position_current.y
            };
            
            //if circles are on the exact same position, prevents division by 0 and nan result. 
            if(axis_collision.x == 0.0 && axis_collision.y == 0.0) axis_collision.x = 0.01;

            float dist = vector_length(axis_collision);
            int radius_sum = sim->circles[i].radius + sim->circles[j].radius;

            collisions[(i*sim->circle_count) + j] = dist < (float)radius_sum;
        }
    }
    return NULL;
}

void thread_col(verlet_sim_t *sim){
    bool *collisions = malloc(sizeof(*collisions)*  sim->circle_count * sim->circle_count);

    struct timeval start_time, end_time;
    double time_spent;

    // Measure the start time
    gettimeofday(&start_time, NULL);

    //get collisions multihreaded
    uint thread_count = sim->thread_count; 

    pthread_t threads[thread_count];
    struct args_collide *args_to_free[thread_count];

    int ret;
    for (size_t i = 0; i < thread_count; i++)
    {
        struct args_collide *args = malloc(sizeof(struct args_collide));
        args->sim = sim;
        int ratio = sim->circle_count/thread_count;
        if(sim->circle_count%thread_count != 0) ratio++;

        args->circle_begin = i * (ratio);
        args->circle_end = ((i+1)*(ratio)) - 1;
        args->collisions = collisions;
        // printf("sending thread %zu with section %u to %u\n", i, args.col_begin, args.col_end);
        ret = pthread_create(threads+i, NULL, circle_collide, (void *) args);   
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

    // Measure the end time after completing the multithreaded collision detection
    gettimeofday(&end_time, NULL);
    time_spent = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1e3;
    printf("\tTime spent in multithreaded collision detection: %f microseconds\n", time_spent);



    //solve collisions 
    // Measure the start time
    gettimeofday(&start_time, NULL);

    for (size_t i = 0; i < sim->circle_count; i++)
    {
        for (size_t j = 0; j < sim->circle_count; j++)
        {
            if (i == j) continue;
            if (!collisions[(i*sim->circle_count) + j]) continue;
            // printf("collide betwewen %zu and %zu\n", i,j);

            vector axis_collision = {
                .x = sim->circles[i].position_current.x - sim->circles[j].position_current.x,
                .y = sim->circles[i].position_current.y - sim->circles[j].position_current.y
            };
            
            //if circles are on the exact same position, prevents division by 0 and nan result. 
            if(axis_collision.x == 0.0 && axis_collision.y == 0.0) axis_collision.x = 0.01;

            float dist = vector_length(axis_collision);
            int radius_sum = sim->circles[i].radius + sim->circles[j].radius;

            vector n = {
                .x = axis_collision.x / dist,
                .y = axis_collision.y / dist
            };
            float delta = radius_sum - dist;

            sim->circles[i].position_current.x += 0.5 * delta * n.x;
            sim->circles[i].position_current.y += 0.5 * delta * n.y;

            sim->circles[j].position_current.x -= 0.5 * delta * n.x;
            sim->circles[j].position_current.y -= 0.5 * delta * n.y;
        }
    }

    free(collisions);
    // Measure the end time after completing the collision resolution
    gettimeofday(&end_time, NULL);
    time_spent = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1e3;
    printf("\tTime spent in collision resolution: %f microseconds\n", time_spent);
}

void solve_naive(verlet_sim_t *sim){
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        for (size_t j = 0; j < sim->circle_count; j++)
        {
            solve_circle_collision(&sim->circles[i], &sim->circles[j]);
        }
    }
    
}

void update_sticks(verlet_sim_t *sim, float sub_dt, bool right_to_left){
    // printf("updating %zu sitcks\n",sim->stick_count );
    if (right_to_left){
        for (size_t i = 0; i < sim->stick_count; i++)
            stick_update(&sim->sticks[i]);
    }else{
        for (size_t i = sim->stick_count; i > 0 ; i--)
            stick_update(&sim->sticks[i-1]);

    }
}


void update_simulation(verlet_sim_t *sim, float dt){

    uint sub_steps = sim->sub_steps;
    float sub_dt = dt/(float)sub_steps;
    struct timeval start_time, end_time;
    struct timeval total_start_time, total_end_time;
    double time_spent;  
    gettimeofday(&total_start_time, NULL);    

    for (size_t i = 0; i < sub_steps; i++)
    {

        gettimeofday(&start_time, NULL);    
        apply_gravity(sim);
        gettimeofday(&end_time, NULL); 
        time_spent = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1e3;
        // printf("Time spent in apply gravity: %f milliseconds\n", time_spent);
        

        gettimeofday(&start_time, NULL);    
        apply_constraint(sim);
        gettimeofday(&end_time, NULL);
        time_spent = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1e3;
        // printf("Time spent in apply constraint: %f milliseconds\n", time_spent);
     
     
        gettimeofday(&start_time, NULL);
        // thread_col(sim);
        update_spatial_hashing(sim);
        solve_threaded_collision(sim);
        // seq_col(sim);
        // solve_naive(sim);
        // simd_solve_seq(sim);
        gettimeofday(&end_time, NULL);
        time_spent = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1e3;
        // printf("Time spent in collisions: %f milliseconds\n", time_spent);


        gettimeofday(&start_time, NULL);    
        update_positions(sim, sub_dt);
        gettimeofday(&end_time, NULL);
        time_spent = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1e3;
        // printf("Time spent in update positions: %f milliseconds\n", time_spent);
    
        update_sticks(sim, sub_dt, ((i%2)==0));
    }
    sim->total_frames++;
    gettimeofday(&total_end_time, NULL);    
    long long int time_spent_usec = (total_end_time.tv_sec - total_start_time.tv_sec) * 1000000LL + (total_end_time.tv_usec - total_start_time.tv_usec);
    double time_spent_ms = time_spent_usec / 1e3;
    // printf("Simulation frametime : %f milliseconds\n", time_spent_ms);

}


verlet_circle * add_circle(verlet_sim_t *sim, uint radius, float px, float py, color_t color, float acc_x, float acc_y, bool pinned){
    verlet_circle *new_circle = &sim->circles[sim->circle_count];
    new_circle->radius = radius;
    new_circle->position_old.x = px;
    new_circle->position_old.y = py;
    new_circle->position_current.x = px;
    new_circle->position_current.y = py;
    new_circle->acceleration.x = acc_x;
    new_circle->acceleration.y = acc_y;
    new_circle->color = color;
    new_circle->pinned = pinned;

    sim->circle_count += 1;

    // Update the spatial hashing width if the new circle have the biggest radius 
    if (new_circle->radius > sim->biggest_circle_radius && new_circle->radius > 10){
        sim->biggest_circle_radius = new_circle->radius;
        spacehash_update_cell_width(&sim->space_grid, 2*new_circle->radius);
    }

    // Pointer on the circle 
    return  &sim->circles[sim->circle_count - 1];
}

stick * add_stick(verlet_sim_t *sim, verlet_circle *p0, verlet_circle *p1, float len){
    sim->sticks[sim->stick_count].p0 = p0;
    sim->sticks[sim->stick_count].p1 = p1;
    sim->sticks[sim->stick_count].length = len;

    sim->stick_count++;

    return &sim->sticks[sim->stick_count - 1];
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

size_t sim_get_grid_row_count(verlet_sim_t *sim){
    return spacehash_get_row_count(sim->space_grid);
}

size_t sim_get_grid_col_count(verlet_sim_t *sim){
    return spacehash_get_column_count(sim->space_grid);
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

size_t sim_get_stick_count(verlet_sim_t *sim){
    return sim->stick_count;
}

stick * sim_get_nth_stick(verlet_sim_t *sim, unsigned int n){
    return &sim->sticks[n];
}

verlet_circle * sim_get_circle_at_coord(verlet_sim_t *sim, float x, float y){
    verlet_circle *c = NULL;
    for (size_t i = 0; i < sim->circle_count; i++)
    {
        c = &sim->circles[i];
        if (c->position_current.x - c->radius < x && c->position_current.x + c->radius > x
                && c->position_current.y - c->radius < y && c->position_current.y + c->radius > y)
            {
                return c;
            }
    }
    
    return NULL;
}
