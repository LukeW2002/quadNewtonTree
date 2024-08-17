#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#ifndef NO_LOGGING
    #define LOG_DEBUG(fmt, ...) fprintf(stderr, "[DEBUG] %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
    #define LOG_INFO(fmt, ...) fprintf(stderr, "[INFO] %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
    #define LOG_WARNING(fmt, ...) fprintf(stderr, "[WARNING] %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR] %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(fmt, ...)
    #define LOG_INFO(fmt, ...)
    #define LOG_WARNING(fmt, ...)
    #define LOG_ERROR(fmt, ...)
#endif


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


// Additional debug helper functions
void log_point(const char* prefix, double x, double y);
void log_quad_tree(const char* prefix, double x, double y, double width, double height);
void log_memory_usage(void);
void log_opengl_error(const char* file, int line, const char* func);

#endif // DEBUG_H
