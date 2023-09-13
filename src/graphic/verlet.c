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
    int left_padding = sim_get_constraint_x(sim) - sim_get_constraint_radius(sim);
    for (size_t i = 0; i < count; i++)
        add_circle(sim, 3, left_padding + rand()%width, rand()%height, rainbow_color(sim_get_object_count(sim)), 0, 0, false);
}

void spawn_random_sticks(verlet_sim_t *sim, size_t count, int height, int width, size_t knots){
    for (size_t i = 0; i < count; i++){
        verlet_circle *p0 = add_circle(sim, CIRCLE_RADIUS, rand()%width, rand()%height, rainbow_color(sim_get_object_count(sim)), 0, 0, true);
        for (size_t j = 0; j < knots; j++){
            verlet_circle *p1 = add_circle(sim, CIRCLE_RADIUS, rand()%width, rand()%height, rainbow_color(sim_get_object_count(sim)), 0, 0, false);
            add_stick(sim, p0, p1, 100.0);
            p0 = p1;
        }
    }
}

void spawn_cloth(verlet_sim_t *sim, float spread, int cols, int rows){
    float start_x = 600.0;
    float start_y = 200.0;
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            add_circle(sim, CIRCLE_RADIUS, start_x + j*spread, start_y + i*spread, rainbow_color(sim_get_object_count(sim)), 0, 0, (i==0 && j == 0) || (i == 0  && j == cols-1));
        }
    }

    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            int c_index = i*cols + j;
            verlet_circle *c = sim_get_nth_circle(sim, c_index);
            if (j + 1 < cols) add_stick(sim, c, sim_get_nth_circle(sim, c_index + 1), spread);
            if (i + 1 < rows) add_stick(sim, c, sim_get_nth_circle(sim, c_index + cols), spread);
        }
    }
}

void spawn_billard_triangle(verlet_sim_t *sim, int center_x, int center_y, int ball_radius, size_t layer_count){
    assert(layer_count%2 == 0);
    int ball_diameter = ball_radius*2;
    // Getting the lowest(highest on  the screen) y coord of the ball
    for (size_t i = layer_count; i != 0; i--)
    {
        int current_y = center_y - (i/1.075 * ball_diameter); //<--- 1.075? why 💀 
        int current_x = center_x - (i/2.0 * ball_diameter);
        for (size_t ball = 0; ball < i; ball++)
        {  
            add_circle(sim, ball_radius, current_x+(ball*ball_diameter), current_y,  rainbow_color(sim_get_object_count(sim)), 0 , 0, false);
        }
    }
}

void spawn_plinko(verlet_sim_t *sim, int rows, int post_radius){
    int sim_h = sim_get_height(sim);
    int sim_w = sim_get_height(sim);
    int row_posts = 3;
    int height_start = sim_h/10;
    int y_spacing = (sim_h - sim_h/10 - height_start) / rows;
    int width_start = sim_w/10;
    int x_spacing = (sim_w - sim_w/10 - width_start) / (row_posts+rows-1);

    color_t post_color = {.r = 0, .g = 0, .b = 0 };
    for (int row = 0; row < rows; row++)
    {
        for (int post = 0; post < row_posts; post++)
        {
            add_circle(sim, post_radius, (WINDOW_WIDTH-WINDOW_HEIGHT)+ width_start/2 + x_spacing*post - row*(x_spacing/2), height_start + row * y_spacing, post_color, 0.0, 0.0, true);
        }
        
        row_posts++;        
    }
    
}

enum sim_scenario {
    SCENARIO_EMPTY,
    SCENARIO_RANDOM,
    SCENARIO_CLOTH,
    SCENARIO_PLINKO,
    SCENARIO_POOL,

    SCENARIO_COUNT
};
const char * SCENARIO_NAMES[SCENARIO_COUNT] = {"Empty", "Random", "Cloth", "Plinko", "Pool"};

verlet_sim_t * new_simulation(enum sim_scenario scenario){
    verlet_sim_t *sim = init_simulation(SQUARE, WINDOW_WIDTH/2, WINDOW_HEIGHT/2, WINDOW_HEIGHT/2, WINDOW_WIDTH, WINDOW_HEIGHT, GRAV_X, GRAV_Y);
    assert(sim);

    switch (scenario)
    {
    case SCENARIO_RANDOM:
        spawn_random_circles(sim, 30000, 2*sim_get_constraint_radius(sim), 2*sim_get_constraint_radius(sim));
        break;
    
    case SCENARIO_CLOTH:
        spawn_cloth(sim, 2*CIRCLE_RADIUS, 25, 1);
        break;

    case SCENARIO_PLINKO:
        spawn_plinko(sim, 20, 7);
        break;

    case SCENARIO_POOL:
        vector gravity = {.x = 0, .y = 0};
        sim_set_gravity(sim, gravity);
        spawn_billard_triangle(sim, WINDOW_WIDTH/2, WINDOW_HEIGHT/2, 11, 6);
        break;
    default:
        break;
    }

    return sim;
}



