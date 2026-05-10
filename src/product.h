#ifndef PRODUCT_H
#define PRODUCT_H

#define MAX_STR 64

typedef struct Product {
    char   group[MAX_STR];
    int    code;
    char   name[MAX_STR];
    char   model[MAX_STR];
    double price;
    int    quantity;
} Product;

typedef struct Node {
    Product      data;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int   size;
} List;

typedef enum {
    SORT_BY_CODE,
    SORT_BY_NAME,
    SORT_BY_PRICE
} SortKey;

typedef enum {
    SORT_ASC,
    SORT_DESC
} SortOrder;

#endif
