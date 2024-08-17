#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

typedef enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR
} LogLevel;

void init_logger(const char* filename);
void close_logger(void);
void set_log_level(LogLevel level);
void log_message(LogLevel level, const char* file, int line, const char* func, const char* format, ...);

#define LOG_DEBUG(...) log_message(DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...) log_message(INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARNING(...) log_message(WARNING, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...) log_message(ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

// Additional debug helper functions
void log_point(const char* prefix, double x, double y);
void log_quad_tree(const char* prefix, double x, double y, double width, double height);
void log_memory_usage(void);
void log_opengl_error(const char* file, int line, const char* func);

#endif // DEBUG_H
