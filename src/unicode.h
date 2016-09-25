#ifndef INC_UNICODE_H
#define INC_UNICODE_H

#include <stdint.h>

#include "aqueue.h"
#include "coro.h"

struct unidecode
{
    struct coro *coro;
    struct aqueue *read_queue;
    struct aqueue *out_queue;
};

struct unidecode *unidecode_new(size_t bufsize);
void unidecode_free(struct unidecode *unidecode);

int utf32_to_utf8(uint32_t in, uint8_t *out);

#endif
