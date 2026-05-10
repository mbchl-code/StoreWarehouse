#include "filter_window.h"
#include "product_object.h"
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

static void result_store_fill(GListStore *store, const List *list,
                              int (*match)(const Product *, gpointer), gpointer arg) {
    g_list_store_remove_all(store);
    Node *cur = list->head;
    while (cur != NULL) {
        if (match(&cur->data, arg)) {
            ProductObject *obj = product_object_new(&cur->data);
            g_list_store_append(store, obj);
            g_object_unref(obj);
        }
        cur = cur->next;
    }
}

/* ── match functions ─────────────────────────────────────────── */

typedef struct { int threshold; } QtyArg;
typedef struct { int mode; char query[MAX_STR]; double pmin; double pmax; } SearchArg;

static int match_qty(const Product *p, gpointer arg) {
    QtyArg *a = arg;
    return p->quantity < a->threshold;
}

static int match_search(const Product *p, gpointer arg) {
    SearchArg *a = arg;
    int result = 0;
    if (a->mode == 0) {
        result = (strstr(p->name, a->query) != NULL);
    } else if (a->mode == 1) {
        result = (strstr(p->model, a->query) != NULL);
    } else {
        result = (p->price >= a->pmin && p->price <= a->pmax);
    }
    return result;
}

/* ── apply callbacks ─────────────────────────────────────────── */

static void on_apply_qty(GtkButton *btn, gpointer user_data) {
    (void)btn;
    FilterCtx *ctx = user_data;
    QtyArg arg;
    arg.threshold = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(ctx->spin_qty));
    result_store_fill(ctx->result_store, &ctx->state->list, match_qty, &arg);
}

static void on_apply_search(GtkButton *btn, gpointer user_data) {
    (void)btn;
    FilterCtx *ctx = user_data;
    SearchArg arg;
    arg.mode = (int)gtk_drop_down_get_selected(GTK_DROP_DOWN(ctx->dd_search));
    strncpy(arg.query,
            gtk_editable_get_text(GTK_EDITABLE(ctx->entry_query)),
            MAX_STR - 1);
    arg.query[MAX_STR - 1] = '\0';
    arg.pmin = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ctx->spin_price_min));
    arg.pmax = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ctx->spin_price_max));
    result_store_fill(ctx->result_store, &ctx->state->list, match_search, &arg);
}

/* ── column factory (shared) ─────────────────────────────────── */

typedef struct { int col_idx; } ColD;

static void on_setup(GtkSignalListItemFactory *f, GtkListItem *item, gpointer u) {
    (void)f; (void)u;
    gtk_list_item_set_child(item, gtk_label_new(""));
}

static void on_bind(GtkSignalListItemFactory *f, GtkListItem *item, gpointer user_data) {
    (void)f;
    ColD *cd = user_data;
    GtkWidget *label = gtk_list_item_get_child(item);
    const Product *p = product_object_get_product(
        PRODUCT_OBJECT(gtk_list_item_get_item(item)));
    char buf[128];
    buf[0] = '\0';
    if (cd->col_idx == 0)      { snprintf(buf, sizeof(buf), "%s", p->group); }
    else if (cd->col_idx == 1) { snprintf(buf, sizeof(buf), "%d", p->code);  }
    else if (cd->col_idx == 2) { snprintf(buf, sizeof(buf), "%s", p->name);  }
    else if (cd->col_idx == 3) { snprintf(buf, sizeof(buf), "%s", p->model); }
    else if (cd->col_idx == 4) { snprintf(buf, sizeof(buf), "%.2f", p->price); }
    else if (cd->col_idx == 5) { snprintf(buf, sizeof(buf), "%d", p->quantity); }
    gtk_label_set_text(GTK_LABEL(label), buf);
}

static void free_col_d(gpointer data, GClosure *closure) {
    (void)closure;
    g_free(data);
}

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

/* ── window build ────────────────────────────────────────────── */

static void on_destroy(GtkWidget *w, gpointer user_data) {
    (void)w;
    FilterCtx *ctx = user_data;
    g_object_unref(ctx->result_store);
    g_free(ctx);
}

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

    /* ── tab 1: filter by quantity ── */
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

    /* ── tab 2: search ── */
    GtkWidget *tab2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_start(tab2, 8);
    gtk_widget_set_margin_top(tab2, 8);
    gtk_widget_set_margin_bottom(tab2, 8);

    static const char *modes[] = { "Наименование", "Модель", "Цена (диапазон)", NULL };
    ctx->dd_search = gtk_drop_down_new_from_strings(modes);
    gtk_box_append(GTK_BOX(tab2), ctx->dd_search);

    ctx->entry_query = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->entry_query), "Запрос / мин.цена");
    gtk_widget_set_hexpand(ctx->entry_query, TRUE);
    gtk_box_append(GTK_BOX(tab2), ctx->entry_query);

    ctx->spin_price_min = gtk_spin_button_new_with_range(0, 9999999, 0.01);
    ctx->spin_price_max = gtk_spin_button_new_with_range(0, 9999999, 0.01);
    gtk_box_append(GTK_BOX(tab2), gtk_label_new("до"));
    gtk_box_append(GTK_BOX(tab2), ctx->spin_price_min);
    gtk_box_append(GTK_BOX(tab2), ctx->spin_price_max);

    GtkWidget *btn_search = gtk_button_new_with_label("Найти");
    gtk_box_append(GTK_BOX(tab2), btn_search);
    g_signal_connect(btn_search, "clicked", G_CALLBACK(on_apply_search), ctx);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab2,
                             gtk_label_new("Поиск"));

    /* ── results table ── */
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
