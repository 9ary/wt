#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aqueue.h"
#include "misc.h"

struct aqueue *aqueue_new(size_t isize, size_t capacity)
{
    struct aqueue *ret = malloc(sizeof(struct aqueue));
    ret->realsize = capacity + 1;
    ret->buffer = calloc(ret->realsize, isize);
    ret->isize = isize;
    ret->capacity = capacity;
    ret->head = 0;
    ret->tail = 0;

    return ret;
}

void aqueue_free(struct aqueue *aqueue)
{
    free(aqueue->buffer);
    free(aqueue);
}

size_t aqueue_avail(const struct aqueue *aqueue)
{
    size_t ret;
    if (aqueue->tail >= aqueue->head)
        ret = aqueue->capacity - (aqueue->tail - aqueue->head);
    else
        ret = aqueue->head - aqueue->tail - 1;
    return ret;
}

int aqueue_empty(const struct aqueue *aqueue)
{
    return aqueue_avail(aqueue) == aqueue->capacity;
}

int aqueue_full(const struct aqueue *aqueue)
{
    return aqueue_avail(aqueue) == 0;
}

void aqueue_push(struct aqueue *aqueue, const void *item)
{
    assert(aqueue_full(aqueue) == 0);

    memcpy(aqueue->buffer + aqueue->isize * aqueue->tail, item, aqueue->isize);
    aqueue->tail = (aqueue->tail + 1) % aqueue->realsize;
}

void aqueue_await_push(struct aqueue *aqueue, struct coro *coro, const void *item)
{
    while (aqueue_full(aqueue))
        coro_yield(coro);

    aqueue_push(aqueue, item);
}

const void *aqueue_pop(struct aqueue *aqueue)
{
    assert(aqueue_empty(aqueue) == 0);

    const void *ret = aqueue->buffer + aqueue->isize * aqueue->head;
    aqueue->head = (aqueue->head + 1) % aqueue->realsize;
    return ret;
}

const void *aqueue_await_pop(struct aqueue *aqueue, struct coro *coro)
{
    while (aqueue_empty(aqueue))
        coro_yield(coro);

    return aqueue_pop(aqueue);
}

void aqueue_unpop(struct aqueue *aqueue)
{
    assert(aqueue_full(aqueue) == 0);

    aqueue->head--;
    if (aqueue->head > aqueue->capacity)
        aqueue->head = aqueue->capacity;
}

static size_t aqueue_read_partial(struct aqueue *aqueue, int fd, size_t count)
{
    ssize_t bytes_read = read(fd, aqueue->buffer + aqueue->tail, count);
    if (bytes_read > 0)
    {
        aqueue->tail = (aqueue->tail + (size_t) bytes_read) % aqueue->realsize;
        return (size_t) bytes_read;
    }

    return 0;
}

size_t aqueue_read(struct aqueue *aqueue, int fd, size_t count)
{
    // Only use for byte streams
    assert(aqueue->isize == 1);

    count = min(count, aqueue_avail(aqueue));
    size_t ret = 0;

    // Don't read further than the end of the buffer
    size_t bytes_to_read = min(count, aqueue->realsize - aqueue->tail);
    if (bytes_to_read)
        ret += aqueue_read_partial(aqueue, fd, bytes_to_read);

    // Wrap around if needed
    if ((ret == bytes_to_read) && (bytes_to_read != count))
        ret += aqueue_read_partial(aqueue, fd, count - bytes_to_read);

    return ret;
}
