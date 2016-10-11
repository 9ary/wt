#ifndef INC_SCREEN_H
#define INC_SCREEN_H

#include <stdint.h>
#include <stdlib.h>

enum screen_pen_attr
{
    SCREEN_PEN_NORMAL           = 0,
    SCREEN_PEN_BOLD             = 1 << 0,
    SCREEN_PEN_ITALICS          = 1 << 1,
    SCREEN_PEN_UNDERLINE        = 1 << 2,
    SCREEN_PEN_UNDERLINE_DOUBLE = 1 << 3,
    SCREEN_PEN_STRIKE_THROUGH   = 1 << 4,
    SCREEN_PEN_OVERLINE         = 1 << 5,
    SCREEN_PEN_NEGATIVE         = 1 << 6,
    SCREEN_PEN_CONCEAL          = 1 << 7,
    SCREEN_PEN_BLINK            = 1 << 8,
    SCREEN_PEN_BLINK_FAST       = 1 << 9,
    SCREEN_PEN_FONT_MASK        = 0xF << 10

    /*
     * Unsupported features from ECMA-48
     *
     * Fraktur (Gothic)
     * Framed/circled
     * Ideogram properties
     */
};

enum screen_palette
{
    SCREEN_PALETTE_DEFAULT = 0 << 24,
    SCREEN_PALETTE_BASE    = 1 << 24,
    SCREEN_PALETTE_INTENSE = 2 << 24,
    SCREEN_PALETTE_FAINT   = 3 << 24,
    SCREEN_PALETTE_256     = 4 << 24,
    SCREEN_PALETTE_RGB     = 5 << 24,
    SCREEN_PALETTE_MASK    = 0x7F << 24
};

enum screen_line_ends
{
    SCREEN_LF,
    SCREEN_CRLF
};

struct screen_pen
{
    uint32_t fg, bg; // Colors
    int attr;
};

struct screen_cell
{
    uint32_t c;
    struct screen_pen pen;
};

struct screen_line
{
    size_t length;
    struct screen_cell *cells;
};

struct screen_cursor
{
    size_t line, col;
};

struct screen
{
    size_t width, height;
    size_t last_line;
    size_t buffer_lines;
    struct screen_pen pen;
    struct screen_cursor cursor;
    enum screen_line_ends line_ends;
    struct screen_line *lines;
};

struct screen *screen_new(size_t width, size_t height, size_t buffer_lines);
void screen_free(struct screen *screen);
void screen_line_realloc(struct screen_line *line, size_t length);
void screen_putchar(struct screen *screen, uint32_t codepoint);

#endif
