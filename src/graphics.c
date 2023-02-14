#include "graphics.h"
#include "error_handler.h"

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

    gui->window = SDL_CreateWindow("Verlet Simulation", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (gui->window == NULL){
        gui_exit_with_error("Window creation failed", gui);
    }

    gui->renderer = SDL_CreateRenderer(gui->window, SDL_RENDERER_SOFTWARE, 0);
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



// void render_simulation(SDL_Renderer *renderer, SDL_Window *window){

    



//     // for(uint y = 0; y < LIGNE; y++){
//     //     for(uint x = 0; x < COLONNE; x++){
//     //         if(SDL_SetRenderDrawColor(renderer, world[y][x].color.R, world[y][x].color.G, world[y][x].color.B, world[y][x].color.ALPHA) != 0){
//     //             SDL_ExitWithError("Changement couleur impossible",window,renderer);
//     //         }
//     //         SDL_Rect rectangle = {PIXEL_WIDTH * x, PIXEL_HEIGHT * y,PIXEL_WIDTH,PIXEL_HEIGHT};
//     //         if(SDL_RenderFillRect(renderer, &rectangle) != 0){
//     //             SDL_ExitWithError("Impossible de dessiner un pixel(rectangle)",window,renderer);
//     //         }
//     //     }
//     // }
// }