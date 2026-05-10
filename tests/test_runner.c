#include <stdio.h>
#include "test_list.h"
#include "test_sort.h"

typedef struct { const char *name; int (*fn)(void); } TestCase;

int main(void) {
    TestCase tests[] = {
        { "list: add and find",   test_list_add_and_find   },
        { "list: duplicate code", test_list_duplicate_code },
        { "list: delete",         test_list_delete         },
        { "list: edit",           test_list_edit           },
        { "sort: by code",        test_sort_by_code        },
        { "sort: by name",        test_sort_by_name        },
        { "sort: by price",       test_sort_by_price       },
    };

    int total  = (int)(sizeof(tests) / sizeof(tests[0]));
    int passed = 0;
    int i      = 0;
    while (i < total) {
        int ok = tests[i].fn();
        printf("[%s] %s\n", ok ? "PASS" : "FAIL", tests[i].name);
        if (ok) { passed++; }
        i++;
    }
    printf("\n%d/%d passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
