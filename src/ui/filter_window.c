#include "filter_window.h"
#include "product_object.h"
#include "filter.h"
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    AppState   *state;
    GtkWidget  *window;
    GtkWidget  *spin_qty;
    GtkWidget  *dd_search;
    GtkWidget  *entry_query;
    GtkWidget  *spin_price_min;
    GtkWidget  *spin_price_max;
    GListStore *result_store;
} FilterCtx;

// Fill the result store with products that pass the quantity filter
/*
 * Clears the store, then appends a ProductObject for every node whose
 * quantity is strictly below the threshold defined in f.
 */
static void store_fill_qty(GListStore *store, const List *list, const QtyFilter *f) {
    g_list_store_remove_all(store);
    Node *cur = list->head;
    while (cur != NULL) {
        if (filter_match_qty(&cur->data, f)) {
            ProductObject *obj = product_object_new(&cur->data);
            g_list_store_append(store, obj);
            g_object_unref(obj);
        }
        cur = cur->next;
    }
}

// Fill the result store with products that match the search filter
/*
 * Clears the store, then appends a ProductObject for every node that
 * satisfies the name, model or price-range criterion defined in f.
 */
static void store_fill_search(GListStore *store, const List *list, const SearchFilter *f) {
    g_list_store_remove_all(store);
    Node *cur = list->head;
    while (cur != NULL) {
        if (filter_match_search(&cur->data, f)) {
            ProductObject *obj = product_object_new(&cur->data);
            g_list_store_append(store, obj);
            g_object_unref(obj);
        }
        cur = cur->next;
    }
}

// Toggle visibility of the text entry and price-range box when mode changes
/*
 * Connected to the "notify::selected" signal of the search drop-down.
 * Shows the price-range box only when mode index 2 (Цена) is selected,
 * and hides it otherwise, showing the text entry instead.
 */
static void on_search_mode_changed(GtkDropDown *dd, GParamSpec *ps, gpointer ud) {
    (void)ps; (void)ud;
    guint mode = gtk_drop_down_get_selected(dd);
    GtkWidget *entry     = g_object_get_data(G_OBJECT(dd), "entry");
    GtkWidget *price_box = g_object_get_data(G_OBJECT(dd), "price-box");
    int is_price = (mode == 2);
    gtk_widget_set_visible(entry,     !is_price);
    gtk_widget_set_visible(price_box,  is_price);
}

// Apply the quantity filter and populate the result table
/*
 * Reads the threshold from the spin button and delegates matching
 * to filter_match_qty via store_fill_qty.
 */
static void on_apply_qty(GtkButton *btn, gpointer user_data) {
    (void)btn;
    FilterCtx *ctx = user_data;
    QtyFilter f;
    f.threshold = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(ctx->spin_qty));
    store_fill_qty(ctx->result_store, &ctx->state->list, &f);
}

// Apply the search filter and populate the result table
/*
 * Reads the selected mode, query text and price bounds from the UI,
 * then delegates matching to filter_match_search via store_fill_search.
 */
static void on_apply_search(GtkButton *btn, gpointer user_data) {
    (void)btn;
    FilterCtx *ctx = user_data;
    SearchFilter f;
    f.mode = (SearchMode)gtk_drop_down_get_selected(GTK_DROP_DOWN(ctx->dd_search));
    strncpy(f.query,
            gtk_editable_get_text(GTK_EDITABLE(ctx->entry_query)),
            MAX_STR - 1);
    f.query[MAX_STR - 1] = '\0';
    f.price_min = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ctx->spin_price_min));
    f.price_max = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ctx->spin_price_max));
    store_fill_search(ctx->result_store, &ctx->state->list, &f);
}

// GObject instance initialiser for a single column label widget
/*
 * Called by GtkSignalListItemFactory for each new list row slot.
 * Creates an empty GtkLabel as the child widget of the list item.
 */
static void on_setup(GtkSignalListItemFactory *f, GtkListItem *item, gpointer u) {
    (void)f; (void)u;
    gtk_list_item_set_child(item, gtk_label_new(""));
}

