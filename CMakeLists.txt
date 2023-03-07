cmake_minimum_required(VERSION 3.0)

project("Verlet" C)

# # test
# include(CTest)
# enable_testing()



set(CMAKE_C_FLAGS "-std=c99 -Wall -pthread -Wextra -Wunused-parameter -Wstrict-prototypes")
set(CMAKE_C_FLAGS_DEBUG "-g")    # use CMake option: -DCMAKE_BUILD_TYPE=DEBUG
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")   # use CMake option: -DCMAKE_BUILD_TYPE=RELEASE


# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/standardSave.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

############################# CIMGUI ############################
set(BAKENDS_FOLDER "cimgui/imgui/backends/")
set(TABLES_SOURCE "cimgui/imgui/imgui_tables.cpp")

include_directories(cimgui/imgui)
add_definitions("-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1")

include_directories(cimgui)
set(IMGUI_SOURCES 
cimgui/cimgui.cpp 
cimgui/imgui/imgui.cpp 
cimgui/imgui/imgui_draw.cpp 
cimgui/imgui/imgui_demo.cpp 
cimgui/imgui/imgui_widgets.cpp
${TABLES_SOURCE} 
)
add_definitions("-DIMGUI_IMPL_API=extern \"C\" ")
add_compile_definitions("IMGUI_IMPL_OPENGL_LOADER_GL3W")

list(APPEND IMGUI_SOURCES ${BAKENDS_FOLDER}imgui_impl_sdl2.cpp)

#opengl3
list(APPEND IMGUI_SOURCES ${BAKENDS_FOLDER}imgui_impl_opengl3.cpp)
include_directories(cimgui/imgui/examples/libs/gl3w)
list(APPEND IMGUI_LIBRARIES GL)


###
add_library(cimgui_sdl SHARED ${IMGUI_SOURCES})


############################# SDL2 ############################

## use a CMake3 package to solve SDL2 dependencies
find_package(SDL2 REQUIRED)
set(SDL2_ALL_INC ${SDL2_INCLUDE_DIRS})
set(SDL2_ALL_LIBS ${SDL2_LIBRARIES} ) 

## print this 
message(STATUS "SDL2 include dir: ${SDL2_ALL_INC}")
message(STATUS "SDL2 all libraries: ${SDL2_ALL_LIBS}")

include_directories(${SDL2_ALL_INC})
include_directories(./src/simulation)
include_directories(./src/graphic)

############################# SRC #############################


add_executable(verlet_circle src/simulation/circle_verlet.c src/graphic/verlet.c src/graphic/graphics.c src/simulation/error_handler.c)

target_compile_definitions(verlet_circle PUBLIC -DCIMGUI_USE_OPENGL3 -DCIMGUI_USE_SDL2)
target_link_libraries(verlet_circle ${IMGUI_SDL_LIBRARY} cimgui_sdl)


add_library(verlet src/simulation/error_handler.c src/simulation/circle_verlet.c src/simulation/simulation.c src/simulation/color.c src/simulation/grid.c src/simulation/save.c)

target_link_libraries(verlet m pthread)
target_link_libraries(verlet_circle ${SDL2_ALL_LIBS} m verlet)


############################ TESTS TMP ############################

add_executable(verlet_test test/test.c)
target_link_libraries(verlet_test verlet)

# ############################# TEST V1 #############################

# # Aux Tests(game_aux.h)
# add_test(testv1_default ./game_test "default")
# add_test(testv1_default_solution ./game_test "default_solution")
# add_test(testv1_print ./game_test "print")

# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/badSave.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# EOF