int main(int argc, char* argv[]) {

    struct gui *gui = init_gui();
    verlet_sim_t *sim = new_simulation(SCENARIO_EMPTY);

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
    bool button_mousedown_left = false;
    bool button_mousedown_right = false;
    verlet_circle *selected_circle = NULL;
    float zoom_level = 1.0;
    float view_offset_x = 0.0;
    float view_offset_y = 0.0;
    bool gui_spawn_player_ball = false;
    bool pause_sim = false;
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
        static int gui_circle_radius = CIRCLE_RADIUS;

        while(SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (ioptr->WantCaptureMouse) continue;
            switch(event.type)
            {

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        button_mousedown_left = true;
                        int x,y;
                        SDL_GetMouseState( &x, &y );
                        // add_circle(sim, 4+(rand()%(CIRCLE_RADIUS-4)), x, y, rainbow_color(sim->circle_count), 0, 0);
                    }else if (event.button.button == SDL_BUTTON_RIGHT){
                        button_mousedown_right = true;
                        int x,y;
                        SDL_GetMouseState( &x, &y );
                        selected_circle = sim_get_circle_at_coord(sim, x*1.0, y*1.0);
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        button_mousedown_left = false;
                    }else if (event.button.button == SDL_BUTTON_RIGHT){
                        button_mousedown_right = false;
                    }
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
                    case SDLK_p:
                        int x,y;
                        SDL_GetMouseState( &x, &y );
                        verlet_circle *c = sim_get_circle_at_coord(sim, x*1.0, y*1.0);
                        if (c){
                            c->pinned = !c->pinned;
                        };
                        continue;
                    
                    case SDLK_SPACE:
                        pause_sim = !pause_sim;
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
        if(button_mousedown_left){
            int x,y;
            SDL_GetMouseState( &x, &y );
            add_circle(sim, gui_circle_radius, x, y, rainbow_color(sim_get_object_count(sim)), 0, 0, false);
            // add_circle(sim, 4+(rand()%(CIRCLE_RADIUS-4)), x, y, rainbow_color(sim_get_object_count(sim)), 0, 0);
        }
        if(button_mousedown_right){
            int x,y;
            SDL_GetMouseState( &x, &y );
            if (selected_circle != NULL){
                selected_circle->position_current.x = x;
                selected_circle->position_old.x = x;
                selected_circle->position_current.y = y;
                selected_circle->position_old.y = y;
            }
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
        if(igSliderInt("Sub Steps", &gui_sub_steps, 0, 100, "%d", 0)){
            sim_set_sub_steps(sim, gui_sub_steps);  
        }
        if(igSliderInt("Thread Count", &gui_thread_count, 0, 32, "%d", 0)){
            sim_set_thread_count(sim, gui_thread_count);  
        }
        igSliderInt("Circle Radius", &gui_circle_radius, 1, 30, "%d", 0);

        ImVec2 buttonSize = {.x = 0, .y = 0};
        float button_spacing = 0.0;
        for (int scenario = 0; scenario < SCENARIO_COUNT; scenario++)
        {
            if (igButton(SCENARIO_NAMES[scenario], buttonSize)){
                destroy_simulation(sim);
                sim = new_simulation(scenario);
                gui_spawn_player_ball = false;
            }
            button_spacing -= 1.0;
            if (scenario < SCENARIO_COUNT - 1) igSameLine(0.0f, button_spacing);
        }
        
        if(igButton("Spawn Mouse Balls", buttonSize) && !gui_spawn_player_ball){
            gui_spawn_player_ball = !gui_spawn_player_ball;
            gui_player_ball_id = sim_get_object_count(sim);
            add_circle(sim, 30, 0, 0, rainbow_color(sim_get_object_count(sim)), 0, 0, true);
        }

        igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);
        igText("Number of objects : %zu", sim_get_object_count(sim) + sim_get_stick_count(sim));
        igEnd();

        // unsigned int start_time_simulation = SDL_GetPerformanceCounter();
        if (!pause_sim){
            update_simulation(sim, 1/60.0);
        }
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

       if(button_mousedown_right){
            int x,y;
            SDL_GetMouseState( &x, &y );
            verlet_circle *c = sim_get_circle_at_coord(sim, x*1.0, y*1.0);
            if (c){
                c->position_current.x = x;
                c->position_current.y = y;
            }
            
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