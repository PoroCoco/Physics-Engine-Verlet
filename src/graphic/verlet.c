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


int main(int argc, char* argv[]) {

    struct gui *gui = init_gui();
    verlet_sim_t *sim = init_simulation(SQUARE, 1920/2, 1080/2, 1080/2, WINDOW_WIDTH, WINDOW_HEIGHT, GRID_WIDTH, GRID_HEIGHT);

    spawn_random_circles(sim, 800, WINDOW_HEIGHT, WINDOW_WIDTH);
    

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
    while(program_launched)
    {
        unsigned int start_time = SDL_GetPerformanceCounter();
        int display_w, display_h;
        SDL_GL_GetDrawableSize(gui->window, &display_w, &display_h);

        static float gui_constraint_radius = 1.0f;
        static int gui_gravity = 1000;
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

        // Rendering
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspectRatio = (float)display_w / (float)display_h;
        if (display_w >= display_h) {
            glOrtho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
        } else {
            glOrtho(-1.0f, 1.0f, -1.0f / aspectRatio, 1.0f / aspectRatio, -1.0f, 1.0f);
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