typedef struct { int col_idx; } ColD;

// Bind a ProductObject's field to the label in the current list item
/*
 * Called by GtkSignalListItemFactory whenever a row becomes visible.
 * Formats the field selected by col_idx and sets the label text.
 */
static void on_bind(GtkSignalListItemFactory *f, GtkListItem *item, gpointer user_data) {
    (void)f;
    ColD *cd = user_data;
    GtkWidget *label = gtk_list_item_get_child(item);
    const Product *p = product_object_get_product(
        PRODUCT_OBJECT(gtk_list_item_get_item(item)));
    char buf[128];
    buf[0] = '\0';
    if (cd->col_idx == 0)      { snprintf(buf, sizeof(buf), "%s",   p->group);    }
    else if (cd->col_idx == 1) { snprintf(buf, sizeof(buf), "%d",   p->code);     }
    else if (cd->col_idx == 2) { snprintf(buf, sizeof(buf), "%s",   p->name);     }
    else if (cd->col_idx == 3) { snprintf(buf, sizeof(buf), "%s",   p->model);    }
    else if (cd->col_idx == 4) { snprintf(buf, sizeof(buf), "%.2f", p->price);    }
    else if (cd->col_idx == 5) { snprintf(buf, sizeof(buf), "%d",   p->quantity); }
    gtk_label_set_text(GTK_LABEL(label), buf);
}

// Destructor passed to g_signal_connect_data to free the ColD allocation
/*
 * Called by GLib when the signal connection is finalised.
 */
static void free_col_d(gpointer data, GClosure *closure) {
    (void)closure;
    g_free(data);
}

// Append a text column to the column view using a signal-based factory
/*
 * Creates a ColD heap allocation that survives until the factory's
 * "bind" signal connection is destroyed, at which point free_col_d
 * releases it.
 */
static void add_col(GtkColumnView *cv, const char *title, int idx) {
    ColD *cd = g_new(ColD, 1);
    cd->col_idx = idx;
    GtkListItemFactory *f = gtk_signal_list_item_factory_new();
    g_signal_connect(f, "setup", G_CALLBACK(on_setup), NULL);
    g_signal_connect_data(f, "bind", G_CALLBACK(on_bind),
                          cd, free_col_d, 0);
    GtkColumnViewColumn *col = gtk_column_view_column_new(title, f);
    gtk_column_view_column_set_expand(col, TRUE);
    gtk_column_view_append_column(cv, col);
    g_object_unref(col);
}

// Free FilterCtx when the filter window is closed
/*
 * Connected to the "destroy" signal of the filter window.
 * Unrefs the result GListStore and frees the context struct.
 */
static void on_destroy(GtkWidget *w, gpointer user_data) {
    (void)w;
    FilterCtx *ctx = user_data;
    g_object_unref(ctx->result_store);
    g_free(ctx);
}

// Build and present the filter/search window
/*
 * Creates a transient window with two notebook tabs:
 *   Tab 1 — quantity filter (показывает товары с количеством < N)
 *   Tab 2 — text/price search with dynamic UI based on selected mode
 * Results are shown in a GtkColumnView at the bottom of the window.
 */
