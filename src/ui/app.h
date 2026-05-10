#ifndef APP_H
#define APP_H

#include <gtk/gtk.h>
#include "product.h"

typedef struct {
    GtkApplication *app;
    GtkWidget      *window;
    GtkWidget      *column_view;
    GListStore     *store;
    List            list;
    char            current_file[512];
} AppState;

void app_state_init(AppState *state, GtkApplication *app);
void app_state_free(AppState *state);

void app_refresh_store(AppState *state);

#endif
