#ifndef _GRAPHICS_H__
#define _GRAPHICS_H__

#include <SDL2/SDL.h>

#define WINDOW_WIDTH 910
#define WINDOW_HEIGHT 540


struct gui{
    SDL_Renderer *renderer;
    SDL_Window *window;
};


struct gui* init_gui(void);
void end_gui(struct gui *gui);
void render_gui(struct gui *gui);

#endif // _GRAPHICS_H__
