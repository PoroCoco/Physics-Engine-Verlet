#ifndef _COLOR_H__
#define _COLOR_H__

typedef struct color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} color_t;

color_t rainbow_color(float t);
color_t random_color(void);

#endif // _COLOR_H__
