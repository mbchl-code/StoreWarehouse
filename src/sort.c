#include "sort.h"
#include <stdlib.h>
#include <string.h>

// Compare two nodes according to the requested sort key
/*
 * Returns a negative value if a < b, zero if equal, positive if a > b.
 * Used by quicksort as the ordering predicate.
 */
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

// Swap the product data stored in two nodes in place
/*
 * Pointer links are not touched; only the Product payloads are exchanged.
 * This keeps the list structure intact while reordering values.
 */
static void swap_data(Node *a, Node *b) {
    Product tmp = a->data;
    a->data = b->data;
    b->data = tmp;
}

// Recursively sort the sublist between lo and hi using Lomuto partition
/*
 * Selects hi as the pivot, partitions the range [lo, hi] so that all
 * elements <= pivot precede it, then recurses on both halves.
 * No break or continue statements are used; loop termination is
 * controlled entirely by pointer advancement and the entry guard.
 */
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

// Reverse the order of all nodes in the list
/*
 * Swaps next and prev pointers of every node, then exchanges
 * list->head and list->tail. Used to implement descending order
 * on top of the ascending quicksort.
 */
static void reverse_list(List *list) {
    Node *cur = list->head;
    while (cur != NULL) {
        Node *next = cur->next;
        cur->next  = cur->prev;
        cur->prev  = next;
        cur        = next;
    }
    Node *tmp  = list->head;
    list->head = list->tail;
    list->tail = tmp;
}

// Sort the entire list by the given key and order
/*
 * Runs quicksort in ascending order, then reverses the list if
 * SORT_DESC was requested. Has no effect on lists with fewer than
 * two elements.
 */
void list_sort(List *list, SortKey key, SortOrder order) {
    if (list->size > 1) {
        quicksort(list->head, list->tail, key);
        if (order == SORT_DESC) {
            reverse_list(list);
        }
    }
}
