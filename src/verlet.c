#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include "graphics.h"
#include "simulation.h"


int main(int argc, char* argv[]) {

    struct gui *gui = init_gui();
    simulation *sim = init_simulation();
    
    srand(time(NULL));

    SDL_Event event;
    bool program_launched = true;
    bool button_mousedown = false;

    while(program_launched)
    {
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


        render_simulation(gui, sim);

        render_gui(gui);
    }

    destroy_simulation(sim);
    end_gui(gui);

    return EXIT_SUCCESS;
}