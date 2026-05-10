#include "main_window.h"
#include "product_object.h"
#include "product_dialog.h"
#include "filter_window.h"
#include "list.h"
#include "sort.h"
#include "fileio.h"
#include "export.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

static void add_text_column(GtkColumnView *cv, const char *title,
                            int col_idx, AppState *state);

/* ── helpers ─────────────────────────────────────────────────── */

static void show_error(GtkWidget *parent, const char *msg) {
    GtkAlertDialog *dlg = gtk_alert_dialog_new("%s", msg);
    gtk_alert_dialog_show(dlg, GTK_WINDOW(parent));
    g_object_unref(dlg);
}

static ProductObject *get_selected(AppState *state) {
    GtkSingleSelection *sel =
        GTK_SINGLE_SELECTION(gtk_column_view_get_model(
            GTK_COLUMN_VIEW(state->column_view)));
    return PRODUCT_OBJECT(gtk_single_selection_get_selected_item(sel));
}

/* ── toolbar callbacks ───────────────────────────────────────── */

static void on_add(GtkButton *btn, gpointer user_data) {
    (void)btn;
    AppState *state = user_data;
    product_dialog_show(state, NULL);
}

static void on_edit(GtkButton *btn, gpointer user_data) {
    (void)btn;
    AppState *state = user_data;
    ProductObject *obj = get_selected(state);
    if (obj != NULL) {
        product_dialog_show(state, product_object_get_product(obj));
    } else {
        show_error(state->window, "Выберите товар для редактирования.");
    }
}

static void on_delete(GtkButton *btn, gpointer user_data) {
    (void)btn;
    AppState *state = user_data;
    ProductObject *obj = get_selected(state);
    if (obj != NULL) {
        int code = product_object_get_product(obj)->code;
        list_delete(&state->list, code);
        app_refresh_store(state);
    } else {
        show_error(state->window, "Выберите товар для удаления.");
    }
}

static void on_sort(GtkButton *btn, gpointer user_data) {
    (void)btn;
    AppState *state = user_data;
    GtkDropDown *dd = g_object_get_data(G_OBJECT(state->window), "sort-dd");
    guint idx = gtk_drop_down_get_selected(dd);
    SortKey key = (idx == 1) ? SORT_BY_NAME : (idx == 2) ? SORT_BY_PRICE : SORT_BY_CODE;
    list_sort(&state->list, key);
    app_refresh_store(state);
}

static void on_filter(GtkButton *btn, gpointer user_data) {
    (void)btn;
    AppState *state = user_data;
    filter_window_show(state);
}

/* ── file callbacks ──────────────────────────────────────────── */

static void on_open_ready(GObject *src, GAsyncResult *res, gpointer user_data) {
    AppState *state = user_data;
    GError *err = NULL;
    GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(src), res, &err);
    if (file != NULL) {
        char *path = g_file_get_path(file);
        if (!fileio_load(path, &state->list)) {
            show_error(state->window, "Ошибка загрузки файла.");
        } else {
            snprintf(state->current_file, sizeof(state->current_file), "%s", path);
            app_refresh_store(state);
        }
        g_free(path);
        g_object_unref(file);
    } else {
        if (err != NULL) { g_error_free(err); }
    }
}

static GtkFileFilter *make_inv_filter(void) {
    GtkFileFilter *f = gtk_file_filter_new();
    gtk_file_filter_set_name(f, "Файлы инвентаря (*.inv)");
    gtk_file_filter_add_suffix(f, "inv");
    return f;
}

static GtkFileFilter *make_xlsx_filter(void) {
    GtkFileFilter *f = gtk_file_filter_new();
    gtk_file_filter_set_name(f, "Таблицы Excel (*.xlsx)");
    gtk_file_filter_add_suffix(f, "xlsx");
    return f;
}

static void on_open(GtkButton *btn, gpointer user_data) {
    (void)btn;
    AppState *state = user_data;
    GtkFileDialog *fd = gtk_file_dialog_new();
    gtk_file_dialog_set_title(fd, "Открыть файл инвентаря");

    GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, make_inv_filter());
    gtk_file_dialog_set_filters(fd, G_LIST_MODEL(filters));
    gtk_file_dialog_set_default_filter(fd, g_list_model_get_item(G_LIST_MODEL(filters), 0));
    g_object_unref(filters);

    gtk_file_dialog_open(fd, GTK_WINDOW(state->window), NULL, on_open_ready, state);
    g_object_unref(fd);
}

static char *ensure_inv_ext(const char *path) {
    size_t len = strlen(path);
    int has_ext = (len > 4 && strcmp(path + len - 4, ".inv") == 0);
    char *result = NULL;
    if (has_ext) {
        result = g_strdup(path);
    } else {
        result = g_strconcat(path, ".inv", NULL);
    }
    return result;
}

