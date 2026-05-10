#include "filter.h"
#include <string.h>

// Check whether a product's quantity is below the filter threshold
/*
 * Returns 1 if p->quantity is strictly less than f->threshold, 0 otherwise.
 * Used to find low-stock items.
 */
int filter_match_qty(const Product *p, const QtyFilter *f) {
    return p->quantity < f->threshold;
}

// Check whether a product matches the text or price-range search criteria
/*
 * SEARCH_BY_NAME  : substring match against p->name  (case-sensitive)
 * SEARCH_BY_MODEL : substring match against p->model (case-sensitive)
 * SEARCH_BY_PRICE : inclusive range [price_min, price_max]
 * Returns 1 on match, 0 otherwise.
 */
int filter_match_search(const Product *p, const SearchFilter *f) {
    int result = 0;
    if (f->mode == SEARCH_BY_NAME) {
        result = (strstr(p->name, f->query) != NULL);
    } else if (f->mode == SEARCH_BY_MODEL) {
        result = (strstr(p->model, f->query) != NULL);
    } else {
        result = (p->price >= f->price_min && p->price <= f->price_max);
    }
    return result;
}
