#ifndef QUAD_TREE_H
#define QUAD_TREE_H
#include "globals.h"

QuadTree * createQuadTree( double x, double y, double width, double height);
bool isInBounds(QuadTree *qt, Point *p);
void subdivide(QuadTree *qt);
void freeQuadTree(QuadTree* qt);
void clearQuadTree(QuadTree* qt);
void insert(QuadTree *qt, Point *p);

#endif
