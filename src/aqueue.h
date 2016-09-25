#ifndef INC_AQUEUE_H
#define INC_AQUEUE_H

#include "coro.h"

struct aqueue
{
    unsigned char *buffer;
    size_t isize, capacity, realsize;
    size_t head, tail;
};

struct aqueue *aqueue_new(size_t isize, size_t capacity);
void aqueue_free(struct aqueue *aqueue);

size_t aqueue_avail(const struct aqueue *aqueue);
int aqueue_empty(const struct aqueue *aqueue);
int aqueue_full(const struct aqueue *aqueue);

void aqueue_push(struct aqueue *aqueue, const void *item);
void aqueue_await_push(struct aqueue *aqueue, struct coro *coro, const void *item);

const void *aqueue_pop(struct aqueue *aqueue);
const void *aqueue_await_pop(struct aqueue *aqueue, struct coro *coro);
void aqueue_unpop(struct aqueue *aqueue);

size_t aqueue_read(struct aqueue *aqueue, int fd, size_t count);

#endif
