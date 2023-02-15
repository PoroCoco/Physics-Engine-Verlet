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

    gui->window = SDL_CreateWindow("Verlet Simulation", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_FULLSCREEN);
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



//How to draw filled circle with SDL2 : https://gist.github.com/Gumichan01/332c26f6197a432db91cc4327fcabb1c
int
draw_circle(SDL_Renderer * renderer, int x, int y, int radius)
{
    int offsetx, offsety, d;
    int status;

    offsetx = 0;
    offsety = radius;
    d = radius -1;
    status = 0;

    while (offsety >= offsetx) {

        status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
                                     x + offsety, y + offsetx);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
                                     x + offsetx, y + offsety);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
                                     x + offsetx, y - offsety);
        status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
                                     x + offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }

    return status;
}


void render_simulation(struct gui *gui, simulation *sim){
    SDL_SetRenderDrawColor(gui->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(gui->renderer);
    
    #ifdef CIRCLE_BOUNDARY
    SDL_SetRenderDrawColor(gui->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    draw_circle(gui->renderer, (int)(WINDOW_WIDTH/2), (int)(WINDOW_HEIGHT/2), (int)((WINDOW_HEIGHT-100)/2));
    #elif SQUARE_BOUNDARY
    SDL_SetRenderDrawColor(gui->renderer, 125, 125, 125, SDL_ALPHA_OPAQUE);
    SDL_Rect rectangle = {.x = (WINDOW_WIDTH/4), .y = 50, .h = WINDOW_HEIGHT-100, .w = WINDOW_WIDTH - 2*(WINDOW_WIDTH/4), };
    SDL_RenderFillRect(gui->renderer, &rectangle);
    #endif


    for (size_t i = 0; i < sim->circle_count; i++)
    {
        verlet_circle *c = sim->circles + i;
        if (c->position_current.x > WINDOW_WIDTH || c->position_current.x < 0) continue;
        if (c->position_current.y > WINDOW_HEIGHT || c->position_current.y < 0) continue;
        SDL_SetRenderDrawColor(gui->renderer, c->color.r, c->color.g, c->color.b, SDL_ALPHA_OPAQUE);
        draw_circle(gui->renderer, (int)c->position_current.x, (int)c->position_current.y, c->radius);
    }
}
