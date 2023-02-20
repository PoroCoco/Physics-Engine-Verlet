cmake_minimum_required(VERSION 3.0)

project("Verlet" C)

# # test
# include(CTest)
# enable_testing()



set(CMAKE_C_FLAGS "-std=c99 -Wall -pthread -Wextra -Wunused-parameter -Wstrict-prototypes")
set(CMAKE_C_FLAGS_DEBUG "-g")    # use CMake option: -DCMAKE_BUILD_TYPE=DEBUG
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")   # use CMake option: -DCMAKE_BUILD_TYPE=RELEASE

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
