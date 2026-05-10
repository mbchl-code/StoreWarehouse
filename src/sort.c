#include "sort.h"
#include <stdlib.h>
#include <string.h>

static int compare_nodes(const Node *a, const Node *b, SortKey key) {
    int result = 0;
    if (key == SORT_BY_CODE) {
        result = a->data.code - b->data.code;
    } else if (key == SORT_BY_NAME) {
        result = strncmp(a->data.name, b->data.name, MAX_STR);
    } else {
        if (a->data.price < b->data.price) {
            result = -1;
        } else if (a->data.price > b->data.price) {
            result = 1;
        }
    }
    return result;
}

static void swap_data(Node *a, Node *b) {
    Product tmp = a->data;
    a->data = b->data;
    b->data = tmp;
}

static void quicksort(Node *lo, Node *hi, SortKey key) {
    if (lo != NULL && hi != NULL && lo != hi && lo != hi->next) {
        Node *pivot = hi;
        Node *i     = lo->prev;
        Node *j     = lo;

        while (j != hi) {
            if (compare_nodes(j, pivot, key) <= 0) {
                i = (i == NULL) ? lo : i->next;
                swap_data(i, j);
            }
            j = j->next;
        }

        i = (i == NULL) ? lo : i->next;
        swap_data(i, hi);

        Node *part = i;
        if (part != NULL && part->prev != lo->prev) {
            quicksort(lo, part->prev, key);
        }
        quicksort(part->next, hi, key);
    }
}

void list_sort(List *list, SortKey key) {
    if (list->size > 1) {
        quicksort(list->head, list->tail, key);
    }
}
