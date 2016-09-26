#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "unicode.h"

static void utf8_to_utf32(struct coro *self) __attribute__((noreturn));
static void utf8_to_utf32(struct coro *self)
{
    struct unidecode iface = *(struct unidecode *) self->data;

    while (1)
    {
        const uint8_t *in;
        uint32_t out;

        // Single byte character, part of the ASCII subset
        in = aqueue_await_pop(iface.read_queue, self);
        if (*in < 0x80)
        {
            out = *in;
            aqueue_await_push(iface.out_queue, self, &out);
            continue;
        }

        // Multibyte characters
        // The first byte can be in one of three ranges, determining the length of the sequence.
        // The second byte is in a subrange of 0x80-0xBF, depending on the first byte.
        // The third and fourth bytes are always in the range 0x80-0xBF.
        // If at any point we encounter a byte which is out of its assigned range, we've reached an
        // unconvertible offset. The part we've already decoded is the maximal subpart of the
        // malformed sequence, so we consume it and emit a replacement character instead.
        int len;
        if (*in >= 0xC2 && *in <= 0xDF)
        {
            out = *in & 0x1F;
            len = 2;
        }
        else if (*in >= 0xE0 && *in <= 0xEF)
        {
            out = *in & 0x0F;
            len = 3;
        }
        else if (*in >= 0xF0 && *in <= 0xF4)
        {
            out = *in & 0x07;
            len = 4;
        }
        else
        {
            goto malformed;
        }

        const uint8_t *next = aqueue_await_pop(iface.read_queue, self);
        aqueue_unpop(iface.read_queue); // This is safe here
        // Clamp second byte ranges based on first byte
        if ((*in == 0xE0 && *next < 0xA0)
                || (*in == 0xED && *next > 0x9F)
                || (*in == 0xF0 && *next < 0x90)
                || (*in == 0xF4 && *next > 0x8F))
            goto malformed;

        // Decode the rest
        for (int i = 1; i < len; i++)
        {
            in = aqueue_await_pop(iface.read_queue, self);
            out <<= 6;
            if (*in < 0x80 || *in > 0xBF)
            {
                aqueue_unpop(iface.read_queue);
                goto malformed;
            }
            out |= *in & 0x3F;
        }

        aqueue_await_push(iface.out_queue, self, &out);
        continue;

malformed:
        // U+FFFD is the replacement character for malformed sequences
        out = 0xFFFD;
        aqueue_await_push(iface.out_queue, self, &out);
    }
}

struct unidecode *unidecode_new(size_t bufsize)
{
    struct unidecode *ret = malloc(sizeof(struct unidecode));
    ret->coro = coro_new(&utf8_to_utf32);
    ret->read_queue= aqueue_new(sizeof(uint8_t), bufsize);
    ret->out_queue= aqueue_new(sizeof(uint32_t), bufsize);
    ret->coro->data = ret;

    return ret;
}

void unidecode_free(struct unidecode *unidecode)
{
    aqueue_free(unidecode->out_queue);
    aqueue_free(unidecode->read_queue);
    coro_free(unidecode->coro);
    free(unidecode);
}

int utf32_to_utf8(uint32_t in, uint8_t *out)
{
    int len;
    if (in < 0x80)
        len = 1;
    else if (in < 0x800)
        len = 2;
    else if (in < 0x10000)
        len = 3;
    else if (in < 0x200000)
        len = 4;
    else
        return 0;

    for (int i = len - 1; i > 0; i--)
    {
        out[i] = 0x80 | (in & 0x3F);
        in >>= 6;
    }

    switch (len)
    {
        case 1:
            out[0] = 0x00 | (in & 0x7F);
            break;
        case 2:
            out[0] = 0xC0 | (in & 0x1F);
            break;
        case 3:
            out[0] = 0xE0 | (in & 0x0F);
            break;
        case 4:
            out[0] = 0xF0 | (in & 0x07);
            break;
    }

    return len;
}
