#include <immintrin.h>

#include "sim.h"
#include "spatial_hashing.h"

void simd_solve_circle_collision(float * c1_xs, float * c1_ys, float * c2_xs, float * c2_ys, float* c1_radius, float* c2_radius,
                                __m256 *new_x1, __m256 *new_y1, __m256 *new_x2, __m256 *new_y2){

    // Loads the first circle x and y positions
    __m256 x_positions1 = _mm256_loadu_ps(c1_xs);
    __m256 y_positions1 = _mm256_loadu_ps(c1_ys);

    // Loads the second circle x and y positions
    __m256 x_positions2 = _mm256_loadu_ps(c2_xs);
    __m256 y_positions2 = _mm256_loadu_ps(c2_ys);

    // Loads the first and second circles radius
    __m256 c1_radius_vec = _mm256_loadu_ps(c1_radius);
    __m256 c2_radius_vec = _mm256_loadu_ps(c2_radius);

    // Computes the x and y of the axis of collision of the circles
    __m256 x_collision = _mm256_sub_ps(x_positions1, x_positions2);
    __m256 y_collision = _mm256_sub_ps(y_positions1, y_positions2);

    // Computes the distance of the collision axis
    __m256 x_squared = _mm256_mul_ps(x_collision, x_collision);
    __m256 y_squared = _mm256_mul_ps(y_collision, y_collision);
    __m256 distance = _mm256_add_ps(x_squared, y_squared);
    distance = _mm256_sqrt_ps(distance);
    
    // Sums the radius of the circles
    __m256 sum_radius = _mm256_add_ps(c1_radius_vec, c2_radius_vec);



    __m256 comparison_result = _mm256_cmp_ps(distance, sum_radius, _CMP_LT_OS);
    // Convert the comparison result into a bitmask
    int mask = _mm256_movemask_ps(comparison_result);



    // if (dist < sum_radius){ // mask this ? but how

        __m256 nx = _mm256_div_ps(x_collision, distance);
        __m256 ny = _mm256_div_ps(y_collision, distance);

        // float delta = radius_sum - dist;
        __m256 delta = _mm256_sub_ps(sum_radius, distance);
        __m256 constant_half = _mm256_set1_ps(0.5);
        __m256 constant_zero = _mm256_set1_ps(0.);

        // Define a vector mask for the blending operation
        __m256 mask_vector = _mm256_cmp_ps(distance, sum_radius, _CMP_LT_OS);

        // Computes new position of the first circle after collision
        __m256 x_1_movement = _mm256_mul_ps(delta, nx);
        x_1_movement = _mm256_mul_ps(x_1_movement, constant_half);
        *new_x1 = _mm256_add_ps(x_positions1, _mm256_and_ps(x_1_movement, mask_vector));

        __m256 y_1_movement = _mm256_mul_ps(delta, ny);
        y_1_movement = _mm256_mul_ps(y_1_movement, constant_half);
        *new_y1 = _mm256_add_ps(y_positions1, _mm256_and_ps(y_1_movement, mask_vector));

        // Computes new position of the second circle after collision
        __m256 x_2_movement = _mm256_mul_ps(delta, nx);
        x_2_movement = _mm256_mul_ps(x_2_movement, constant_half);
        *new_x2 = _mm256_sub_ps(x_positions2, _mm256_and_ps(x_2_movement, mask_vector));

        __m256 y_2_movement = _mm256_mul_ps(delta, ny);
        y_2_movement = _mm256_mul_ps(y_2_movement, constant_half);
        *new_y2 = _mm256_sub_ps(y_positions2, _mm256_and_ps(y_2_movement, mask_vector));
}

void simd_solve_seq(verlet_sim_t *sim){
    size_t max_neighbors = 1000;
    verlet_circle * neighbors[max_neighbors];

    size_t collision_count = 0;
    float c1_x[8] __attribute__((aligned(32))) ;
    float c1_y[8] __attribute__((aligned(32))) ;
    float c2_x[8] __attribute__((aligned(32))) ;
    float c2_y[8] __attribute__((aligned(32))) ;
    float c1_radius[8] __attribute__((aligned(32))) ;
    float c2_radius[8] __attribute__((aligned(32))) ;
    verlet_circle *circles_1[8] __attribute__((aligned(32))) ;
    verlet_circle *circles_2[8] __attribute__((aligned(32))) ;

    __m256 new_x1, new_y1, new_x2, new_y2;

    for (size_t i = 0; i < sim->circle_count; i++)
    {
        spacehash_query_neighbors(sim->space_grid, &sim->circles[i], (void**)neighbors, max_neighbors);
        size_t j = 0;
        verlet_circle *c1 = &sim->circles[i];
        while(j < max_neighbors && neighbors[j] != NULL)
        {
            if (c1 != neighbors[j]){
                c1_x[collision_count] = c1->position_current.x;
                c1_y[collision_count] = c1->position_current.y;
                c2_x[collision_count] = neighbors[j]->position_current.x;
                c2_y[collision_count] = neighbors[j]->position_current.y;
                c1_radius[collision_count] = (float)c1->radius;
                c2_radius[collision_count] = (float)neighbors[j]->radius;
                circles_1[collision_count] = c1;
                circles_2[collision_count] = neighbors[j];

                collision_count++;
                if (collision_count == 8){
                    simd_solve_circle_collision(c1_x, c1_y, c2_x, c2_y, c1_radius, c2_radius, &new_x1, &new_y1, &new_x2, &new_y2);
                    _mm256_store_ps(c1_x, new_x1);
                    _mm256_store_ps(c1_y, new_y1);
                    _mm256_store_ps(c2_x, new_x2);
                    _mm256_store_ps(c2_y, new_y2);

                    for (size_t k = 0; k < 8; k++)
                    {
                        circles_1[k]->position_current.x = c1_x[k];
                        circles_1[k]->position_current.y = c1_y[k];
                        circles_2[k]->position_current.x = c2_x[k];
                        circles_2[k]->position_current.y = c2_y[k];
                    }
                    
                    collision_count = 0;
                }
            }

            // // if circles are on the exact same position, prevents division by 0 and nan result. 
            // if(axis_collision.x == 0.0 && axis_collision.y == 0.0) axis_collision.x = 0.01;
            
            j++;
        }
    }

    // Solve collision for left over circles
    for (size_t i = 0; i < collision_count; i++)
    {
        solve_circle_collision(circles_1[i], circles_2[i]);
    }
    

}
