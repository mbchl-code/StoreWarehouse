#include "test_list.h"
#include "list.h"
#include <string.h>
#include <stdio.h>

static Product make(int code, const char *name, double price, int qty) {
    Product p = {0};
    p.code     = code;
    p.price    = price;
    p.quantity = qty;
    strncpy(p.group, "Группа",  MAX_STR - 1);
    strncpy(p.name,  name,      MAX_STR - 1);
    strncpy(p.model, "Модель",  MAX_STR - 1);
    return p;
}

int test_list_add_and_find(void) {
    List list;
    list_init(&list);
    Product p = make(1, "Товар А", 100.0, 10);
    int ok = list_add(&list, &p);
    int pass = (ok == 1 && list.size == 1);
    Node *found = list_find_by_code(&list, 1);
    pass = pass && (found != NULL) && (found->data.code == 1);
    list_clear(&list);
    return pass;
}

int test_list_duplicate_code(void) {
    List list;
    list_init(&list);
    Product p = make(42, "Дубль", 50.0, 5);
    list_add(&list, &p);
    int second = list_add(&list, &p);
    int pass = (second == 0 && list.size == 1);
    list_clear(&list);
    return pass;
}

int test_list_delete(void) {
    List list;
    list_init(&list);
    Product p1 = make(1, "А", 1.0, 1);
    Product p2 = make(2, "Б", 2.0, 2);
    list_add(&list, &p1);
    list_add(&list, &p2);
    int del = list_delete(&list, 1);
    int pass = (del == 1 && list.size == 1 && list_find_by_code(&list, 1) == NULL);
    list_clear(&list);
    return pass;
}

int test_list_edit(void) {
    List list;
    list_init(&list);
    Product p = make(7, "Товар", 10.0, 3);
    list_add(&list, &p);
    Node *node = list_find_by_code(&list, 7);
    list_edit(node, 99.99, 50, "НоваяМодель");
    int pass = (node->data.price == 99.99 &&
                node->data.quantity == 50 &&
                strcmp(node->data.model, "НоваяМодель") == 0);
    list_clear(&list);
    return pass;
}
