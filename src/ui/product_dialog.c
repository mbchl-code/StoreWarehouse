#include "product_dialog.h"
#include "list.h"
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    AppState   *state;
    GtkWidget  *dialog;
    GtkWidget  *entry_group;
    GtkWidget  *spin_code;
    GtkWidget  *entry_name;
    GtkWidget  *entry_model;
    GtkWidget  *spin_price;
    GtkWidget  *spin_qty;
    int         edit_code;
} DialogCtx;

// Show a GTK alert dialog with an error message
/*
 * Creates a modal GtkAlertDialog, presents it over parent, then
 * unrefs it. Used for validation and IO error feedback.
 */
static void show_error(GtkWidget *parent, const char *msg) {
    GtkAlertDialog *dlg = gtk_alert_dialog_new("%s", msg);
    const char *buttons[] = { "Закрыть", NULL };
    gtk_alert_dialog_set_buttons(dlg, buttons);
    gtk_alert_dialog_show(dlg, GTK_WINDOW(parent));
    g_object_unref(dlg);
}

// Handle the OK button: validate input, then add or edit the product
/*
 * In add mode (edit_code < 0): validates all fields are non-empty and
 * code > 0, then calls list_add. Reports ФТ-7 duplicate-code error if
 * the code already exists.
 * In edit mode: calls list_edit to update price, quantity and model,
 * then closes the dialog.
 */
static void on_ok(GtkButton *btn, gpointer user_data) {
    (void)btn;
    DialogCtx *ctx = user_data;

    const char *group = gtk_editable_get_text(GTK_EDITABLE(ctx->entry_group));
    const char *name  = gtk_editable_get_text(GTK_EDITABLE(ctx->entry_name));
    const char *model = gtk_editable_get_text(GTK_EDITABLE(ctx->entry_model));
    int    code  = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(ctx->spin_code));
    double price = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ctx->spin_price));
    int    qty   = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(ctx->spin_qty));

    int valid = (group[0] != '\0' && name[0] != '\0' && model[0] != '\0');
    if (!valid) {
        show_error(ctx->dialog, "Заполнены не все поля! Для добавления товара необходимо заполнить все поля.");
    } else if (price < 0.0) {
        show_error(ctx->dialog, "Цена должна быть положительным числом!");
    } else if (ctx->edit_code < 0) {
        Product p = {0};
        strncpy(p.group, group, MAX_STR - 1);
        p.code = code;
        strncpy(p.name,  name,  MAX_STR - 1);
        strncpy(p.model, model, MAX_STR - 1);
        p.price    = price;
        p.quantity = qty;
        int added = list_add(&ctx->state->list, &p);
        if (!added) {
            show_error(ctx->dialog, "Товар с таким кодом уже существует.");
        } else {
            app_refresh_store(ctx->state);
            gtk_window_destroy(GTK_WINDOW(ctx->dialog));
        }
    } else {
        Node *node = list_find_by_code(&ctx->state->list, ctx->edit_code);
        if (node != NULL) {
            list_edit(node, price, qty, model);
            app_refresh_store(ctx->state);
        }
        gtk_window_destroy(GTK_WINDOW(ctx->dialog));
    }
}

// Close the dialog without saving changes
/*
 * Connected to the Cancel button. Destroys the dialog window,
 * which triggers on_destroy and frees the DialogCtx.
 */
static void on_cancel(GtkButton *btn, gpointer user_data) {
    (void)btn;
    DialogCtx *ctx = user_data;
    gtk_window_destroy(GTK_WINDOW(ctx->dialog));
}

// Free the DialogCtx allocation when the dialog is destroyed
/*
 * Connected to the "destroy" signal of the dialog window so that
 * cleanup occurs regardless of whether OK or Cancel was pressed.
 */
static void on_destroy(GtkWidget *w, gpointer user_data) {
    (void)w;
    g_free(user_data);
}

// Build and present the add/edit product dialog
/*
 * When existing is NULL the dialog is in add mode (edit_code = -1).
 * When existing points to a product the dialog pre-fills all fields;
 * code and name are made insensitive because they identify the record.
 * The dialog is modal and transient over the main window.
 */
void product_dialog_show(AppState *state, const Product *existing) {
    int is_edit = (existing != NULL);

    DialogCtx *ctx = g_new0(DialogCtx, 1);
    ctx->state     = state;
    ctx->edit_code = is_edit ? existing->code : -1;

    ctx->dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(ctx->dialog),
                         is_edit ? "Редактировать товар" : "Добавить товар");
    gtk_window_set_transient_for(GTK_WINDOW(ctx->dialog), GTK_WINDOW(state->window));
    gtk_window_set_modal(GTK_WINDOW(ctx->dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(ctx->dialog), 360, 260);
    g_signal_connect(ctx->dialog, "destroy", G_CALLBACK(on_destroy), ctx);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);
    gtk_window_set_child(GTK_WINDOW(ctx->dialog), vbox);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_box_append(GTK_BOX(vbox), grid);

    ctx->entry_group = gtk_entry_new();
    ctx->spin_code   = gtk_spin_button_new_with_range(1, 9999999, 1);
    ctx->entry_name  = gtk_entry_new();
    ctx->entry_model = gtk_entry_new();
    ctx->spin_price  = gtk_spin_button_new_with_range(-9999999.0, 9999999.0, 0.01);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(ctx->spin_price), 2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->spin_price), 0.0);
    ctx->spin_qty    = gtk_spin_button_new_with_range(0, 999999, 1);

    struct { const char *label; GtkWidget *widget; } rows[] = {
        { "Группа:",       ctx->entry_group },
        { "Код:",          ctx->spin_code   },
        { "Наименование:", ctx->entry_name  },
        { "Модель:",       ctx->entry_model },
        { "Цена:",         ctx->spin_price  },
        { "Количество:",   ctx->spin_qty    },
    };

    int n = (int)(sizeof(rows) / sizeof(rows[0]));
    int i = 0;
    while (i < n) {
        GtkWidget *lbl = gtk_label_new(rows[i].label);
        gtk_label_set_xalign(GTK_LABEL(lbl), 1.0f);
        gtk_grid_attach(GTK_GRID(grid), lbl,          0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), rows[i].widget, 1, i, 1, 1);
        gtk_widget_set_hexpand(rows[i].widget, TRUE);
        i++;
    }

    if (is_edit) {
        gtk_editable_set_text(GTK_EDITABLE(ctx->entry_group), existing->group);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->spin_code), existing->code);
        gtk_widget_set_sensitive(ctx->spin_code, FALSE);
        gtk_editable_set_text(GTK_EDITABLE(ctx->entry_name), existing->name);
        gtk_widget_set_sensitive(ctx->entry_name, FALSE);
        gtk_editable_set_text(GTK_EDITABLE(ctx->entry_model), existing->model);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->spin_price), existing->price);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->spin_qty), existing->quantity);
    }

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vbox), btn_box);

    GtkWidget *btn_cancel = gtk_button_new_with_label("Отмена");
    GtkWidget *btn_ok     = gtk_button_new_with_label("OK");
    gtk_box_append(GTK_BOX(btn_box), btn_cancel);
    gtk_box_append(GTK_BOX(btn_box), btn_ok);

    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(on_cancel), ctx);
    g_signal_connect(btn_ok,     "clicked", G_CALLBACK(on_ok),     ctx);

    gtk_window_present(GTK_WINDOW(ctx->dialog));
}
