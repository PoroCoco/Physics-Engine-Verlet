#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include "graphics.h"
#include "simulation.h"

#define CIRCLE_RADIUS 5

int main(int argc, char* argv[]) {

    struct gui *gui = init_gui();
    simulation *sim = init_simulation();
    
    add_circle(sim, CIRCLE_RADIUS, 150, 300, 0, 0, 255, 0, 0);

    srand(time(NULL));

    SDL_Event event;
    bool program_launched = true;
    bool button_mousedown = false;

    while(program_launched)
    {
        uint start_time = SDL_GetPerformanceCounter();
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {

                case SDL_MOUSEBUTTONDOWN:
                    button_mousedown = true;
                    break;

                case SDL_MOUSEBUTTONUP:
                    button_mousedown = false;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        printf("Esc pressed, closing simulation\n");
                        program_launched = SDL_FALSE;
                        continue;

                    default:
                        break;
                    }
                    break;

                case SDL_QUIT:
                    program_launched = SDL_FALSE;
                    break;

                default:
                    break;
            }
        }
        if(button_mousedown){
            add_circle(sim, CIRCLE_RADIUS, event.button.x, event.button.y, 0, 0, 255, 0, 0);
        }

        update_simulation(sim, 1/60.0);
        render_simulation(gui, sim);
        render_gui(gui);

        uint end_time = SDL_GetPerformanceCounter();
        float freq_time = SDL_GetPerformanceFrequency();
        float elapsedMS = (end_time - start_time) / freq_time*1000.0;
        float fps_delay = floor(16.666 - elapsedMS);
        if(fps_delay>0){
            printf("freeze for %f\n", fps_delay);
            SDL_Delay(fps_delay);
        } 

    }

    destroy_simulation(sim);
    end_gui(gui);

    button_mousedown = button_mousedown;
    argc = argc;
    argv = argv;

    return EXIT_SUCCESS;
}