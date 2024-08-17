#include "../include/globals.h"

bool show_grid = false;
SDL_Window *window = NULL;
SDL_GLContext glContext = NULL;
GLuint shaderProgram = 0, vao = 0, vbo = 0;
GLuint instanceVBO = 0;
GLuint pointVAO = 0, gridVAO = 0;
GLuint gridVBO = 0;
