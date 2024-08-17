#ifndef GLOBALS_H
#define GLOBALS_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <math.h>

#include "debug.h"

#define _MAX_POINTS 1
#define _TOTAL_MEMORY_POINTS 10000
#define _WINDOW_WIDTH 800
#define _WINDOW_HEIGHT 600
#define _G 0.01
#define _SCALE_FACTOR 1
#define _TIME_STEP 0.0001
#define _AMOUNT_OF_BALLS 100
#define _SOFTENING 10

#define _RELATIVE_WIDTH _WINDOW_WIDTH * _SCALE_FACTOR
#define _RELATIVE_HEIGHT _WINDOW_HEIGHT * _SCALE_FACTOR  
#define _CALCS_PER_FRAME 10

#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION

typedef struct Point {
	long double x, y;
	long double vx, vy;
	long double mass;
	long double fx, fy;
	double r, g, b, a;
} Point;

typedef struct QuadTree {
	double x, y, width, height;
	struct QuadTree* children[4];
	Point* points[_TOTAL_MEMORY_POINTS];
	int point_count;

	double total_mass;
	double cmX, cmY;
} QuadTree;

extern bool show_grid;

extern SDL_Window *window;

extern SDL_GLContext glContext;
extern GLuint shaderProgram, vao, vbo;
extern GLuint instanceVBO;
extern GLuint pointVAO, gridVAO;
extern GLuint gridVBO;
#endif
