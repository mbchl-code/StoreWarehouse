#include "app.h"
#include "product_object.h"
#include "list.h"

// Initialise AppState with a fresh empty list and a new GListStore
/*
 * Must be called once in the "activate" signal before any UI is built.
 * Allocates the GListStore used by GtkColumnView and zeroes the
 * current_file path.
 */
void app_state_init(AppState *state, GtkApplication *app) {
    state->app         = app;
    state->window      = NULL;
    state->column_view = NULL;
    state->store       = g_list_store_new(PRODUCT_TYPE_OBJECT);
    list_init(&state->list);
    state->current_file[0] = '\0';
}

// Release all resources owned by AppState
/*
 * Frees every node in the product list and unrefs the GListStore.
 * GTK widgets are owned by the window and freed automatically when
 * the window is destroyed.
 */
void app_state_free(AppState *state) {
    list_clear(&state->list);
    g_object_unref(state->store);
}

// Rebuild the GListStore from the current state of the product list
/*
 * Removes all items from the store, then iterates the doubly-linked
 * list and appends a new ProductObject for each node.
 * GtkColumnView observes the store and updates its rows automatically.
 */
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
