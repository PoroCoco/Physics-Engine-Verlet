gcc -std=c99 -Wall -O3 src/verlet.c src/graphics.c src/error_handler.c src/circle_verlet.c src/simulation.c src/color.c src/grid.c -o bin/verlet -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lm -pthread -DSQUARE_BOUNDARY=1