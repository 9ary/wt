#ifndef INC_CORO_H
#define INC_CORO_H

#include <ucontext.h>

struct coro
{
    ucontext_t ucp;
    ucontext_t caller_ucp;
    unsigned char *stack;
    int ended;

    void *data;
};

typedef void (*coro_func_t)(struct coro *self);

struct coro *coro_new(coro_func_t func);
void coro_free(struct coro *coro);
void coro_resume(struct coro *coro);
void coro_yield(struct coro *self);

#endif
