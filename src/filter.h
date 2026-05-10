#ifndef FILTER_H
#define FILTER_H

#include "product.h"

typedef enum {
    SEARCH_BY_NAME  = 0,
    SEARCH_BY_MODEL = 1,
    SEARCH_BY_PRICE = 2
} SearchMode;

typedef struct {
    int threshold;
} QtyFilter;

typedef struct {
    SearchMode mode;
    char       query[MAX_STR];
    double     price_min;
    double     price_max;
} SearchFilter;

int filter_match_qty(const Product *p, const QtyFilter *f);
int filter_match_search(const Product *p, const SearchFilter *f);

#endif
