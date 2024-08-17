#ifndef RENDER_H
#define RENDER_H
#include "globals.h"
void collectGridLines(QuadTree *qt, float **lines, int *size, int *capacity);
void updateInstanceData(QuadTree *qt, float **instanceData, int *dataSize, int *capacity);

void collectQuadTreeData(QuadTree *qt);
void renderQuadTreeData();
void drawQuadTree(QuadTree *qt);
void checkOpenGLState();
#endif
