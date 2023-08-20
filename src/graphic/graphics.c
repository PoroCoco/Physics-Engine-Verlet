#include "graphics.h"
#include "config.h"
#include "../simulation/error_handler.h"
#include "../simulation/verlet_interface.h"
#include "../simulation/circle_verlet.h"

#include <GL/gl.h>
#include <stdio.h>

void gui_exit_with_error(const char *message, struct gui *gui){
    printf("Error\n");
    SDL_Log("ERROR : %s > %s\n", message, SDL_GetError());
    SDL_DestroyRenderer(gui->renderer);
    SDL_DestroyWindow(gui->window);
    SDL_Quit();
    exit(EXIT_FAILURE);
}


struct gui* init_gui(void){
    struct gui *gui = malloc(sizeof(struct gui));
    _check_malloc(gui, __LINE__, __FILE__);


    gui->window = NULL;
    gui->renderer = NULL;

    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        gui_exit_with_error("SDL Initialisation", gui);
    }

    gui->window = SDL_CreateWindow("Verlet Simulation", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (gui->window == NULL){
        gui_exit_with_error("Window creation failed", gui);
    }

    gui->renderer = SDL_CreateRenderer(gui->window, SDL_RENDERER_ACCELERATED, 0);
    if (gui->renderer == NULL){
        gui_exit_with_error("Renderer creation failed", gui);
    }


    return gui;
}

void end_gui(struct gui *gui){
    if (!gui) return;

    if (gui->renderer) SDL_DestroyRenderer(gui->renderer);
    if (gui->window) SDL_DestroyWindow(gui->window);
    
    free(gui);

    SDL_Quit();
}

void render_gui(struct gui *gui){
    SDL_RenderPresent(gui->renderer);
}




void draw_grid(struct gui *gui, verlet_sim_t *sim){
    SDL_SetRenderDrawColor(gui->renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
    for (size_t y = 0; y < sim_get_grid_height(sim); y++){
        SDL_RenderDrawLine(gui->renderer, 0, y*(WINDOW_HEIGHT/sim_get_grid_height(sim)), WINDOW_WIDTH, (y)*(WINDOW_HEIGHT/sim_get_grid_height(sim)));
    }

    for (size_t x = 0; x < sim_get_grid_width(sim); x++){
        SDL_RenderDrawLine(gui->renderer, x*(WINDOW_WIDTH/sim_get_grid_width(sim)), 0, x*(WINDOW_WIDTH/sim_get_grid_width(sim)), WINDOW_HEIGHT);
    }
}

void render_simulation(struct gui *gui, verlet_sim_t *sim){
    SDL_SetRenderDrawColor(gui->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(gui->renderer);

    
    if(sim_get_shape(sim) == CIRCLE){
        SDL_SetRenderDrawColor(gui->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        draw_circle(gui->renderer, sim_get_constraint_x(sim), sim_get_constraint_y(sim), sim_get_constraint_radius(sim));
    }else if (sim_get_shape(sim) == SQUARE){
        SDL_SetRenderDrawColor(gui->renderer, 125, 125, 125, SDL_ALPHA_OPAQUE);
        SDL_Rect rectangle = {.x = sim_get_constraint_x(sim) - sim_get_constraint_radius(sim), .y = sim_get_constraint_y(sim) - sim_get_constraint_radius(sim), .h = sim_get_constraint_radius(sim) * 2, .w = sim_get_constraint_radius(sim) * 2};
        SDL_RenderFillRect(gui->renderer, &rectangle);
    }

    

    // SDL_Rect *circles_rectangles = malloc(sizeof(SDL_Rect) * sim_get_object_count(sim));

    for (size_t i = 0; i < sim_get_object_count(sim); i++)
    {
        verlet_circle *c = sim_get_nth_circle(sim, i);
        if (c->position_current.x > WINDOW_WIDTH || c->position_current.x < 0) continue;
        if (c->position_current.y > WINDOW_HEIGHT || c->position_current.y < 0) continue;
        color_t clr = rainbow_color((c->position_current.x - c->position_old.x + c->position_current.y - c->position_old.y)/2.0);
        SDL_SetRenderDrawColor(gui->renderer, clr.r, clr.g, clr.b, SDL_ALPHA_OPAQUE);
        // SDL_Rect r = {.x = c->position_current.x - c->radius, .y = c->position_current.y - c->radius, .h = 2 * c->radius, .w = 2 * c->radius};
        // circles_rectangles[i] = r;
        draw_circle(gui->renderer, (int)c->position_current.x, (int)c->position_current.y, c->radius);
    }

    // SDL_RenderFillRects(gui->renderer, circles_rectangles, sim_get_object_count(sim));
    // free(circles_rectangles);

    draw_grid(gui, sim);

}


void draw_circle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < num_segments; ++i) {
        float theta = 2.0f * 3.1415926f * (float)i / (float)num_segments;
        float x = r * cos(theta);
        float y = r * sin(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void draw_squares(float top_left_x, float top_left_y, float bottom_right_x, float bottom_right_y) {
    glBegin(GL_TRIANGLES);
    
    glVertex2f(bottom_right_x, top_left_y);
    glVertex2f(bottom_right_x, bottom_right_y);
    glVertex2f(top_left_x, bottom_right_y);
    glVertex2f(bottom_right_x, top_left_y);
    glVertex2f(top_left_x, bottom_right_y);
    glVertex2f(top_left_x, top_left_y);
    
    glEnd();
}

void sim_render(verlet_sim_t *sim){
    uint sim_size = sim_get_height(sim);

    if(sim_get_shape(sim) == CIRCLE){
		glColor4f(1.0, 1.0, 1.0, 1.0);
        draw_circle(0., 0., sim_get_constraint_radius(sim)*2.0/sim_get_height(sim), 300);
    }else if (sim_get_shape(sim) == SQUARE){
		glColor4f(0.66, 0.66, 0.66, 1.0);
        double x = (2.0 * sim_get_constraint_x(sim) / sim_size) - 1.0;
        x -= 0.776;
        double y = (2.0 * sim_get_constraint_y(sim) / sim_size) - 1.0;
        double radius = sim_get_constraint_radius(sim) * 2.0/sim_size;
        draw_squares( x - radius, y + radius, x + radius, y - radius);
    }

    for (size_t i = 0; i < sim_get_object_count(sim); i++)
    {
        verlet_circle *c = sim_get_nth_circle(sim, i);
        double converted_x = (2.0 * c->position_current.x  / sim_size) - 1.0;
        converted_x -= 0.776;
        if (converted_x > 1. || converted_x < -1.) continue;
        double converted_y = (2.0 * c->position_current.y  / sim_size) - 1.0;
        // Invert the sign as opengl 1 is the top and -1 the bottom of the window
        converted_y *= -1;
        if (converted_y > 1. || converted_y < -1.) continue;

        double converted_radius = (2.0 * c->radius / sim_size);
        // printf("%zu : %lf, %lf -> %lf, %lf\n", i, c->position_current.x, c->position_current.y, converted_x, converted_y);
        // printf("%d,%d,%d\n", c->color.r, c->color.g, c->color.b);
		glColor3ub(c->color.r, c->color.g, c->color.b);
        draw_circle(converted_x, converted_y, converted_radius, 30);


        

    }


}