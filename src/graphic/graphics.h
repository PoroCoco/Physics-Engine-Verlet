#ifndef _GRAPHICS_H__
#define _GRAPHICS_H__

#if defined(__linux__) || defined(__unix__) 
    #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32) 
    #include <SDL.h>
#endif

#include "verlet_interface.h"


struct gui{
    SDL_Renderer *renderer;
    SDL_Window *window;
};


struct gui* init_gui(void);
void end_gui(struct gui *gui);
void render_gui(struct gui *gui);

/// @brief updates the gui's renderer with the circles in the simulation
/// @param gui the gui to update
/// @param sim the simulation with the informations
void render_simulation(struct gui *gui, verlet_sim_t *sim);

void sim_render(verlet_sim_t *sim);

#endif // _GRAPHICS_H__
