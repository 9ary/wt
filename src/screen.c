#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "screen.h"

struct screen *screen_new(unsigned int width, unsigned int height, unsigned int buffer_lines)
{
    struct screen *screen = malloc(sizeof(struct screen));

    screen->width = width;
    screen->height = height;
    screen->first_line = 0;
    screen->buffer_lines = buffer_lines;

    memset(&(screen->pen), 0, sizeof(struct screen_pen));
    screen->pen.fg = (unsigned) -1;
    screen->pen.bg = (unsigned) -1;

    screen->lines = malloc(buffer_lines * sizeof(struct screen_line));

    memset(screen->lines, 0, buffer_lines * sizeof(struct screen_line));

    for (unsigned int i = 0; i < height; i++)
        screen_line_realloc(&(screen->lines[i]), width);

    return screen;
}

void free_screen(struct screen *screen)
{
    if (screen)
    {
        if (screen->lines)
        {
            for (unsigned int i = 0; i < screen->buffer_lines; i++)
                free(screen->lines[i].cells);

            free(screen->lines);
        }

        free(screen);
    }
}

void screen_line_realloc(struct screen_line *line, unsigned int length)
{
    struct screen_cell *new_cells = realloc(line->cells, length * sizeof(struct screen_cell));
    line->length = length;
    line->cells = new_cells;
}
