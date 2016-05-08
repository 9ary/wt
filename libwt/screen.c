#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "screen.h"

struct screen *screen_new(int width, int height, int buffer_lines)
{
    struct screen *screen = malloc(sizeof(struct screen));
    if (screen == NULL)
    {
        perror("malloc");
        goto fail;
    }

    screen->width = width;
    screen->height = height;
    screen->first_line = 0;
    screen->buffer_lines = buffer_lines;

    screen->lines = malloc(buffer_lines * sizeof(struct screen_line));
    if (screen->lines == NULL)
    {
        perror("malloc");
        goto free_screen;
    }

    memset(screen->lines, 0, buffer_lines * sizeof(struct screen_line));

    for (int i = 0; i < height; i++)
    {
        int rc = screen_line_realloc(&(screen->lines[i]), width);
        if (rc < 0)
            goto free_screen;
    }

    return screen;

free_screen:
    free_screen(screen);
fail:
    return NULL;
}

void free_screen(struct screen *screen)
{
    if (screen)
        if (screen->lines)
        {
            for (int i = 0; i < screen->buffer_lines; i++)
                free(screen->lines[i].cells);

            free(screen->lines);
        }

    free(screen);
}

int screen_line_realloc(struct screen_line *line, int length)
{
    struct screen_cell *new_cells = realloc(line->cells, length * sizeof(struct screen_cell));
    if (new_cells == NULL)
    {
        perror("realloc");
        goto fail;
    }

    line->length = length;

    return 0;

fail:
    return -1;
}