static void on_save_ready(GObject *src, GAsyncResult *res, gpointer user_data) {
    AppState *state = user_data;
    GError *err = NULL;
    GFile *file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(src), res, &err);
    if (file != NULL) {
        char *raw  = g_file_get_path(file);
        char *path = ensure_inv_ext(raw);
        if (!fileio_save(path, &state->list)) {
            show_error(state->window, "Ошибка сохранения файла.");
        } else {
            snprintf(state->current_file, sizeof(state->current_file), "%s", path);
        }
        g_free(path);
        g_free(raw);
        g_object_unref(file);
    } else {
        if (err != NULL) { g_error_free(err); }
    }
}

static void on_save(GtkButton *btn, gpointer user_data) {
    (void)btn;
    AppState *state = user_data;
    if (state->current_file[0] != '\0') {
        if (!fileio_save(state->current_file, &state->list)) {
            show_error(state->window, "Ошибка сохранения файла.");
        }
    } else {
        GtkFileDialog *fd = gtk_file_dialog_new();
        gtk_file_dialog_set_title(fd, "Сохранить файл инвентаря");
        gtk_file_dialog_set_initial_name(fd, "inventory.inv");

        GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
        g_list_store_append(filters, make_inv_filter());
        gtk_file_dialog_set_filters(fd, G_LIST_MODEL(filters));
        gtk_file_dialog_set_default_filter(fd, g_list_model_get_item(G_LIST_MODEL(filters), 0));
        g_object_unref(filters);

        gtk_file_dialog_save(fd, GTK_WINDOW(state->window), NULL, on_save_ready, state);
        g_object_unref(fd);
    }
}

static void on_export_ready(GObject *src, GAsyncResult *res, gpointer user_data) {
    AppState *state = user_data;
    GError *err = NULL;
    GFile *file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(src), res, &err);
    if (file != NULL) {
        char *path = g_file_get_path(file);
        if (!export_to_excel(path, &state->list)) {
            show_error(state->window, "Ошибка экспорта.");
        }
        g_free(path);
        g_object_unref(file);
    } else {
        if (err != NULL) { g_error_free(err); }
    }
}

static void on_export(GtkButton *btn, gpointer user_data) {
    (void)btn;
    AppState *state = user_data;
    GtkFileDialog *fd = gtk_file_dialog_new();
    gtk_file_dialog_set_title(fd, "Экспорт в Excel");
    gtk_file_dialog_set_initial_name(fd, "inventory.xlsx");

    GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, make_xlsx_filter());
    gtk_file_dialog_set_filters(fd, G_LIST_MODEL(filters));
    gtk_file_dialog_set_default_filter(fd, g_list_model_get_item(G_LIST_MODEL(filters), 0));
    g_object_unref(filters);

    gtk_file_dialog_save(fd, GTK_WINDOW(state->window), NULL, on_export_ready, state);
    g_object_unref(fd);
}

/* ── column factory ──────────────────────────────────────────── */

typedef struct { int col_idx; } ColData;

static void on_bind(GtkSignalListItemFactory *f, GtkListItem *item, gpointer user_data) {
    (void)f;
    ColData *cd = user_data;
    GtkWidget *label = gtk_list_item_get_child(item);
    ProductObject *obj = PRODUCT_OBJECT(gtk_list_item_get_item(item));
    const Product *p = product_object_get_product(obj);
    char buf[128];
    buf[0] = '\0';
    if (cd->col_idx == 0) { snprintf(buf, sizeof(buf), "%s", p->group); }
    else if (cd->col_idx == 1) { snprintf(buf, sizeof(buf), "%d", p->code); }
    else if (cd->col_idx == 2) { snprintf(buf, sizeof(buf), "%s", p->name); }
    else if (cd->col_idx == 3) { snprintf(buf, sizeof(buf), "%s", p->model); }
    else if (cd->col_idx == 4) { snprintf(buf, sizeof(buf), "%.2f", p->price); }
    else if (cd->col_idx == 5) { snprintf(buf, sizeof(buf), "%d", p->quantity); }
    gtk_label_set_text(GTK_LABEL(label), buf);
}

static void on_setup(GtkSignalListItemFactory *f, GtkListItem *item, gpointer user_data) {
    (void)f;
    (void)user_data;
    gtk_list_item_set_child(item, gtk_label_new(""));
}

static void free_col_data(gpointer data, GClosure *closure) {
    (void)closure;
    g_free(data);
}

