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

float convert_circle_x(verlet_circle *c, uint sim_size){
    return (2.0 * c->position_current.x  / sim_size) - 1.0 - 0.776;
}

float convert_circle_y(verlet_circle *c, uint sim_size){
    // Invert the sign as opengl 1 is the top and -1 the bottom of the window
    return ((2.0 * c->position_current.y  / sim_size) - 1.0) * -1;
}

float convert_circle_radius(verlet_circle *c, uint sim_size){
    return (2.0 * c->radius / sim_size);
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

void draw_line(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
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
        double converted_x = convert_circle_x(c, sim_size);
        if (converted_x > 1. || converted_x < -1.) continue;
        double converted_y = convert_circle_y(c, sim_size);
        if (converted_y > 1. || converted_y < -1.) continue;

        double converted_radius = convert_circle_radius(c, sim_size);
        // printf("%zu : %lf, %lf -> %lf, %lf\n", i, c->position_current.x, c->position_current.y, converted_x, converted_y);
        // printf("%d,%d,%d\n", c->color.r, c->color.g, c->color.b);
		glColor3ub(c->color.r, c->color.g, c->color.b);
        draw_circle(converted_x, converted_y, converted_radius, 30);
    }

    glColor3ub(0, 0, 0);
    for (size_t i = 0; i < sim_get_stick_count(sim); i++)
    {
        stick *s = sim_get_nth_stick(sim, i);
        draw_line(convert_circle_x(s->p0, sim_size), convert_circle_y(s->p0, sim_size),
                    convert_circle_x(s->p1, sim_size), convert_circle_y(s->p1, sim_size));
    }

}