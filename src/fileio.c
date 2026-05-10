#include "fileio.h"
#include "list.h"
#include <stdio.h>

// Write all products from the list to a typed binary file
/*
 * Opens the file at path in binary-write mode and writes each Product
 * struct as a fixed-size record using fwrite.
 * Returns 1 on success, 0 if the file could not be opened or a write
 * error occurred.
 */
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

// Load products from a typed binary file into the list
/*
 * Clears the existing list, then reads Product records one by one
 * with fread until EOF. Each successfully read record is appended
 * via list_add, which enforces the unique-code invariant.
 * Returns 1 if the file was opened successfully, 0 otherwise.
 */
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