void filter_window_show(AppState *state) {
    FilterCtx *ctx = g_new0(FilterCtx, 1);
    ctx->state        = state;
    ctx->result_store = g_list_store_new(PRODUCT_TYPE_OBJECT);

    ctx->window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(ctx->window), "Фильтр и поиск");
    gtk_window_set_transient_for(GTK_WINDOW(ctx->window), GTK_WINDOW(state->window));
    gtk_window_set_default_size(GTK_WINDOW(ctx->window), 700, 500);
    g_signal_connect(ctx->window, "destroy", G_CALLBACK(on_destroy), ctx);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);
    gtk_window_set_child(GTK_WINDOW(ctx->window), vbox);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_box_append(GTK_BOX(vbox), notebook);

    /* tab 1 — quantity filter */
    GtkWidget *tab1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_start(tab1, 8);
    gtk_widget_set_margin_top(tab1, 8);
    gtk_widget_set_margin_bottom(tab1, 8);

    gtk_box_append(GTK_BOX(tab1), gtk_label_new("Количество <"));
    ctx->spin_qty = gtk_spin_button_new_with_range(0, 999999, 1);
    gtk_box_append(GTK_BOX(tab1), ctx->spin_qty);
    GtkWidget *btn_qty = gtk_button_new_with_label("Применить");
    gtk_box_append(GTK_BOX(tab1), btn_qty);
    g_signal_connect(btn_qty, "clicked", G_CALLBACK(on_apply_qty), ctx);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab1,
                             gtk_label_new("Фильтр по количеству"));

    /* tab 2 — search */
    GtkWidget *tab2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_start(tab2, 8);
    gtk_widget_set_margin_top(tab2, 8);
    gtk_widget_set_margin_bottom(tab2, 8);

    static const char *modes[] = { "Наименование", "Модель", "Цена", NULL };
    ctx->dd_search = gtk_drop_down_new_from_strings(modes);
    gtk_box_append(GTK_BOX(tab2), ctx->dd_search);

    ctx->entry_query = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->entry_query), "Поисковый запрос");
    gtk_widget_set_hexpand(ctx->entry_query, TRUE);
    gtk_box_append(GTK_BOX(tab2), ctx->entry_query);

    /* price range box — hidden until mode "Цена" is selected */
    GtkWidget *price_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_append(GTK_BOX(price_box), gtk_label_new("от"));
    ctx->spin_price_min = gtk_spin_button_new_with_range(0, 9999999, 0.01);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(ctx->spin_price_min), 2);
    gtk_widget_set_size_request(ctx->spin_price_min, 100, -1);
    gtk_box_append(GTK_BOX(price_box), ctx->spin_price_min);
    gtk_box_append(GTK_BOX(price_box), gtk_label_new("до"));
    ctx->spin_price_max = gtk_spin_button_new_with_range(0, 9999999, 0.01);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(ctx->spin_price_max), 2);
    gtk_widget_set_size_request(ctx->spin_price_max, 100, -1);
    gtk_box_append(GTK_BOX(price_box), ctx->spin_price_max);
    gtk_widget_set_visible(price_box, FALSE);
    gtk_box_append(GTK_BOX(tab2), price_box);

    g_object_set_data(G_OBJECT(ctx->dd_search), "entry",     ctx->entry_query);
    g_object_set_data(G_OBJECT(ctx->dd_search), "price-box", price_box);
    g_signal_connect(ctx->dd_search, "notify::selected",
                     G_CALLBACK(on_search_mode_changed), NULL);

    GtkWidget *btn_search = gtk_button_new_with_label("Найти");
    gtk_box_append(GTK_BOX(tab2), btn_search);
    g_signal_connect(btn_search, "clicked", G_CALLBACK(on_apply_search), ctx);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab2,
                             gtk_label_new("Поиск"));

    /* results table */
    GtkSingleSelection *sel = gtk_single_selection_new(
        G_LIST_MODEL(ctx->result_store));
    GtkWidget *cv = gtk_column_view_new(GTK_SELECTION_MODEL(sel));
    gtk_column_view_set_show_row_separators(GTK_COLUMN_VIEW(cv), TRUE);
    gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(cv), TRUE);

    add_col(GTK_COLUMN_VIEW(cv), "Группа",       0);
    add_col(GTK_COLUMN_VIEW(cv), "Код",          1);
    add_col(GTK_COLUMN_VIEW(cv), "Наименование", 2);
    add_col(GTK_COLUMN_VIEW(cv), "Модель",       3);
    add_col(GTK_COLUMN_VIEW(cv), "Цена",         4);
    add_col(GTK_COLUMN_VIEW(cv), "Количество",   5);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), cv);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_append(GTK_BOX(vbox), scroll);

    gtk_window_present(GTK_WINDOW(ctx->window));
}
