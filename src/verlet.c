#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include "graphics.h"
#include "simulation.h"


int main(int argc, char* argv[]) {

    struct gui *gui = init_gui();
    simulation *sim = init_simulation();
    
    // add_circle(sim, CIRCLE_RADIUS, 400, 300, 0, 0, 255, 0, 0);

    srand(6969);

    SDL_Event event;
    bool program_launched = true;
    bool button_mousedown = false;
    uint frame_counter = 0;

    while(program_launched)
    {
        uint start_time = SDL_GetPerformanceCounter();
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {

                case SDL_MOUSEBUTTONDOWN:
                    button_mousedown = true;
                    int x,y;
                    SDL_GetMouseState( &x, &y );
                    // add_circle(sim, CIRCLE_RADIUS, x, y, 0, 0, 255, 0, 0);
                    break;

                case SDL_MOUSEBUTTONUP:
                    button_mousedown = false;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        printf("Esc pressed, closing simulation\n");
                        program_launched = false;
                        continue;

                    default:
                        break;
                    }
                    break;

                case SDL_QUIT:
                    program_launched = false;
                    break;

                default:
                    break;
            }
        }
        if(button_mousedown){
            int x,y;
            SDL_GetMouseState( &x, &y );
            add_circle(sim, 4+(rand()%(CIRCLE_RADIUS-4)), x, y, rand()%255, rand()%255, rand()%255, 0, 0);
        }
        
        update_simulation(sim, 1/60.0);
        render_simulation(gui, sim);
        render_gui(gui);

        uint end_time = SDL_GetPerformanceCounter();
        float freq_time = SDL_GetPerformanceFrequency();
        float elapsedMS = (end_time - start_time) / freq_time*1000.0;
        float fps_delay = floor(16.666 - elapsedMS);
        if(fps_delay>0){
            // printf("freeze for %f\n", fps_delay);
            SDL_Delay(fps_delay);
        }else if (fps_delay<-1){
            // printf("%zu objects in simulation\n", sim->circle_count);
            // program_launched = false;
        }

        if(frame_counter%60 == 0){
            printf("%zu objects in simulation\n", sim->circle_count);
        }

        frame_counter++;
    }

    destroy_simulation(sim);
    end_gui(gui);

    argc = argc;
    argv = argv;

    return EXIT_SUCCESS;
}