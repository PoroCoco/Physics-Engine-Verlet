#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "graphics.h"
#include "config.h"
#include "verlet_interface.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"
#include <GL/gl.h>

void spawn_random_circles(verlet_sim_t *sim, size_t count, int height, int width){
    for (size_t i = 0; i < count; i++)
        add_circle(sim, CIRCLE_RADIUS, rand()%width, rand()%height, rainbow_color(sim_get_object_count(sim)), 0, 0);
}

void spawn_billard_triangle(verlet_sim_t *sim, int center_x, int center_y, int ball_radius, size_t layer_count){
    assert(layer_count%2 == 0);
    int ball_diameter = ball_radius*2;
    // Getting the lowest(highest on  the screen) y coord of the ball
    for (size_t i = layer_count; i != 0; i--)
    {
        int current_y = center_y - (i/1.15 * ball_diameter); //<--- 1.15? why 💀 
        int current_x = center_x - (i/2.0 * ball_diameter);
        for (size_t ball = 0; ball < i; ball++)
        {  
            add_circle(sim, ball_radius, current_x+(ball*ball_diameter), current_y,  rainbow_color(sim_get_object_count(sim)), 0 , 0);
        }
    }
    
}

int main(int argc, char* argv[]) {

    struct gui *gui = init_gui();
    verlet_sim_t *sim = init_simulation(SQUARE, 1920/2, 1080/2, 1080/2, WINDOW_WIDTH, WINDOW_HEIGHT, GRID_WIDTH, GRID_HEIGHT, GRAV_X, GRAV_Y);


    assert(sim);

        // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // and prepare OpenGL stuff
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);

    
    SDL_GLContext gl_context = SDL_GL_CreateContext(gui->window);
    

    // setup imgui
    igCreateContext(NULL);
  
    //set docking
    ImGuiIO* ioptr = igGetIO();
    ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
  
    ImGui_ImplSDL2_InitForOpenGL(gui->window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);


    printf("loading succesful\n");
    
    srand(time(NULL));

    SDL_Event event;
    bool program_launched = true;
    bool button_mousedown = false;
    float zoom_level = 1.0;
    float view_offset_x = 0.0;
    float view_offset_y = 0.0;
    bool gui_spawn_player_ball = false;
    size_t gui_player_ball_id = 0;
    while(program_launched)
    {
        unsigned int start_time = SDL_GetPerformanceCounter();
        int display_w, display_h;
        SDL_GL_GetDrawableSize(gui->window, &display_w, &display_h);

        static float gui_constraint_radius = 1.0f;
        vector g = sim_get_gravity(sim);
        static int gui_gravity = 1;
        gui_gravity = g.y;
        static int gui_sub_steps = 5;
        static int gui_thread_count = 8;

        while(SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (ioptr->WantCaptureMouse) continue;
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

                case SDL_MOUSEWHEEL:
                    if (event.wheel.y > 0) {
                        // Zoom in
                        zoom_level *= 1.1f;
                    } else if (event.wheel.y < 0) {
                        // Zoom out
                        zoom_level /= 1.1f;
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_UP:
                        // Pan the view downward
                        view_offset_y -= 0.1f; 
                        break;
                    case SDLK_DOWN:
                        // Pan the view upward
                        view_offset_y += 0.1f;
                        break;
                    case SDLK_LEFT:
                        // Pan the view to the right
                        view_offset_x += 0.1f;
                        break;
                    case SDLK_RIGHT:
                        // Pan the view to the left 
                        view_offset_x -= 0.1f;
                        break;
                    case SDLK_s:
                        printf("s pressed, saving simulation\n");
                        sim_save_current_state(sim, "saved.txt");
                        continue;

                    case SDLK_u:
                        update_simulation(sim, 1/60.0);
                        continue;

                    case SDLK_g:
                        vector gravity = sim_get_gravity(sim);
                        gravity.y *= -1;
                        gui_gravity *= -1;
                        sim_set_gravity(sim, gravity);
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
            add_circle(sim, CIRCLE_RADIUS, x, y, rainbow_color(sim_get_object_count(sim)), 0, 0);
            // add_circle(sim, 4+(rand()%(CIRCLE_RADIUS-4)), x, y, rainbow_color(sim_get_object_count(sim)), 0, 0);
        }
        if (gui_spawn_player_ball){
            int x,y;
            SDL_GetMouseState( &x, &y );
            verlet_circle *c =  sim_get_nth_circle(sim, gui_player_ball_id);
            c->position_current.x = x/1.0;
            c->position_current.y = y/1.0;
            c->position_old.x = x/1.0;
            c->position_old.y = y/1.0;
        }

        // if (sim_get_object_count(sim) < 0) add_circle(sim, CIRCLE_RADIUS, CONSTRAINT_CENTER_X+300, CONSTRAINT_CENTER_Y+300, rainbow_color(sim_get_object_count(sim)), 0, 0);
        // if (sim_get_object_count(sim) < 0) add_circle(sim, CIRCLE_RADIUS, CONSTRAINT_CENTER_X-150, CONSTRAINT_CENTER_Y-150, rainbow_color(sim_get_object_count(sim)), 0, 0);
        // if (sim_get_object_count(sim) < 0) add_circle(sim, CIRCLE_RADIUS, CONSTRAINT_CENTER_X-150, CONSTRAINT_CENTER_Y+150, rainbow_color(sim_get_object_count(sim)), 0, 0);
        // if (sim_get_object_count(sim) < 0) add_circle(sim, CIRCLE_RADIUS, CONSTRAINT_CENTER_X+150, CONSTRAINT_CENTER_Y-150, rainbow_color(sim_get_object_count(sim)), 0, 0);
        // start imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        igNewFrame();

        

        igBegin("Control panel", NULL, 0);
        if(igSliderFloat("Constraint Radius", &gui_constraint_radius, 0.0f, 1.0f, "%.3f", 0)){
            sim_set_constraint_radius(sim, gui_constraint_radius*(sim_get_height(sim)/2.0));
        }
        if(igSliderInt("Gravity", &gui_gravity, -5000, 5000, "%d", 0)){
            vector sim_gravity = sim_get_gravity(sim);
            sim_gravity.y = gui_gravity;
            sim_set_gravity(sim, sim_gravity);  
        }
        if(igSliderInt("Sub Steps", &gui_sub_steps, 0, 20, "%d", 0)){
            sim_set_sub_steps(sim, gui_sub_steps);  
        }
        if(igSliderInt("Thread Count", &gui_thread_count, 0, 32, "%d", 0)){
            sim_set_thread_count(sim, gui_thread_count);  
        }
        ImVec2 buttonSize;
        buttonSize.x = 0;
        buttonSize.y = 0;
        if(igButton("Spawn Mouse Balls", buttonSize) && !gui_spawn_player_ball){
            gui_spawn_player_ball = !gui_spawn_player_ball;
            gui_player_ball_id = sim_get_object_count(sim);
            add_circle(sim, 30, 0, 0, rainbow_color(sim_get_object_count(sim)), 0, 0);
        }

        // ImVec2 buttonSize;
        // buttonSize.x = 0;
        // buttonSize.y = 0;
        // if (igButton("Button", buttonSize)) counter++;
        // igSameLine(0.0f, -1.0f);
        // igText("counter = %d", counter);

        igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);
        igText("Number of objects : %zu", sim_get_object_count(sim));
        igEnd();

        // unsigned int start_time_simulation = SDL_GetPerformanceCounter();
        update_simulation(sim, 1/60.0);
        // unsigned int end_time_simulation = SDL_GetPerformanceCounter();
        if (gui_spawn_player_ball){
            int x,y;
            SDL_GetMouseState( &x, &y );
            verlet_circle *c =  sim_get_nth_circle(sim, gui_player_ball_id);
            c->position_current.x = x/1.0;
            c->position_current.y = y/1.0;
            c->position_old.x = x/1.0;
            c->position_old.y = y/1.0;
        }

        // Rendering
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glTranslatef(view_offset_x, view_offset_y, 0.0f);
        float aspectRatio = (float)display_w / (float)display_h;
        if (display_w >= display_h) {
            glOrtho(-aspectRatio / zoom_level, aspectRatio / zoom_level, -1.0f / zoom_level, 1.0f / zoom_level, -1.0f, 1.0f);
        } else {
            glOrtho(-1.0f / zoom_level, 1.0f / zoom_level, -1.0f / aspectRatio / zoom_level, 1.0f / aspectRatio / zoom_level, -1.0f, 1.0f);
        }

        glViewport(0, 0, display_w, display_h);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        sim_render(sim);


        // Render ImGui and swap buffers
        igRender();
        SDL_GL_MakeCurrent(gui->window, gl_context);
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
        SDL_GL_SwapWindow(gui->window);


        unsigned int end_time = SDL_GetPerformanceCounter();
        float freq_time = SDL_GetPerformanceFrequency();
        float elapsedMS = (end_time - start_time) / freq_time*1000.0;
        // float elapsedMS_simulation = (end_time_simulation - start_time_simulation) / freq_time*1000.0;
        float fps_delay = floor(16.666 - elapsedMS);
        if(fps_delay>0){
            // printf("freeze for %f\n", fps_delay);
            SDL_Delay(fps_delay);
        }else if (fps_delay<-1){
            // printf("%zu objects in simulation\n", sim->circle_count);
            // program_launched = false;
        }

    }

    // clean up
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    igDestroyContext(NULL);

    SDL_GL_DeleteContext(gl_context);

    destroy_simulation(sim);
    end_gui(gui);

    argc = argc;
    argv = argv;

    return EXIT_SUCCESS;
}