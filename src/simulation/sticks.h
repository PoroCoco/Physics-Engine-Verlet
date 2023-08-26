#ifndef _STICKS_H__
#define _STICKS_H__


#include "circle_verlet.h"


typedef struct stick{
    verlet_circle * p0;
    verlet_circle * p1;
    float length;
} stick;


void stick_update(stick *s);


#endif // _STICKS_H__
