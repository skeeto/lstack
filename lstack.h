#ifndef LSTACK_H
#define LSTACK_H

#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>

struct lstack_node {
    void *value;
    struct lstack_node *next;
};

struct lstack_head {
    uintptr_t aba;
    struct lstack_node *node;
};

typedef struct {
    struct lstack_node *node_list;
    _Atomic struct lstack_head head, free;
    _Atomic size_t size;
} lstack_t;

inline size_t lstack_size(lstack_t *lstack)
{
    return atomic_load(&lstack->size);
}

inline void lstack_free(lstack_t *lstack)
{
    free(lstack->node_list);
}

int   lstack_init(lstack_t *lstack, size_t size);
int   lstack_push(lstack_t *lstack, void *value);
void *lstack_pop(lstack_t *lstack);

#endif
