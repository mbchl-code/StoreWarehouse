#include "fileio.h"
#include "list.h"
#include <stdio.h>

int fileio_save(const char *path, const List *list) {
    FILE *f = fopen(path, "wb");
    int ok = 0;
    if (f != NULL) {
        ok = 1;
        Node *cur = list->head;
        while (cur != NULL && ok) {
            size_t written = fwrite(&cur->data, sizeof(Product), 1, f);
            if (written != 1) {
                ok = 0;
            }
            cur = cur->next;
        }
        fclose(f);
    }
    return ok;
}

int fileio_load(const char *path, List *list) {
    FILE *f = fopen(path, "rb");
    int ok = 0;
    if (f != NULL) {
        ok = 1;
        list_clear(list);
        Product p;
        int running = (fread(&p, sizeof(Product), 1, f) == 1);
        while (running) {
            list_add(list, &p);
            running = (fread(&p, sizeof(Product), 1, f) == 1);
        }
        fclose(f);
    }
    return ok;
}
