#ifndef FILEIO_H
#define FILEIO_H

#include "product.h"

int fileio_save(const char *path, const List *list);
int fileio_load(const char *path, List *list);

#endif
