#include "../include/debug.h"
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <GL/glew.h>

static const char* LOG_LEVEL_STRINGS[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
static LogLevel CURRENT_LOG_LEVEL = DEBUG;
static FILE* LOG_FILE = NULL;
static time_t START_TIME;

void init_logger(const char* filename) {
    LOG_FILE = fopen(filename, "a");
    if (LOG_FILE == NULL) {
        fprintf(stderr, "Failed to open log file: %s\n", filename);
        exit(1);
    }
    time(&START_TIME);
    fprintf(LOG_FILE, "\n--- Logging started at %s", ctime(&START_TIME));
    fflush(LOG_FILE);
}

void close_logger(void) {
    if (LOG_FILE != NULL) {
        time_t end_time;
        time(&end_time);
        fprintf(LOG_FILE, "--- Logging ended at %s", ctime(&end_time));
        fclose(LOG_FILE);
        LOG_FILE = NULL;
    }
}

void set_log_level(LogLevel level) {
    CURRENT_LOG_LEVEL = level;
}

void log_message(LogLevel level, const char* file, int line, const char* func, const char* format, ...) {
    if (level < CURRENT_LOG_LEVEL || LOG_FILE == NULL) return;

    time_t now;
    time(&now);
    double elapsed = difftime(now, START_TIME);

    va_list args;
    va_start(args, format);

    fprintf(LOG_FILE, "[%.2f] [%s] %s:%d in %s(): ", 
            elapsed, LOG_LEVEL_STRINGS[level], file, line, func);
    vfprintf(LOG_FILE, format, args);
    fprintf(LOG_FILE, "\n");
    fflush(LOG_FILE);

    va_end(args);
}

void log_point(const char* prefix, double x, double y) {
    LOG_DEBUG("%s: (%.2f, %.2f)", prefix, x, y);
}

void log_quad_tree(const char* prefix, double x, double y, double width, double height) {
    LOG_DEBUG("%s: (%.2f, %.2f) %.2f x %.2f", prefix, x, y, width, height);
}

void log_memory_usage(void) {
    LOG_DEBUG("Memory usage logging not implemented");
}

void log_opengl_error(const char* file, int line, const char* func) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char* error_string;
        switch (err) {
            case GL_INVALID_ENUM:                  error_string = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error_string = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error_string = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error_string = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error_string = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error_string = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error_string = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default:                               error_string = "UNKNOWN"; break;
        }
        log_message(ERROR, file, line, func, "OpenGL error: %s", error_string);
    }
}
