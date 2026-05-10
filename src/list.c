#include "list.h"
#include <stdlib.h>
#include <string.h>

void list_init(List *list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

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

void list_edit(Node *node, double price, int quantity, const char *model) {
    if (node != NULL) {
        node->data.price    = price;
        node->data.quantity = quantity;
        strncpy(node->data.model, model, MAX_STR - 1);
        node->data.model[MAX_STR - 1] = '\0';
    }
}
