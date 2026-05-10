#include "filter.h"
#include <string.h>

int filter_match_qty(const Product *p, const QtyFilter *f) {
    return p->quantity < f->threshold;
}

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
