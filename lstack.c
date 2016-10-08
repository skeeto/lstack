#include <stdlib.h>
#include <errno.h>
#include "lstack.h"

int lstack_init(lstack_t *lstack, size_t max_size)
{
    struct lstack_head head_init = {0, NULL};
    lstack->head = ATOMIC_VAR_INIT(head_init);
    lstack->size = ATOMIC_VAR_INIT(0);

    /* Pre-allocate all nodes. */
    lstack->node_buffer = malloc(max_size * sizeof(struct lstack_node));
    if (lstack->node_buffer == NULL)
        return ENOMEM;
    for (size_t i = 0; i < max_size - 1; i++)
        lstack->node_buffer[i].next = lstack->node_buffer + i + 1;
    lstack->node_buffer[max_size - 1].next = NULL;
    struct lstack_head free_init = {0, lstack->node_buffer};
    lstack->free = ATOMIC_VAR_INIT(free_init);
    return 0;
}

static struct lstack_node *pop(_Atomic struct lstack_head *head)
{
    struct lstack_head next, orig = atomic_load(head);
    do {
        if (orig.node == NULL)
            return NULL;
        next.aba = orig.aba + 1;
        next.node = orig.node->next;
    } while (!atomic_compare_exchange_weak(head, &orig, next));
    return orig.node;
}

static void push(_Atomic struct lstack_head *head, struct lstack_node *node)
{
    struct lstack_head next, orig = atomic_load(head);
    do {
        node->next = orig.node;
        next.aba = orig.aba + 1;
        next.node = node;
    } while (!atomic_compare_exchange_weak(head, &orig, next));
}

int lstack_push(lstack_t *lstack, void *value)
{
    struct lstack_node *node = pop(&lstack->free);
    if (node == NULL)
        return ENOMEM;
    node->value = value;
    push(&lstack->head, node);
    atomic_fetch_add(&lstack->size, 1);
    return 0;
}

void *lstack_pop(lstack_t *lstack)
{
    struct lstack_node *node = pop(&lstack->head);
    if (node == NULL)
        return NULL;
    atomic_fetch_sub(&lstack->size, 1);
    void *value = node->value;
    push(&lstack->free, node);
    return value;
}
