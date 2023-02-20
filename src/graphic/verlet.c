#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "graphics.h"
#include "config.h"
#include "verlet_interface.h"


int main(int argc, char* argv[]) {

    struct gui *gui = init_gui();
    verlet_sim_t *sim = init_simulation(CIRCLE, CONSTRAINT_CENTER_X, CONSTRAINT_CENTER_Y, CONSTRAINT_RADIUS, WINDOW_WIDTH, WINDOW_HEIGHT, GRID_WIDTH, GRID_HEIGHT);
    assert(sim);

    printf("loading succesful\n");

    srand(time(NULL));

    SDL_Event event;
    bool program_launched = true;
    bool button_mousedown = false;

    while(program_launched)
    {
        unsigned int start_time = SDL_GetPerformanceCounter();
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {

                case SDL_MOUSEBUTTONDOWN:
                    button_mousedown = true;
                    int x,y;
                    SDL_GetMouseState( &x, &y );
                    // add_circle(sim, 4+(rand()%(CIRCLE_RADIUS-4)), x, y, rainbow_color(sim->circle_count), 0, 0);
                    break;

                case SDL_MOUSEBUTTONUP:
                    button_mousedown = false;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_s:
                        printf("s pressed, saving simulation\n");
                        sim_save_current_state(sim, "saved.txt");
                        continue;

                    case SDLK_u:
                        update_simulation(sim, 1/60.0);
                        continue;
                    
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
            add_circle(sim, 4+(rand()%(CIRCLE_RADIUS-4)), x, y, rainbow_color(sim_get_object_count(sim)), 0, 0);
        }
        
        update_simulation(sim, 1/60.0);
        render_simulation(gui, sim);
        render_gui(gui);

        unsigned int end_time = SDL_GetPerformanceCounter();
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

        if(sim_get_current_step(sim)%10 == 0){
            printf("frame %zu, %zu objects in simulation. Frame time : %.3f\n", sim_get_current_step(sim), sim_get_object_count(sim), elapsedMS);
        }
    }

    destroy_simulation(sim);
    end_gui(gui);

    argc = argc;
    argv = argv;

    return EXIT_SUCCESS;
}