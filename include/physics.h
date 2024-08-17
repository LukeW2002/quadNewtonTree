#ifndef PHYSICS_H
#define PHYSICS_H
#include "globals.h"

void calcForce(Point *p, QuadTree *qt, double theta);
void updatePos(QuadTree *qt);

#endif
