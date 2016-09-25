#ifndef INC_LOG_H
#define INC_LOG_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

enum log_level
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

void log_start(FILE *file, enum log_level level);
void log_stop(void);

#define putlog(level, ...) _putlog(level, __FILE__, __LINE__, __VA_ARGS__)
#define errlog(level, s) _putlog(level, __FILE__, __LINE__, "%s: %s", s, strerror(errno))
void _putlog(enum log_level level, const char *file, const int line, const char *format, ...);

#endif
