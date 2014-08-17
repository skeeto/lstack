#include <stdlib.h>
#include <errno.h>
#include "lstack.h"

int lstack_push(lstack_t *lstack, void *value)
{
    struct lstack_node *node = malloc(sizeof(struct lstack_node));
    if (node == NULL)
        return ENOMEM;
    node->value = value;
    node->next = (struct lstack_node *) atomic_load(&lstack->head);
    while (!atomic_compare_exchange_weak(&lstack->head, &node->next, node));
    atomic_fetch_add(&lstack->size, 1);
    return 0;
}

void *lstack_pop(lstack_t *lstack)
{
    struct lstack_node *node, *head = atomic_load(&lstack->head);
    do {
        if (head == NULL)
            return NULL;
        node = head;
    } while (atomic_compare_exchange_weak(&lstack->head, &head, head->next));
    atomic_fetch_sub(&lstack->size, 1);
    void *value = node->value;
    free(node);
    return value;
}
