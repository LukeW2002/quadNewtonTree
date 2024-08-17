#ifndef POINT_H
#define POINT_H
#include "globals.h"


void freePoint(Point *p);
void freeAllPoints(QuadTree* root);
void createPoint( Point *p,
                 long double x, 
                 long double y, 
                 long double vx, 
                 long double vy, 
                 long double mass,
                 long double fx,
                 long double fy,
                 double r,
                 double g,
                 double b,
                 double a
                 );

#endif
