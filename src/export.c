#include "export.h"
#include <string.h>
#include <xlsxwriter.h>

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
