#ifndef LIST_H
#define LIST_H

#include "product.h"

void    list_init(List *list);
void    list_clear(List *list);

int     list_add(List *list, const Product *p);
int     list_delete(List *list, int code);
Node   *list_find_by_code(const List *list, int code);

void    list_edit(Node *node, double price, int quantity, const char *model);

#endif
