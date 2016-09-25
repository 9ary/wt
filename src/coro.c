#include <limits.h>
#include <stdlib.h>

#include "coro.h"

enum
{
    CORO_STACK_SIZE = PTHREAD_STACK_MIN
};

static void coro_wrapper(struct coro *self, coro_func_t func)
{
    func(self);
    self->ended = 1;
    coro_yield(self);
}

struct coro *coro_new(coro_func_t func)
{
    struct coro *ret = malloc(sizeof(struct coro));

    ret->ended = 0;

    unsigned char *stack = malloc(CORO_STACK_SIZE);
    ret->stack = stack;

    getcontext(&ret->ucp);
    ret->ucp.uc_link = NULL;
    ret->ucp.uc_stack = (stack_t)
    {
        .ss_sp = stack,
        .ss_size = CORO_STACK_SIZE,
        .ss_flags = 0
    };
    makecontext(&ret->ucp, (void (*)(void)) &coro_wrapper, 2, ret, func);

    return ret;
}

void coro_free(struct coro *coro)
{
    free(coro->stack);
    free(coro);
}

void coro_resume(struct coro *coro)
{
    swapcontext(&coro->caller_ucp, &coro->ucp);
}

void coro_yield(struct coro *self)
{
    swapcontext(&self->ucp, &self->caller_ucp);
}
