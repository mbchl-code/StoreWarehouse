#include "app.h"
#include "product_object.h"
#include "list.h"

void app_state_init(AppState *state, GtkApplication *app) {
    state->app         = app;
    state->window      = NULL;
    state->column_view = NULL;
    state->store       = g_list_store_new(PRODUCT_TYPE_OBJECT);
    list_init(&state->list);
    state->current_file[0] = '\0';
}

void app_state_free(AppState *state) {
    list_clear(&state->list);
    g_object_unref(state->store);
}

void app_refresh_store(AppState *state) {
    g_list_store_remove_all(state->store);
    Node *cur = state->list.head;
    while (cur != NULL) {
        ProductObject *obj = product_object_new(&cur->data);
        g_list_store_append(state->store, obj);
        g_object_unref(obj);
        cur = cur->next;
    }
}
