#include <stdarg.h>

#include "log.h"

static enum log_level min_level;
static FILE *log_file = NULL;

static const char *log_colors[] =
{
    "\x1b[34m",
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[31m"
};
static const char *log_color_reset = "\x1b[0m";

void log_start(FILE *file, enum log_level level)
{
    log_file = file;
    min_level = level;
}

void log_stop(void)
{
    log_file = NULL;
}

void _putlog(enum log_level level, const char *file, const int line, const char *format, ...)
{
    if (level < min_level || log_file == NULL)
        return;

    va_list args;
    va_start(args, format);

    fprintf(log_file, "%s[%s:%d] ", log_colors[level], file, line);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
    vfprintf(log_file, format, args);
#pragma clang diagnostic pop
    fprintf(log_file, "%s\n", log_color_reset);
}
