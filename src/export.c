#include "export.h"
#include <string.h>
#include <xlsxwriter.h>

// Write the header row with column titles to the worksheet
/*
 * Fills row 0 with the six column names used in the inventory table.
 * Must be called before writing any data rows.
 */
static void write_header(lxw_worksheet *ws) {
    const char *cols[] = {
        "Группа", "Код", "Наименование", "Модель", "Цена", "Количество"
    };
    int n = (int)(sizeof(cols) / sizeof(cols[0]));
    int i = 0;
    while (i < n) {
        worksheet_write_string(ws, 0, (lxw_col_t)i, cols[i], NULL);
        i++;
    }
}

// Export the product list to an xlsx file using libxlsxwriter
/*
 * Creates a new workbook at path, adds a worksheet named "Инвентарь",
 * writes the header row, then writes one row per product in list order.
 * Strings are written with worksheet_write_string, numbers with
 * worksheet_write_number.
 * Returns 1 on success, 0 if the workbook could not be created or
 * workbook_close reports an error.
 */
int export_to_excel(const char *path, const List *list) {
    lxw_workbook  *wb = workbook_new(path);
    lxw_worksheet *ws = workbook_add_worksheet(wb, "Инвентарь");
    int ok = (wb != NULL && ws != NULL);
    if (ok) {
        write_header(ws);
        int row = 1;
        Node *cur = list->head;
        while (cur != NULL) {
            worksheet_write_string(ws, row, 0, cur->data.group, NULL);
            worksheet_write_number(ws, row, 1, cur->data.code,     NULL);
            worksheet_write_string(ws, row, 2, cur->data.name,     NULL);
            worksheet_write_string(ws, row, 3, cur->data.model,    NULL);
            worksheet_write_number(ws, row, 4, cur->data.price,    NULL);
            worksheet_write_number(ws, row, 5, cur->data.quantity, NULL);
            row++;
            cur = cur->next;
        }
        lxw_error err = workbook_close(wb);
        ok = (err == LXW_NO_ERROR);
    }
    return ok;
}
