#include "list.h"
#include <stdlib.h>
#include <string.h>

// Initialize an empty doubly-linked list
/*
 * Sets head and tail to NULL and size to zero.
 * Must be called before any other list operation.
 */
void list_init(List *list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

// Free all nodes and reset the list to empty state
/*
 * Traverses the list from head to tail, freeing each node.
 * After this call the list is valid but empty.
 */
void list_clear(List *list) {
    Node *cur = list->head;
    while (cur != NULL) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

// Add a new product to the tail of the list
/*
 * Allocates a new node and copies the product data into it.
 * Returns 1 on success, 0 if a product with the same code already exists
 * or if memory allocation fails.
 */
int list_add(List *list, const Product *p) {
    Node *existing = list_find_by_code(list, p->code);
    int result = 0;
    if (existing == NULL) {
        Node *node = malloc(sizeof(Node));
        if (node != NULL) {
            node->data = *p;
            node->next = NULL;
            node->prev = list->tail;
            if (list->tail != NULL) {
                list->tail->next = node;
            } else {
                list->head = node;
            }
            list->tail = node;
            list->size++;
            result = 1;
        }
    }
    return result;
}

// Remove the node with the given product code from the list
/*
 * Finds the node by code, unlinks it from its neighbours,
 * frees its memory and decrements the list size.
 * Returns 1 if the node was found and removed, 0 otherwise.
 */
int list_delete(List *list, int code) {
    Node *node = list_find_by_code(list, code);
    int result = 0;
    if (node != NULL) {
        if (node->prev != NULL) {
            node->prev->next = node->next;
        } else {
            list->head = node->next;
        }
        if (node->next != NULL) {
            node->next->prev = node->prev;
        } else {
            list->tail = node->prev;
        }
        free(node);
        list->size--;
        result = 1;
    }
    return result;
}

// Search the list for a node whose product code matches the given value
/*
 * Performs a linear scan from head to tail.
 * Returns a pointer to the matching node, or NULL if not found.
 */
Node *list_find_by_code(const List *list, int code) {
    Node *cur = list->head;
    Node *found = NULL;
    while (cur != NULL && found == NULL) {
        if (cur->data.code == code) {
            found = cur;
        }
        cur = cur->next;
    }
    return found;
}

// Update the editable fields of an existing product node
/*
 * Only price, quantity and model may be changed after creation.
 * The node pointer must not be NULL.
 */
void list_edit(Node *node, double price, int quantity, const char *model) {
    if (node != NULL) {
        node->data.price    = price;
        node->data.quantity = quantity;
        strncpy(node->data.model, model, MAX_STR - 1);
        node->data.model[MAX_STR - 1] = '\0';
    }
}
