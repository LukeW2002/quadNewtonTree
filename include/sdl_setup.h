#ifndef SDL_SETUP_H
#define SDL_SETUP_H
#include "globals.h"

bool initSDL();
bool initGlew();
GLuint compileShader(const char* source, GLenum type);
bool initOpenGL();
void cleanup();


#endif