static void add_text_column(GtkColumnView *cv, const char *title,
                             int col_idx, AppState *state) {
    (void)state;
    ColData *cd = g_new(ColData, 1);
    cd->col_idx = col_idx;

    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup", G_CALLBACK(on_setup), NULL);
    g_signal_connect_data(factory, "bind", G_CALLBACK(on_bind),
                          cd, free_col_data, 0);

    GtkColumnViewColumn *col = gtk_column_view_column_new(title, factory);
    gtk_column_view_column_set_expand(col, TRUE);
    gtk_column_view_append_column(cv, col);
    g_object_unref(col);
}

/* ── window build ────────────────────────────────────────────── */

void main_window_build(AppState *state) {
    state->window = gtk_application_window_new(state->app);
    gtk_window_set_title(GTK_WINDOW(state->window), "Инвентарь магазина");
    gtk_window_set_default_size(GTK_WINDOW(state->window), 900, 600);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(state->window), vbox);

    /* toolbar */
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_margin_start(toolbar, 6);
    gtk_widget_set_margin_end(toolbar, 6);
    gtk_widget_set_margin_top(toolbar, 6);
    gtk_widget_set_margin_bottom(toolbar, 6);
    gtk_box_append(GTK_BOX(vbox), toolbar);

    GtkWidget *btn_add  = gtk_button_new_with_label("Добавить");
    GtkWidget *btn_edit = gtk_button_new_with_label("Редактировать");
    GtkWidget *btn_del  = gtk_button_new_with_label("Удалить");

    static const char *sort_opts[] = { "По коду", "По наименованию", "По цене", NULL };
    GtkWidget *dd_sort = gtk_drop_down_new_from_strings(sort_opts);
    GtkWidget *btn_sort   = gtk_button_new_with_label("Сортировать");
    GtkWidget *btn_filter = gtk_button_new_with_label("Фильтр/Поиск");
    GtkWidget *btn_open   = gtk_button_new_with_label("Открыть");
    GtkWidget *btn_save   = gtk_button_new_with_label("Сохранить");
    GtkWidget *btn_export = gtk_button_new_with_label("Экспорт");

    g_object_set_data(G_OBJECT(state->window), "sort-dd", dd_sort);

    gtk_box_append(GTK_BOX(toolbar), btn_add);
    gtk_box_append(GTK_BOX(toolbar), btn_edit);
    gtk_box_append(GTK_BOX(toolbar), btn_del);
    gtk_box_append(GTK_BOX(toolbar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
    gtk_box_append(GTK_BOX(toolbar), dd_sort);
    gtk_box_append(GTK_BOX(toolbar), btn_sort);
    gtk_box_append(GTK_BOX(toolbar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
    gtk_box_append(GTK_BOX(toolbar), btn_filter);
    gtk_box_append(GTK_BOX(toolbar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
    gtk_box_append(GTK_BOX(toolbar), btn_open);
    gtk_box_append(GTK_BOX(toolbar), btn_save);
    gtk_box_append(GTK_BOX(toolbar), btn_export);

    g_signal_connect(btn_add,    "clicked", G_CALLBACK(on_add),    state);
    g_signal_connect(btn_edit,   "clicked", G_CALLBACK(on_edit),   state);
    g_signal_connect(btn_del,    "clicked", G_CALLBACK(on_delete), state);
    g_signal_connect(btn_sort,   "clicked", G_CALLBACK(on_sort),   state);
    g_signal_connect(btn_filter, "clicked", G_CALLBACK(on_filter), state);
    g_signal_connect(btn_open,   "clicked", G_CALLBACK(on_open),   state);
    g_signal_connect(btn_save,   "clicked", G_CALLBACK(on_save),   state);
    g_signal_connect(btn_export, "clicked", G_CALLBACK(on_export), state);

    /* column view */
    GtkSingleSelection *sel = gtk_single_selection_new(G_LIST_MODEL(state->store));
    gtk_single_selection_set_autoselect(sel, FALSE);

    state->column_view = gtk_column_view_new(GTK_SELECTION_MODEL(sel));
    gtk_column_view_set_show_row_separators(GTK_COLUMN_VIEW(state->column_view), TRUE);
    gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(state->column_view), TRUE);

    add_text_column(GTK_COLUMN_VIEW(state->column_view), "Группа",       0, state);
    add_text_column(GTK_COLUMN_VIEW(state->column_view), "Код",          1, state);
    add_text_column(GTK_COLUMN_VIEW(state->column_view), "Наименование", 2, state);
    add_text_column(GTK_COLUMN_VIEW(state->column_view), "Модель",       3, state);
    add_text_column(GTK_COLUMN_VIEW(state->column_view), "Цена",         4, state);
    add_text_column(GTK_COLUMN_VIEW(state->column_view), "Количество",   5, state);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), state->column_view);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_append(GTK_BOX(vbox), scroll);

    gtk_window_present(GTK_WINDOW(state->window));
}
