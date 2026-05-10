#include "test_sort.h"
#include "list.h"
#include "sort.h"
#include <string.h>

static void fill(List *list) {
    list_init(list);
    struct { int code; const char *name; double price; } items[] = {
        { 30, "Холодильник", 500.0 },
        { 10, "Вентилятор",  80.0  },
        { 20, "Кондиционер", 300.0 },
    };
    int n = (int)(sizeof(items) / sizeof(items[0]));
    int i = 0;
    while (i < n) {
        Product p = {0};
        p.code  = items[i].code;
        p.price = items[i].price;
        strncpy(p.name,  items[i].name, MAX_STR - 1);
        strncpy(p.group, "Электроника", MAX_STR - 1);
        strncpy(p.model, "М",           MAX_STR - 1);
        p.quantity = 1;
        list_add(list, &p);
        i++;
    }
}

int test_sort_by_code(void) {
    List list;
    fill(&list);
    list_sort(&list, SORT_BY_CODE, SORT_ASC);
    int pass = (list.head->data.code == 10 &&
                list.head->next->data.code == 20 &&
                list.tail->data.code == 30);
    list_clear(&list);
    return pass;
}

int test_sort_by_name(void) {
    List list;
    fill(&list);
    list_sort(&list, SORT_BY_NAME, SORT_ASC);
    int pass = (strcmp(list.head->data.name, "Вентилятор") == 0 &&
                strcmp(list.head->next->data.name, "Кондиционер") == 0 &&
                strcmp(list.tail->data.name, "Холодильник") == 0);
    list_clear(&list);
    return pass;
}

int test_sort_by_price(void) {
    List list;
    fill(&list);
    list_sort(&list, SORT_BY_PRICE, SORT_ASC);
    int pass = (list.head->data.price == 80.0 &&
                list.head->next->data.price == 300.0 &&
                list.tail->data.price == 500.0);
    list_clear(&list);
    return pass;
}

int test_sort_desc(void) {
    List list;
    fill(&list);
    list_sort(&list, SORT_BY_CODE, SORT_DESC);
    int pass = (list.head->data.code == 30 &&
                list.head->next->data.code == 20 &&
                list.tail->data.code == 10);
    list_clear(&list);
    return pass;
}
