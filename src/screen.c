#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "screen.h"

struct screen *screen_new(size_t width, size_t height, size_t buffer_lines)
{
    struct screen *screen = malloc(sizeof(struct screen));

    screen->width = width;
    screen->height = height;
    screen->last_line = height - 1;
    screen->buffer_lines = buffer_lines;

    screen->pen.fg = SCREEN_PALETTE_DEFAULT;
    screen->pen.bg = SCREEN_PALETTE_DEFAULT;
    screen->pen.attr = SCREEN_PEN_NORMAL;

    screen->cursor.line = 0;
    screen->cursor.col = 0;

    screen->line_ends = SCREEN_LF;

    screen->lines = malloc(buffer_lines * sizeof(struct screen_line));

    memset(screen->lines, 0, buffer_lines * sizeof(struct screen_line));

    screen_line_realloc(&screen->lines[0], width);

    return screen;
}

void screen_free(struct screen *screen)
{
    if (screen)
    {
        if (screen->lines)
        {
            for (size_t i = 0; i < screen->buffer_lines; i++)
                free(screen->lines[i].cells);

            free(screen->lines);
        }

        free(screen);
    }
}

void screen_line_realloc(struct screen_line *line, size_t length)
{
    struct screen_cell *new_cells = realloc(line->cells, length * sizeof(struct screen_cell));
    line->length = length;
    line->cells = new_cells;
}

static void screen_put_cr(struct screen *screen)
{
    screen->cursor.col = 0;
}

static void screen_put_lf(struct screen *screen)
{
    screen->cursor.line = (screen->cursor.line + 1) % screen->buffer_lines;

    if (screen->line_ends == SCREEN_LF)
        screen_put_cr(screen);

    screen_line_realloc(&screen->lines[screen->cursor.line], screen->cursor.line);
}

void screen_putchar(struct screen *screen, uint32_t codepoint)
{
    struct screen_line *line = &screen->lines[screen->cursor.line];

    switch (codepoint)
    {
        case '\r':
            screen_put_cr(screen);
            break;

        case '\n':
            screen_put_lf(screen);
            break;

        default:
            line->cells[screen->cursor.col] = (struct screen_cell) { codepoint, screen->pen };

            screen->cursor.col++;
            if (screen->cursor.col >= line->length)
                screen_line_realloc(line, line->length + 64);
    }
}
