#ifndef LSTACK_H
#define LSTACK_H

#include <stdatomic.h>

struct lstack_node {
    void *value;
    struct lstack_node *next;
};

typedef struct {
    _Atomic struct lstack_node *head;
    _Atomic size_t size;
} lstack_t;

inline void lstack_init(lstack_t *lstack)
{
    lstack->head = ATOMIC_VAR_INIT(NULL);
    lstack->size = ATOMIC_VAR_INIT(0);
}

inline size_t lstack_size(lstack_t *lstack)
{
    return atomic_load(&lstack->size);
}

int   lstack_push(lstack_t *lstack, void *value);
void *lstack_pop(lstack_t *lstack);

#endif
