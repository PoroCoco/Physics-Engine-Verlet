#include <math.h>
#include "sticks.h"



void stick_update(stick *s){
    // printf("starting update\n");
    float dx = s->p1->position_current.x - s->p0->position_current.x;
    float dy = s->p1->position_current.y - s->p0->position_current.y;
    // printf("compute sqrt\n");
    float distance = sqrtf(dx*dx + dy*dy);
    // printf("compute other\n");
    float diff = s->length - distance;
    float percent = (diff / distance) / 2.0;

    // printf("compute offsets\n");
    float offset_x = dx * percent;
    float offset_y = dy * percent;

    // printf("ptr are %p, %p\n", s->p0, s->p1 );
    // printf("updating pos %lf\n", s->p0->position_current.x );
    if (!s->p0->pinned){
        s->p0->position_current.x -= offset_x;
        s->p0->position_current.y -= offset_y;
    }

    if (!s->p1->pinned){
        s->p1->position_current.x += offset_x;
        s->p1->position_current.y += offset_y;
    }
}
