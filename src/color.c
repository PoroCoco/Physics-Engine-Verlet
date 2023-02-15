#include "color.h"
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846

color_t rainbow_color(float t){
    float r = sin(t);
    float g = sin(t + 0.33f * 2.0f * PI);
    float b = sin(t + 0.66f * 2.0f * PI);
    color_t c = {
        .r = (255.0 * r * r),
        .g = (255.0 * g * g),
        .b = (255.0 * b * b),
    };
    return c;
}

color_t random_color(void){
    color_t c = {
        c.r = rand()%255,
        c.g = rand()%255,
        c.b = rand()%255
    };
    return c;
}

