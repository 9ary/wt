#ifndef INC_SCREEN_H
#define INC_SCREEN_H

#include <stdint.h>

struct screen_pen
{
    // Colors and palettes (base, intense, 256, RGB888)
    uint32_t fg, bg;
    unsigned int fgp : 2, bgp : 2;

    unsigned int bold : 1,
                 italics : 1,
                 underline : 2, // Doubly underlined text is supported
                 strike_through : 1,
                 overline : 1,
                 negative : 1,
                 conceal : 1,
                 blink : 2, // Slow and fast
                 font : 4; // Up to 10 fonts

    /*
     * Unsupported features from ECMA-48
     *
     * Faint color
     * Fraktur (Gothic)
     * Framed/circled
     * Ideogram properties
     */
};

struct screen_cell
{
    uint32_t c;
    struct screen_pen pen;
};

struct screen_line
{
    unsigned int length;
    struct screen_cell *cells;
};

struct screen
{
    unsigned int width, height;
    unsigned int first_line;
    unsigned int buffer_lines;
    struct screen_pen pen;
    struct screen_line *lines;
};

struct screen *screen_new(unsigned int width, unsigned int height, unsigned int buffer_lines);
void free_screen(struct screen *screen);
int screen_line_realloc(struct screen_line *line, unsigned int length);

#endif
